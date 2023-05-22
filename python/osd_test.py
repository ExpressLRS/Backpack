import time
import serial
import argparse
import serials_find

def crc8_dvb_s2(crc, a):
  crc = crc ^ a
  for ii in range(8):
    if crc & 0x80:
      crc = (crc << 1) ^ 0xD5
    else:
      crc = crc << 1
  return crc & 0xFF

def send_msp(port, baud, body):
  s = serial.Serial(port=port, baudrate=baud,
      bytesize=8, parity='N', stopbits=1,
      timeout=1, xonxoff=0, rtscts=0)
  crc = 0
  for x in body:
    crc = crc8_dvb_s2(crc, x)
  msp = [ord('$'),ord('X'),ord('<')]
  msp = msp + body
  msp.append(crc)
  s.write(msp)
  print(msp)
  time.sleep(2.0)
  print(s.read_all())

def send_clear(port, baud):
  msp = [0,0xb6,0x00,1,0,0x02]
  send_msp(port, baud, msp)

def send_display(port, baud):
  msp = [0,0xb6,0x00,1,0,0x04]
  send_msp(port, baud, msp)

def send_msg(port, baud, row, col, str):
  l = 4+len(str)
  msp = [0,0xb6,0x00,l%256,int(l/256),0x03,row,col,0]
  for x in [*str]:
    msp.append(ord(x))
  send_msp(port, baud, msp)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(
    description="Initialize EdgeTX passthrough to internal module")
  parser.add_argument("-b", "--baud", type=int, default=460800,
    help="Baud rate for passthrough communication")
  parser.add_argument("-p", "--port", type=str,
    help="Override serial port autodetection and use PORT")
  parser.add_argument("-c", "--clear", action='store_true',
    help="Clear the OSD")
  parser.add_argument("-d", "--display", action='store_true',
    help="Draw the currently set OSD")
  parser.add_argument("row", type=int, nargs='?', default=0,
    help="Row where message is to be displayed")
  parser.add_argument("col", type=int, nargs='?', default=0,
    help="Column where message is to be displayed")
  parser.add_argument("message", type=str, nargs='?', default='',
    help="Message to be displayed")
  args = parser.parse_args()

  if (args.port == None):
    args.port = serials_find.get_serial_port()

  if args.clear:
    send_clear(args.port, args.baud)
  elif args.display:
    send_display(args.port, args.baud)
  else:
    send_msg(args.port, args.baud, args.row, args.col, args.message)
