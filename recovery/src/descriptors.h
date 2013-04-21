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

#ifndef _DESCRIPTORS_H
#define _DESCRIPTORS_H

#include "linux/usb/ch9.h"

#define NUM_STRING_DESC 3
#define NUM_CONFIG_DESC 1

struct usb_device_config_descriptor {
	struct usb_config_descriptor cfg;
	struct usb_interface_descriptor if0;
	struct usb_endpoint_descriptor ep1;
} __attribute__((packed));

const struct usb_device_descriptor hs_device_descriptor;
const struct usb_qualifier_descriptor hs_qualifier_descriptor;
struct usb_device_config_descriptor hs_config_descriptor;

const struct usb_device_descriptor fs_device_descriptor;
const struct usb_qualifier_descriptor fs_qualifier_descriptor;
struct usb_device_config_descriptor fs_config_descriptor;

const struct usb_string_descriptor *string_descriptor[NUM_STRING_DESC];

#endif /* _DESCRIPTORS_H */
