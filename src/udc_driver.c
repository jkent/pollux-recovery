/*
 * Copyright (C) 2013 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "asm/io.h"
#include "baremetal/cache.h"
#include "baremetal/util.h"

#include "udc.h"
#include "descriptors.h"

static struct udc_req setup_req = {0};
static struct udc_req buffer_req = {0};

static int process_req_vendor(struct udc *udc,	struct usb_ctrlrequest *ctrl);

static inline int process_req_desc(struct udc *udc,
		struct usb_ctrlrequest *ctrl)
{
	struct udc_ep *ep0 = &udc->ep[0];
	struct udc_req *req = &setup_req;
	int i;

	switch (ctrl->wValue >> 8) {
	case USB_DT_DEVICE:
		if (udc->speed == USB_SPEED_HIGH) {
			req->buf = (u16 *)&hs_device_descriptor;
			req->length = hs_device_descriptor.bLength;
		} else {
			req->buf = (u16 *)&fs_device_descriptor;
			req->length = fs_device_descriptor.bLength;
		}
		break;

	case USB_DT_DEVICE_QUALIFIER:
		if (udc->speed == USB_SPEED_HIGH) {
			req->buf = (u16 *)&hs_qualifier_descriptor;
			req->length = hs_qualifier_descriptor.bLength;
		} else {
			req->buf = (u16 *)&fs_qualifier_descriptor;
			req->length = fs_qualifier_descriptor.bLength;
		}
		break;

	case USB_DT_OTHER_SPEED_CONFIG:
		if (udc->speed == USB_SPEED_HIGH) {
			fs_config_descriptor.cfg.bDescriptorType =
					USB_DT_OTHER_SPEED_CONFIG;
			req->buf = (u16 *)&fs_config_descriptor;
			req->length = fs_config_descriptor.cfg.wTotalLength;
		} else {
			hs_config_descriptor.cfg.bDescriptorType =
					USB_DT_OTHER_SPEED_CONFIG;
			req->buf = (u16 *)&hs_config_descriptor;
			req->length = hs_config_descriptor.cfg.wTotalLength;
		}
		break;

	case USB_DT_CONFIG:
		if (udc->speed == USB_SPEED_HIGH) {
			hs_config_descriptor.cfg.bDescriptorType =
					USB_DT_CONFIG;
			req->buf = (u16 *)&hs_config_descriptor;
			req->length = hs_config_descriptor.cfg.wTotalLength;
		} else {
			fs_config_descriptor.cfg.bDescriptorType = USB_DT_CONFIG;
			req->buf = (u16 *)&fs_config_descriptor;
			req->length = fs_config_descriptor.cfg.wTotalLength;
		}
		break;

	case USB_DT_STRING:
		i = ctrl->wValue & 0xFF;
		if (i >= NUM_STRING_DESC)
			return -1;
		req->buf = (u16 *)string_descriptor[i];
		req->length = string_descriptor[i]->bLength;
		break;

	default:
		return -1;
	}

	INIT_LIST_HEAD(&req->queue);
	req->length = min((u32)ctrl->wLength, req->length);
	ep0->ops->queue(ep0, req);
	return 0;
}

static inline void set_config(struct udc *udc, int config)
{
	struct udc_ep *ep1 = &udc->ep[1];
	struct usb_endpoint_descriptor *ep1_descriptor;

	if (udc->speed == USB_SPEED_HIGH)
		ep1_descriptor = &hs_config_descriptor.ep1;
	else
		ep1_descriptor = &fs_config_descriptor.ep1;

	ep1->ops->disable(ep1);
	if (config)
		ep1->ops->enable(ep1, ep1_descriptor);
	udc->config = config;
}

static inline int process_req_config(struct udc *udc,
		struct usb_ctrlrequest *ctrl)
{
	struct udc_ep *ep0 = &udc->ep[0];
	struct udc_req *req = &setup_req;

	if (ctrl->bRequest == USB_REQ_SET_CONFIGURATION) {
		if (ctrl->wValue > NUM_CONFIG_DESC)
			return -1;
		set_config(udc, ctrl->wValue);
	} else {
		bzero(req, sizeof(*req));
		req->buf = &udc->config;
		req->length = 1;
		ep0->ops->queue(ep0, req);
	}
	return 0;
}

static inline int process_req_iface(struct udc *udc,
		struct usb_ctrlrequest *ctrl)
{
	struct udc_ep *ep0 = &udc->ep[0];
	struct udc_req *req = &setup_req;
	u8 interface = ctrl->wIndex & 0xff;
	u8 alternate = ctrl->wValue & 0xff;
	u16 reply;

	if (ctrl->bRequest == USB_REQ_SET_INTERFACE) {
		if (interface || alternate)
			return -1;
	} else {
		bzero(req, sizeof(*req));
		INIT_LIST_HEAD(&req->queue);
		reply = 0;
		req->buf = &reply;
		req->length = 1;
		ep0->ops->queue(ep0, req);
	}
	return 0;
}

static int process_setup(struct udc *udc, struct usb_ctrlrequest *ctrl)
{
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR)
		return process_req_vendor(udc, ctrl);

	if ((ctrl->bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD)
		return -1;

	switch (ctrl->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE)
			break;
		return process_req_desc(udc, ctrl);

	case USB_REQ_GET_CONFIGURATION:
	case USB_REQ_SET_CONFIGURATION:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE)
			break;
		return process_req_config(udc, ctrl);

	case USB_REQ_GET_INTERFACE:
	case USB_REQ_SET_INTERFACE:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
			break;
		return process_req_iface(udc, ctrl);
	}
	return -1;
}

struct udc_driver udc_driver = {
	.setup = process_setup,
};

/**************************************************************************/
/* Application specific code                                              */
/**************************************************************************/

static u16 cmd;
static u8 buf[8];

enum commands {
	COMMAND_LOAD = 0,
	COMMAND_RUN,
};

struct load_data {
	void *addr;
	u32 length;
};

struct run_data {
	void (*f)(void);
};

static void command_data(struct udc_ep *ep, struct udc_req *req)
{
	struct udc *udc = ep->dev;
	struct udc_ep *ep1 = &udc->ep[1];

	switch (cmd) {
	case COMMAND_LOAD:
		if (req->actual != sizeof(struct load_data))
			return;

		struct load_data *load = req->buf;

		bzero(&buffer_req, sizeof(buffer_req));
		INIT_LIST_HEAD(&buffer_req.queue);
		buffer_req.buf = load->addr;
		buffer_req.length = load->length;
		ep1->ops->queue(ep1, &buffer_req);
		break;

	case COMMAND_RUN:
		if (req->actual != sizeof(struct run_data))
			return;

		struct run_data *run = req->buf;
		disable_cache();
		run->f();
		break;
	}
}

static int command_handler(struct udc *udc, struct usb_ctrlrequest *ctrl)
{
	struct udc_ep *ep0 = &udc->ep[0];
	cmd = ctrl->wValue;
	
	if (!(ctrl->bRequestType & USB_DIR_IN)) {
		bzero(&setup_req, sizeof(setup_req));
		INIT_LIST_HEAD(&setup_req.queue);
		setup_req.buf = buf;
		setup_req.length = min((u32)ctrl->wLength, sizeof(buf));
		setup_req.complete = command_data;
		switch (cmd) {
		case COMMAND_LOAD:
		case COMMAND_RUN:
			ep0->ops->queue(ep0, &setup_req);
			return 0;
		}
	}
	return -1;
}

static int process_req_vendor(struct udc *udc, struct usb_ctrlrequest *ctrl)
{
	if (ctrl->bRequest == 0x40)
		return command_handler(udc, ctrl);

	return -1;
}
