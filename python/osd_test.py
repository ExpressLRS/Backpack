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

def format_bytes(data):
  return " ".join(f"{b:02X}" for b in data)

def send_msp(s, body):
  crc = 0
  for x in body:
    crc = crc8_dvb_s2(crc, x)
  msp = bytes([ord('$'), ord('X'), ord('<')] + body + [crc])
  s.write(msp)
  print("Sending " + format_bytes(msp))

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

class MspReader:
  def __init__(self):
    self.buffer = bytearray()
    self.text_buffer = bytearray()

  def _flush_text(self, chunk):
    events = []
    if not chunk:
      return events
    self.text_buffer.extend(chunk)
    while True:
      newline_pos = -1
      for marker in (b"\n", b"\r"):
        pos = self.text_buffer.find(marker)
        if pos != -1 and (newline_pos == -1 or pos < newline_pos):
          newline_pos = pos
      if newline_pos == -1:
        break
      line = bytes(self.text_buffer[:newline_pos]).decode("utf-8", errors="replace").strip()
      del self.text_buffer[:newline_pos + 1]
      if line:
        events.append(("text", line))
    return events

  def feed(self, data):
    self.buffer.extend(data)
    events = []
    while True:
      start = self.buffer.find(b"$X")
      if start < 0:
        events.extend(self._flush_text(self.buffer))
        self.buffer.clear()
        break
      if start > 0:
        events.extend(self._flush_text(self.buffer[:start]))
        del self.buffer[:start]
      if len(self.buffer) < 9:
        break
      direction = self.buffer[2]
      if direction not in (ord("<"), ord(">"), ord("!")):
        events.extend(self._flush_text(self.buffer[:1]))
        del self.buffer[0]
        continue
      payload_size = self.buffer[6] | (self.buffer[7] << 8)
      frame_size = 9 + payload_size
      if len(self.buffer) < frame_size:
        break
      frame = bytes(self.buffer[:frame_size])
      del self.buffer[:frame_size]
      body = frame[3:-1]
      expected_crc = frame[-1]
      crc = 0
      for value in body:
        crc = crc8_dvb_s2(crc, value)
      events.append(("packet", frame, crc == expected_crc))
    return events

def describe_packet(frame, crc_valid):
  direction = chr(frame[2])
  function = frame[5] << 8 | frame[4]
  payload_size = frame[6] | (frame[7] << 8)
  payload = frame[8:8 + payload_size]
  status = "OK" if crc_valid else "BAD CRC"
  prefix = f"Received {direction} 0x{function:04X} [{status}]"
  if payload:
    return f"{prefix}: {format_bytes(payload)}"
  return prefix

def thread_function(s: serial.Serial):
  reader = MspReader()
  while True:
    data = s.read(s.in_waiting or 1)
    if not data:
      continue
    for event in reader.feed(data):
      if event[0] == "text":
        print(f"Serial log: {event[1]}")
      else:
        _, frame, crc_valid = event
        print(describe_packet(frame, crc_valid))

def short_help():
  print("Command should be one of:")
  print("C = clear the OSD canvas")
  print("D = display the OSD canvas")
  print("H = Print the full help message")
  print("<row> <col> <message> = send message to OSD canvas")

def help():
  print()
  print("Depending on the OSD font only UPPERCASE letters may display as actual letters,")
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
  threading.Thread(target=thread_function, args=(s,), daemon=True).start()

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
