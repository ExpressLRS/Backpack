#!/usr/bin/python

from enum import Enum
import shutil
import sys
import os
import argparse
import hashlib
import json
from json import JSONEncoder
from random import randint
from os.path import dirname
import gzip

import UnifiedConfiguration
import serials_find
import upload_via_esp8266_backpack

sys.path.append(dirname(__file__) + '/external/esptool')
from external.esptool import esptool


class ElrsUploadResult:
    # SUCCESS
    Success = 0
    # ERROR: Unspecified
    ErrorGeneral = -1
    # ERROR: target mismatch
    ErrorMismatch = -2

class DeviceType(Enum):
    VRX = 'vrx'
    TXBP = 'txbp'
    TIMER = 'timer'
    def __str__(self):
        return self.value

class MCUType(Enum):
    ESP8266 = 'esp8266'
    ESP32 = 'esp32'
    def __str__(self):
        return self.value

class UploadMethod(Enum):
    uart = 'uart'
    passthru = 'passthru'
    edgetx = 'etx'
    wifi = 'wifi'
    dir = 'dir'
    def __str__(self):
        return self.value

def upload_wifi(args, mcuType, upload_addr):
    if args.port:
        upload_addr = [args.port]
    try:
        if mcuType == MCUType.ESP8266:
            with open(args.file.name, 'rb') as f_in:
                with gzip.open('firmware.bin.gz', 'wb') as f_out:
                    shutil.copyfileobj(f_in, f_out)
            upload_via_esp8266_backpack.do_upload('firmware.bin.gz', upload_addr, False, {})
        else:
            upload_via_esp8266_backpack.do_upload(args.file.name, upload_addr, False, {})
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp8266_uart(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        esptool.main(['--chip', 'esp8266', '--port', args.port, '--baud', str(args.baud), '--after', 'soft_reset', 'write_flash', '0x0000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp8266_etx(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        esptool.main(['--passthrough', '--chip', 'esp8266', '--port', args.port, '--baud', '460800', '--before', 'etx', '--after', 'hard_reset', 'write_flash', '0x0000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp8266_passthru(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        esptool.main(['--passthrough', '--chip', 'esp8266', '--port', args.port, '--baud', '230400', '--before', 'passthru', '--after', 'hard_reset', 'write_flash', '0x0000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp32_uart(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        dir = os.path.dirname(args.file.name)
        start_addr = '0x0000' if args.platform.startswith('esp32-') else '0x1000'
        esptool.main(['--chip', args.platform.replace('-', ''), '--port', args.port, '--baud', str(args.baud), '--after', 'hard_reset', 'write_flash', '-z', '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect', start_addr, os.path.join(dir, 'bootloader.bin'), '0x8000', os.path.join(dir, 'partitions.bin'), '0xe000', os.path.join(dir, 'boot_app0.bin'), '0x10000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp32_etx(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        dir = os.path.dirname(args.file.name)
        start_addr = '0x0000' if args.platform.startswith('esp32-') else '0x1000'
        esptool.main(['--passthrough', '--chip', args.platform.replace('-', ''), '--port', args.port, '--baud', '460800', '--before', 'etx', '--after', 'hard_reset', 'write_flash', '-z', '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect', start_addr, os.path.join(dir, 'bootloader.bin'), '0x8000', os.path.join(dir, 'partitions.bin'), '0xe000', os.path.join(dir, 'boot_app0.bin'), '0x10000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_esp32_passthru(args):
    if args.port == None:
        args.port = serials_find.get_serial_port()
    try:
        dir = os.path.dirname(args.file.name)
        start_addr = '0x0000' if args.platform.startswith('esp32-') else '0x1000'
        esptool.main(['--passthrough', '--chip', args.platform.replace('-', ''), '--port', args.port, '--baud', '230400', '--before', 'passthru', '--after', 'hard_reset', 'write_flash', '-z', '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect', start_addr, os.path.join(dir, 'bootloader.bin'), '0x8000', os.path.join(dir, 'partitions.bin'), '0xe000', os.path.join(dir, 'boot_app0.bin'), '0x10000', args.file.name])
    except:
        return ElrsUploadResult.ErrorGeneral
    return ElrsUploadResult.Success

def upload_dir(mcuType, args):
    if mcuType == MCUType.ESP8266:
        with open(args.file.name, 'rb') as f_in:
            with gzip.open(os.path.join(args.out, 'firmware.bin.gz'), 'wb') as f_out:
                shutil.copyfileobj(f_in, f_out)
    elif mcuType == MCUType.ESP32:
        dir = os.path.dirname(args.file.name)
        shutil.copy2(args.file.name, args.out)
        shutil.copy2(os.path.join(dir, 'bootloader.bin'), args.out)
        shutil.copy2(os.path.join(dir, 'partitions.bin'), args.out)
        shutil.copy2(os.path.join(dir, 'boot_app0.bin'), args.out)
    return ElrsUploadResult.Success

def upload(deviceType: DeviceType, mcuType: MCUType, args):
    if args.baud == 0:
        args.baud = 460800

    if args.flash == UploadMethod.dir:
        return upload_dir(mcuType, args)
    elif deviceType == DeviceType.VRX:
        if mcuType == MCUType.ESP8266:
            if args.flash == UploadMethod.uart:
                return upload_esp8266_uart(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_vrx', 'elrs_vrx.local'])
        elif mcuType == MCUType.ESP32:
            if args.flash == UploadMethod.uart:
                return upload_esp32_uart(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_vrx', 'elrs_vrx.local'])
    elif deviceType == DeviceType.TIMER:
        if mcuType == MCUType.ESP8266:
            if args.flash == UploadMethod.uart:
                return upload_esp8266_uart(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_timer', 'elrs_timer.local'])
        elif mcuType == MCUType.ESP32:
            if args.flash == UploadMethod.uart:
                return upload_esp32_uart(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_timer', 'elrs_timer.local'])
    else:
        if mcuType == MCUType.ESP8266:
            if args.flash == UploadMethod.edgetx:
                return upload_esp8266_etx(args)
            elif args.flash == UploadMethod.uart:
                return upload_esp8266_uart(args)
            elif args.flash == UploadMethod.passthru:
                return upload_esp8266_passthru(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_txbp', 'elrs_txbp.local'])
        if mcuType == MCUType.ESP32:
            if args.flash == UploadMethod.edgetx:
                return upload_esp32_etx(args)
            elif args.flash == UploadMethod.uart:
                return upload_esp32_uart(args)
            elif args.flash == UploadMethod.passthru:
                return upload_esp32_passthru(args)
            elif args.flash == UploadMethod.wifi:
                return upload_wifi(args, mcuType, ['elrs_txbp', 'elrs_txbp.local'])
    print("Invalid upload method for firmware")
    return ElrsUploadResult.ErrorGeneral

def length_check(l, f):
    def x(s):
        if (len(s) > l):
            raise argparse.ArgumentTypeError(f'too long, {l} chars max')
        else:
            return s
    return x

class readable_dir(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        prospective_dir=values
        if not os.path.isdir(prospective_dir):
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a valid path".format(prospective_dir))
        if os.access(prospective_dir, os.R_OK):
            setattr(namespace,self.dest,prospective_dir)
        else:
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a readable dir".format(prospective_dir))

class writeable_dir(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        prospective_dir=values
        if not os.path.isdir(prospective_dir):
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a valid path".format(prospective_dir))
        if os.access(prospective_dir, os.W_OK):
            setattr(namespace,self.dest,prospective_dir)
        else:
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a writeable dir".format(prospective_dir))

def main():
    parser = argparse.ArgumentParser(description="Flash Binary Firmware")
    # firmware/targets directory
    parser.add_argument('--dir', action=readable_dir, default=None)
    # Bind phrase
    parser.add_argument('--phrase', type=str, help='Your personal binding phrase')
    # WiFi Params
    parser.add_argument('--ssid', type=length_check(32, "ssid"), required=False, help='Home network SSID')
    parser.add_argument('--password', type=length_check(64, "password"), required=False, help='Home network password')
    parser.add_argument('--auto-wifi', type=int, help='Interval (in seconds) before WiFi auto starts, if no connection is made')
    parser.add_argument('--no-auto-wifi', action='store_true', help='Disables WiFi auto start if no connection is made')
    # Unified target
    parser.add_argument('--target', type=str, help='Unified target JSON path')
    # Flashing options
    parser.add_argument("--flash", type=UploadMethod, choices=list(UploadMethod), help="Flashing Method")
    parser.add_argument('--out', action=writeable_dir, default=None)
    parser.add_argument("--port", type=str, help="SerialPort or WiFi address to flash firmware to")
    parser.add_argument("--baud", type=int, default=0, help="Baud rate for serial communication")
    parser.add_argument("--force", action='store_true', default=False, help="Force upload even if target does not match")
    parser.add_argument("--confirm", action='store_true', default=False, help="Confirm upload if a mismatched target was previously uploaded")
    # Firmware file to patch/configure
    parser.add_argument("file", nargs="?", type=argparse.FileType("r+b"))

    args = parser.parse_args()

    if args.dir != None:
        os.chdir(args.dir)

    vendor = args.target.split('.')[0]
    hardware = args.target.split('.')[1]
    target = args.target.split('.')[2]

    with open('hardware/targets.json') as f:
        targets = json.load(f)
    args.platform = targets[vendor][hardware][target]['platform']
    mcu = MCUType.ESP8266 if args.platform == "esp8285" else MCUType.ESP32

    if args.file is None:
        srcdir = targets[vendor][hardware][target]['firmware']
        shutil.copy2(srcdir + '/firmware.bin', '.')
        if os.path.exists(srcdir + '/bootloader.bin'): shutil.copy2(srcdir + '/bootloader.bin', '.')
        if os.path.exists(srcdir + '/partitions.bin'): shutil.copy2(srcdir + '/partitions.bin', '.')
        if os.path.exists(srcdir + '/boot_app0.bin'): shutil.copy2(srcdir + '/boot_app0.bin', '.')
        args.file = open('firmware.bin', 'r+b')

    json_flags = {}
    if args.phrase is not None:
        json_flags['uid'] = bindingPhraseHash = [x for x in hashlib.md5(("-DMY_BINDING_PHRASE=\""+args.phrase+"\"").encode()).digest()[0:6]]
    if args.ssid is not None:
        json_flags['wifi-ssid'] = args.ssid
    if args.password is not None and args.ssid is not None:
        json_flags['wifi-password'] = args.password
    json_flags['flash-discriminator'] = randint(1,2**32-1)
    json_flags['product-name'] = targets[vendor][hardware][target]['product_name']
    UnifiedConfiguration.appendToFirmware(args.file, JSONEncoder().encode(json_flags))

    if hardware == 'txbp':
        devicetype = DeviceType.TXBP
    elif hardware == 'timer':
        devicetype = DeviceType.TIMER
    else:
        devicetype = DeviceType.VRX

    ret = upload(devicetype, mcu, args)
    sys.exit(ret)

if __name__ == '__main__':
    try:
        main()
    except AssertionError as e:
        print(e)
        exit(1)
