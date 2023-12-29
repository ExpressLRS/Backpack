import sys
import time
import csv
import argparse
import serial
import random

_crc_tab = [
    0x00,0xD5,0x7F,0xAA,0xFE,0x2B,0x81,0x54,0x29,0xFC,0x56,0x83,0xD7,0x02,0xA8,0x7D,0x52,0x87,0x2D,0xF8,0xAC,0x79,0xD3,0x06,0x7B,0xAE,0x04,0xD1,0x85,0x50,0xFA,0x2F,\
    0xA4,0x71,0xDB,0x0E,0x5A,0x8F,0x25,0xF0,0x8D,0x58,0xF2,0x27,0x73,0xA6,0x0C,0xD9,0xF6,0x23,0x89,0x5C,0x08,0xDD,0x77,0xA2,0xDF,0x0A,0xA0,0x75,0x21,0xF4,0x5E,0x8B,\
    0x9D,0x48,0xE2,0x37,0x63,0xB6,0x1C,0xC9,0xB4,0x61,0xCB,0x1E,0x4A,0x9F,0x35,0xE0,0xCF,0x1A,0xB0,0x65,0x31,0xE4,0x4E,0x9B,0xE6,0x33,0x99,0x4C,0x18,0xCD,0x67,0xB2,\
    0x39,0xEC,0x46,0x93,0xC7,0x12,0xB8,0x6D,0x10,0xC5,0x6F,0xBA,0xEE,0x3B,0x91,0x44,0x6B,0xBE,0x14,0xC1,0x95,0x40,0xEA,0x3F,0x42,0x97,0x3D,0xE8,0xBC,0x69,0xC3,0x16,\
    0xEF,0x3A,0x90,0x45,0x11,0xC4,0x6E,0xBB,0xC6,0x13,0xB9,0x6C,0x38,0xED,0x47,0x92,0xBD,0x68,0xC2,0x17,0x43,0x96,0x3C,0xE9,0x94,0x41,0xEB,0x3E,0x6A,0xBF,0x15,0xC0,\
    0x4B,0x9E,0x34,0xE1,0xB5,0x60,0xCA,0x1F,0x62,0xB7,0x1D,0xC8,0x9C,0x49,0xE3,0x36,0x19,0xCC,0x66,0xB3,0xE7,0x32,0x98,0x4D,0x30,0xE5,0x4F,0x9A,0xCE,0x1B,0xB1,0x64,\
    0x72,0xA7,0x0D,0xD8,0x8C,0x59,0xF3,0x26,0x5B,0x8E,0x24,0xF1,0xA5,0x70,0xDA,0x0F,0x20,0xF5,0x5F,0x8A,0xDE,0x0B,0xA1,0x74,0x09,0xDC,0x76,0xA3,0xF7,0x22,0x88,0x5D,\
    0xD6,0x03,0xA9,0x7C,0x28,0xFD,0x57,0x82,0xFF,0x2A,0x80,0x55,0x01,0xD4,0x7E,0xAB,0x84,0x51,0xFB,0x2E,0x7A,0xAF,0x05,0xD0,0xAD,0x78,0xD2,0x07,0x53,0x86,0x2C,0xF9,\
]

def crsf_frame_crc(frame: bytes) -> int:
    crc_frame = frame[2:-1]
    return crsf_crc(crc_frame)

def crsf_crc(data: bytes) -> int:
    crc = 0
    for i in data:
        crc = _crc_tab[crc ^ i]
    return crc

def idle(duration: float, uart: serial.Serial):
    if uart:
        start = time.monotonic()
        while time.monotonic() - start < duration:
            b = uart.read(128)
            if len(b) > 0:
                sys.stdout.write(b.decode('ascii', 'replace'))
    else:
        time.sleep(duration)

def output_Crsf(uart: serial.Serial, data):
    if uart is None:
        return
    uart.write(data)

def output_Msp(uart: serial.Serial, data):
    if uart is None:
        return

    # MSPv2 header + MSP_COMMAND + flags
    buf = b'$X<\x00'
    # Command MSP_ELRS_BACKPACK_CRSF_TLM
    buf += int.to_bytes(0x0011, length=2, byteorder='little', signed=False)
    buf += int.to_bytes(len(data), length=2, byteorder='little', signed=False)
    buf += data
    buf += int.to_bytes(crsf_crc(buf[3:]), length=1, byteorder='big', signed=False)

    uart.write(buf)

def processFile(fname, interval, port, baud, jitter, output):
    random.seed()

    if port is not None:
        uart = serial.Serial(port=port, baudrate=baud, timeout=0.1)
        # Delay enough time for the MCU to reboot if DTR/RTS hooked to RST
        idle(1.0, uart)
    else:
        uart = None

    with open(fname, 'r') as csv_file:
        reader = csv.reader(csv_file)
        # Skip header. DictReader is not used for performance and simplicity
        # ['time (us)', 'GPS_fixType', ' GPS_numSat', ' GPS_coord[0]', ' GPS_coord[1]', ' GPS_altitude', ' GPS_speed (m/s)', ' GPS_ground_course', ' GPS_hdop', ' GPS_eph', ' GPS_epv', ' GPS_velned[0]', ' GPS_velned[1]', ' GPS_velned[2]']
        next(reader)

        lastTime = None
        for row in reader:
            timeS = float(row[0]) / 1e6

            if lastTime != None:
                dur = timeS - lastTime
                if dur < interval:
                    continue
                j = random.randrange(0, int(jitter * 1000.0)) / 1000.0 if jitter > 0 else 0
                idle(dur + j, uart)

            # All values here are immediately loaded into their CRSF representations
            sats = int(row[2])
            lat = int(float(row[3]) * 1e7)
            lon = int(float(row[4]) * 1e7)
            alt = int(row[5]) + 1000
            spd = int(float(row[6]) * 360) # m/s to km/h*10
            hdg = int(float(row[7]) * 100) # deg to centideg
            print(f'{timeS:0.6f} GPS: ({lat},{lon}) {alt-1000}m {sats}sats')

            # CRSF_SYNC, len, type (GPS), payload, crc
            buf = bytearray(b'\xc8\x00\x02')
            buf += int.to_bytes(lat, length=4, byteorder='big', signed=True)
            buf += int.to_bytes(lon, length=4, byteorder='big', signed=True)
            buf += int.to_bytes(spd, length=2, byteorder='big', signed=False)
            buf += int.to_bytes(hdg, length=2, byteorder='big', signed=False)
            buf += int.to_bytes(alt, length=2, byteorder='big', signed=False)
            buf += int.to_bytes(sats, length=1, byteorder='big', signed=False)
            buf += b'\x00' # place for the CRC
            buf[-1] = crsf_frame_crc(buf)
            buf[1] = len(buf) - 2
            output(uart, buf)

            lastTime = timeS

parser = argparse.ArgumentParser(
    prog='gpscsv_to_crsf',
    description='Convert iNav GPS CSV to CRSF and send in real time'
)
parser.add_argument('filename')
parser.add_argument('-b', '--baud',
                    default='460800',
                    help='Baud rate used with sending CRSF data to UART')
parser.add_argument('-P', '--port',\
                    help='UART to send CRSF data to')
parser.add_argument('-I', '--interval',
                    default=5.0,
                    type=float,
                    help='If non-zero, skip GPS frames so that GPS updates are limited to this interval (s)')
parser.add_argument('-J', '--jitter',
                    default=0.0,
                    type=float,
                    help='Adds random jitter to the GPS output to simulate real world telemetry delay (s)')
parser.add_argument('-m', '--msp',
                    dest='output',
                    action='store_const', const=output_Msp, default=output_Crsf,
                    help='Wrap CRSF with MSP when sending to UART (default: send just CRSF)')

args = parser.parse_args()
processFile(args.filename, args.interval, args.port, args.baud, args.jitter, args.output)