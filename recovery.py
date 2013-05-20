#!/usr/bin/env python
# vim: ai ts=4 sts=4 et sw=4

import os
import sys
import struct
import time

root_dir = os.path.abspath(os.path.dirname(__file__))
pyusb_dir = os.path.join(root_dir, 'pyusb')
sys.path.insert(1, pyusb_dir)
if sys.platform == 'win32':
	import platform
	bits = platform.architecture()[0][:2]
	lib_dir = os.path.join(pyusb_dir, 'lib' + bits)
	os.environ['PATH'] = lib_dir + ';' + os.environ['PATH']

import usb.core
import usb.util

LOAD_COMMAND = 0
RUN_COMMAND  = 1

class Recovery(object):
    def __init__(self, device):
        self.device = device

        self.device.set_configuration()
        config_descriptor = self.device.get_active_configuration()
        interface_number = config_descriptor[(0,0)].bInterfaceNumber
        alternate_setting = usb.control.get_interface(self.device,
                interface_number)

        interface_descriptor = usb.util.find_descriptor(
            config_descriptor,
            bInterfaceNumber = interface_number,
            bAlternateSetting = alternate_setting
        )

        self.data_out = usb.util.find_descriptor(
            interface_descriptor,
            custom_match = \
            lambda e:
                usb.util.endpoint_direction(e.bEndpointAddress) == \
                usb.util.ENDPOINT_OUT
        )

    def cmd_send(self, command, data=None):
        self.device.ctrl_transfer(0x40, 0x40, command, 0, data)

    def cmd_recv(self, command, count=65535):
        assert(count > 0)
        self.device.ctrl_transfer(0xC0, 0x40, command, 0, count)

    def load(self, data, addr=0):
        length = len(data)
        self.cmd_send(LOAD_COMMAND, data=struct.pack('<II', addr, length))
        chunk_size = 64*1024
        written = 0
        while written < length:
            chunk = data[written:written + chunk_size]
            written += self.data_out.write(chunk)

    def run(self, addr=0):
        self.cmd_send(RUN_COMMAND, data=struct.pack('<I', addr))


if __name__ == '__main__':
    dev = usb.core.find(idVendor=0x0000, idProduct=0x7f20)

    if not dev:
        print "no device found"
        sys.exit(-1)

    recovery = Recovery(dev)
    data_size = 4096
    data = '\0' * (data_size * 1024)
    recovery.load(data)
    #recovery.run()

