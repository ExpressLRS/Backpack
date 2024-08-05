#
# To use this program to send OSD messages to HDZero googles for example.
# 1. Ensure the VRX backpack (HDZero goggles backpack) is flashed with the latest version of it's backpack.
# 2. Flash an ESP32 device using "DEBUG_ESP32_TX_Backpack_via_UART" or similar TX backpack.
# 3. Run this python program and send commands.
#    Depending on the font used for the OSD only UPPERCASE letters may be displayed as letters.
#    This is because the other character positions are used to display other symbols on the OSD.

import serial
import argparse
import serials_find
import sys
import threading

def crc8_dvb_s2(crc, a):
  crc = crc ^ a
  for ii in range(8):
    if crc & 0x80:
      crc = (crc << 1) ^ 0xD5
    else:
      crc = crc << 1
  return crc & 0xFF

def send_msp(s, body):
  crc = 0
  for x in body:
    crc = crc8_dvb_s2(crc, x)
  msp = [ord('$'),ord('X'),ord('<')]
  msp = msp + body
  msp.append(crc)
  s.write(msp)
  print('Sending ' + str(msp))

def send_clear(s):
  msp = [0,0xb6,0x00,1,0,0x02]
  send_msp(s, msp)

def send_display(s):
  msp = [0,0xb6,0x00,1,0,0x04]
  send_msp(s, msp)

def send_msg(s, row, col, str):
  l = 4+len(str)
  msp = [0,0xb6,0x00,l%256,int(l/256),0x03,row,col,0]
  for x in [*str]:
    msp.append(ord(x))
  send_msp(s, msp)

def thread_function(s: serial.Serial):
  while True:
    b = s.readall()
    if len(b): print(b)

def short_help():
  print("Command should be one of:")
  print("C = clear the OSD canvas")
  print("D = display the OSD canvas")
  print("H = Print the full help message")
  print("<row> <col> <message> = send message to OSD canvas")

def help():
  print()
  print("Depending on the OSD font only UPPERCASE letters ay display as actual letters,")
  print("this is because the other character positions are used to display other symbols on the OSD.")
  short_help()
  print()
  print("Example:")
  print("C")
  print("10 10 ELRS ROCKS")
  print("D")

if __name__ == '__main__':
  parser = argparse.ArgumentParser(
    description="Send OSD messages to a VRX backpack via a TX backpack connected to a Serial port")
  parser.add_argument("-b", "--baud", type=int, default=460800,
    help="Baud rate for passthrough communication")
  parser.add_argument("-p", "--port", type=str,
    help="Override serial port autodetection and use PORT")
  args = parser.parse_args()

  if (args.port == None):
    args.port = serials_find.get_serial_port()

  s = serial.Serial(port=args.port, baudrate=args.baud, bytesize=8, parity='N', stopbits=1, timeout=1, xonxoff=0, rtscts=0)
  threading.Thread(target=thread_function, args=(s,)).start()

  help()
  for line in sys.stdin:
    if line.upper().startswith('C'):
      send_clear(s)
    elif line.upper().startswith('D'):
      send_display(s)
    elif line.upper().startswith('H'):
      help()
    else:
      import re
      parts=re.search('(\d+) (\d+) (.+)', line)
      if parts and len(parts.groups()) == 3:
        row = int(parts.group(1))
        col = int(parts.group(2))
        message = parts.group(3)
        send_msg(s, row, col, message)
      else:
        short_help()
