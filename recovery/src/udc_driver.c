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
#include "baremetal/util.h"

#include "udc.h"
#include "descriptors.h"

static struct udc_req setup_req = {0};
static struct udc_req buffer_req = {0};
static struct udc_ep *data_ep;

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
	struct usb_endpoint_descriptor *data_ep_descriptor;

	if (udc->speed == USB_SPEED_HIGH) {
		data_ep_descriptor = &hs_config_descriptor.ep1;
	} else {
		data_ep_descriptor = &fs_config_descriptor.ep1;
	}

	data_ep->ops->disable(data_ep);
	if (config)
		data_ep->ops->enable(data_ep, data_ep_descriptor);
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

static void buffer_req_complete(struct udc_ep *ep, struct udc_req *req)
{
	if (req->status)
		return;
}

static void init(struct udc *udc)
{
	data_ep = &udc->ep[1];

	buffer_req.complete = buffer_req_complete;
	INIT_LIST_HEAD(&buffer_req.queue);
}

struct udc_driver udc_driver = {
	.setup = process_setup,
	.init = init,
};

