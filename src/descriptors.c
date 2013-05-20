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

#include "linux/usb/ch9.h"
#include "descriptors.h"

#define USB_VID 0x0000
#define USB_PID 0x7F20
#define MANUFACTURER_STRING u"Jeff Kent <jeff@jkent.net>"
#define PRODUCT_STRING u"POLLUX recovery"

/* High speed descriptors */
__attribute__((aligned(2)))
const struct usb_device_descriptor hs_device_descriptor = {
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 64,
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.iManufacturer      = 1,
	.iProduct           = 2,
	.bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(2)))
const struct usb_qualifier_descriptor hs_qualifier_descriptor = {
	.bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
	.bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 8,
	.bNumConfigurations = 1,
};

__attribute__((aligned(2)))
struct usb_device_config_descriptor hs_config_descriptor = {
	.cfg = {
		.bLength             = USB_DT_CONFIG_SIZE,
		.bDescriptorType     = USB_DT_CONFIG,
		.wTotalLength        = USB_DT_CONFIG_SIZE +
		                       USB_DT_INTERFACE_SIZE +
		                       (USB_DT_ENDPOINT_SIZE * 1),
		.bNumInterfaces      = 1,
		.bConfigurationValue = 1,
		.bmAttributes        = USB_CONFIG_ATT_ONE |
		                       USB_CONFIG_ATT_SELFPOWER,
	},
	.if0 = {
		.bLength             = USB_DT_INTERFACE_SIZE,
		.bDescriptorType     = USB_DT_INTERFACE,
		.bInterfaceNumber    = 0,
		.bNumEndpoints       = 1,
	},
	.ep1 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 1 | USB_DIR_OUT,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 512,
	},
};

/* Full speed descriptors */
__attribute__((aligned(2)))
const struct usb_device_descriptor fs_device_descriptor = {
	.bLength            = USB_DT_DEVICE_SIZE,
	.bDescriptorType    = USB_DT_DEVICE,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 8,
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.iManufacturer      = 1,
	.iProduct           = 2,
	.bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(2)))
const struct usb_qualifier_descriptor fs_qualifier_descriptor = {
	.bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
	.bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 64,
	.bNumConfigurations = 1,
};

__attribute__((aligned(2)))
struct usb_device_config_descriptor fs_config_descriptor = {
	.cfg = {
		.bLength             = USB_DT_CONFIG_SIZE,
		.bDescriptorType     = USB_DT_CONFIG,
		.wTotalLength        = USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE +
		                       (USB_DT_ENDPOINT_SIZE * 1),
		.bNumInterfaces      = 1,
		.bConfigurationValue = 1,
		.bmAttributes        = USB_CONFIG_ATT_ONE |
		                       USB_CONFIG_ATT_SELFPOWER,
	},
	.if0 = {
		.bLength             = USB_DT_INTERFACE_SIZE,
		.bDescriptorType     = USB_DT_INTERFACE,
		.bInterfaceNumber    = 0,
		.bNumEndpoints       = 1,
	},
	.ep1 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 1 | USB_DIR_OUT,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 64,
	},
};

/* String descriptors */
__attribute__((aligned(2)))
static const struct usb_string_descriptor str0_descriptor = {
	.bLength         = 2 + 1*2,
	.bDescriptorType = USB_DT_STRING,
	.wData           = { 0x0409 },
};

__attribute__((aligned(2)))
static const struct usb_string_descriptor str1_descriptor = {
	.bLength         = 2 + (sizeof(MANUFACTURER_STRING) - 2),
	.bDescriptorType = USB_DT_STRING,
	.wData           = { MANUFACTURER_STRING },
};

__attribute__((aligned(2)))
static const struct usb_string_descriptor str2_descriptor = {
	.bLength         = 2 + (sizeof(PRODUCT_STRING) - 2),
	.bDescriptorType = USB_DT_STRING,
	.wData           = { PRODUCT_STRING },
};

const struct usb_string_descriptor *string_descriptor[NUM_STRING_DESC] = {
	&str0_descriptor,
	&str1_descriptor,
	&str2_descriptor,
};
