#!/usr/bin/python

import json
import struct
import sys

def findFirmwareEnd(f):
    f.seek(0, 0)
    (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
    if magic != 0xe9:
        sys.stderr.write('The file provided does not the right magic for a firmware file!\n')
        exit(1)

    is8285 = False
    if segments == 2: # we have to assume it's an ESP8266/85
        f.seek(0x1000, 0)
        (magic, segments, _, _, _) = struct.unpack('<BBBBI', f.read(8))
        is8285 = True
    else:
        f.seek(24, 0)

    for _ in range(segments):
        (_, size) = struct.unpack('<II', f.read(8))
        f.seek(size, 1)

    pos = f.tell()
    pos = (pos + 16) & ~15
    if not is8285:
        pos = pos + 32
    return pos

def appendToFirmware(firmware_file, defines):
    end = findFirmwareEnd(firmware_file)
    firmware_file.seek(end, 0)
    defines = (defines.encode() + (b'\0' * 512))[0:512]
    firmware_file.write(defines)
    firmware_file.write(b'\0')
    firmware_file.flush()
    # firmware_file.truncate(firmware_file.tell())  # Stupid Windoze! (Permission denied)

def appendConfiguration(source, target, env):
    defines = json.JSONEncoder().encode(env['OPTIONS_JSON'])
    with open(str(target[0]), "r+b") as firmware_file:
        appendToFirmware(firmware_file, defines)
