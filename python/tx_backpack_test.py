#
# To use this program to send TX backpack MSP commands over a serial connection.
# 1. Flash an ESP32 device using a TX backpack target such as
#    "DEBUG_ESP32_TX_Backpack_via_UART".
# 2. Connect the device over USB/UART.
# 3. Run this python program and enter commands to exercise the TX backpack API.

import argparse
import queue
import re
import sys
import threading
import time

import serial

import serials_find

MSP_SET_VTX_CONFIG = 89

MSP_ELRS_BIND = 0x0009
MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE = 0x000C
MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE = 0x000D
MSP_ELRS_GET_BACKPACK_VERSION = 0x0010
MSP_ELRS_BACKPACK_CRSF_TLM = 0x0011
MSP_ELRS_BACKPACK_CONFIG = 0x0030
MSP_ELRS_BACKPACK_CONFIG_TLM_MODE = 0x31

MSP_ELRS_BACKPACK_SET_RECORDING_STATE = 0x0305
MSP_ELRS_BACKPACK_SET_HEAD_TRACKING = 0x030D
MSP_ELRS_BACKPACK_SET_PTR = 0x0383

COMMAND_NAMES = {
  MSP_SET_VTX_CONFIG: "MSP_SET_VTX_CONFIG",
  MSP_ELRS_BIND: "MSP_ELRS_BIND",
  MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE: "MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE",
  MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE: "MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE",
  MSP_ELRS_GET_BACKPACK_VERSION: "MSP_ELRS_GET_BACKPACK_VERSION",
  MSP_ELRS_BACKPACK_CRSF_TLM: "MSP_ELRS_BACKPACK_CRSF_TLM",
  MSP_ELRS_BACKPACK_CONFIG: "MSP_ELRS_BACKPACK_CONFIG",
  MSP_ELRS_BACKPACK_SET_RECORDING_STATE: "MSP_ELRS_BACKPACK_SET_RECORDING_STATE",
  MSP_ELRS_BACKPACK_SET_HEAD_TRACKING: "MSP_ELRS_BACKPACK_SET_HEAD_TRACKING",
  MSP_ELRS_BACKPACK_SET_PTR: "MSP_ELRS_BACKPACK_SET_PTR",
}

RESPONSE_QUEUE = queue.Queue()
RAW_QUEUE = queue.Queue()


def crc8_dvb_s2(crc, a):
  crc = crc ^ a
  for _ in range(8):
    if crc & 0x80:
      crc = (crc << 1) ^ 0xD5
    else:
      crc = crc << 1
  return crc & 0xFF


def format_bytes(data):
  return " ".join(f"{b:02X}" for b in data)


def function_name(function):
  return COMMAND_NAMES.get(function, f"0x{function:04X}")


def build_msp_body(function, payload):
  payload = bytes(payload)
  length = len(payload)
  return bytes([
    0,
    function & 0xFF,
    (function >> 8) & 0xFF,
    length & 0xFF,
    (length >> 8) & 0xFF,
  ]) + payload


def send_msp(port, function, payload=b"", direction="<"):
  if direction not in ("<", ">"):
    raise ValueError("direction must be '<' or '>'")
  body = build_msp_body(function, payload)
  crc = 0
  for value in body:
    crc = crc8_dvb_s2(crc, value)
  frame = bytes([ord("$"), ord("X"), ord(direction)]) + body + bytes([crc])
  port.write(frame)
  print(f"Sent {direction} {function_name(function)}: {format_bytes(frame)}")


def send_get_version_request(port):
  send_msp(port, MSP_ELRS_GET_BACKPACK_VERSION)


def clear_pending_packets():
  while True:
    try:
      RESPONSE_QUEUE.get_nowait()
    except queue.Empty:
      break
  while True:
    try:
      RAW_QUEUE.get_nowait()
    except queue.Empty:
      return


def wait_for_packet(function, direction=None, timeout_seconds=1.0):
  deadline = time.monotonic() + timeout_seconds
  while True:
    remaining = deadline - time.monotonic()
    if remaining <= 0:
      return None
    try:
      frame, crc_valid = RESPONSE_QUEUE.get(timeout=remaining)
    except queue.Empty:
      return None
    packet_direction = chr(frame[2])
    packet_function = frame[5] << 8 | frame[4]
    if crc_valid and packet_function == function and (direction is None or packet_direction == direction):
      return frame, crc_valid


def wait_for_packet_raw(function, direction=None, timeout_seconds=1.0):
  deadline = time.monotonic() + timeout_seconds
  buffer = bytearray()
  while True:
    remaining = deadline - time.monotonic()
    if remaining <= 0:
      return None, bytes(buffer)
    try:
      data = RAW_QUEUE.get(timeout=remaining)
    except queue.Empty:
      return None, bytes(buffer)
    buffer.extend(data)
    while True:
      start = buffer.find(b"$X")
      if start < 0:
        if len(buffer) > 1:
          del buffer[:-1]
        break
      if start > 0:
        del buffer[:start]
      if len(buffer) < 9:
        break
      packet_direction = chr(buffer[2])
      if packet_direction not in ("<", ">", "!"):
        del buffer[0]
        continue
      payload_size = buffer[6] | (buffer[7] << 8)
      frame_size = 9 + payload_size
      if len(buffer) < frame_size:
        break
      frame = bytes(buffer[:frame_size])
      del buffer[:frame_size]
      body = frame[3:-1]
      crc = 0
      for value in body:
        crc = crc8_dvb_s2(crc, value)
      crc_valid = crc == frame[-1]
      packet_function = frame[5] << 8 | frame[4]
      if crc_valid and packet_function == function and (direction is None or packet_direction == direction):
        return (frame, crc_valid), bytes(buffer)


def send_tx_wifi(port):
  send_msp(port, MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE, [0])


def send_vrx_wifi(port):
  send_msp(port, MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE, [0])


def send_head_tracking(port, enabled):
  send_msp(port, MSP_ELRS_BACKPACK_SET_HEAD_TRACKING, [1 if enabled else 0])


def send_recording_state(port, enabled, delay_seconds):
  if not 0 <= delay_seconds <= 0xFFFF:
    raise ValueError("delay must be between 0 and 65535 seconds")
  send_msp(port, MSP_ELRS_BACKPACK_SET_RECORDING_STATE, [
    1 if enabled else 0,
    delay_seconds & 0xFF,
    (delay_seconds >> 8) & 0xFF,
  ])


def send_bind(port, uid):
  if len(uid) != 6:
    raise ValueError("UID must be exactly 6 bytes")
  send_msp(port, MSP_ELRS_BIND, uid)


def send_backpack_tlm_mode(port, tlm_mode):
  if not 0 <= tlm_mode <= 0xFF:
    raise ValueError("telemetry mode must be between 0 and 255")
  send_msp(port, MSP_ELRS_BACKPACK_CONFIG, [
    MSP_ELRS_BACKPACK_CONFIG_TLM_MODE,
    tlm_mode,
  ])


def send_crsf_tlm(port, payload):
  if not payload:
    raise ValueError("CRSF requires at least one byte")
  for value in payload:
    if not 0 <= value <= 0xFF:
      raise ValueError("CRSF bytes must be between 0 and 255")
  send_msp(port, MSP_ELRS_BACKPACK_CRSF_TLM, payload)


def send_ptr(port, values):
  if not values:
    raise ValueError("PTR requires at least one channel value")
  payload = bytearray()
  for value in values:
    if not 0 <= value <= 0xFFFF:
      raise ValueError("PTR values must be between 0 and 65535")
    payload.append(value & 0xFF)
    payload.append((value >> 8) & 0xFF)
  send_msp(port, MSP_ELRS_BACKPACK_SET_PTR, payload)


def send_raw(port, function, payload):
  if not 0 <= function <= 0xFFFF:
    raise ValueError("function must be between 0 and 65535")
  for value in payload:
    if not 0 <= value <= 0xFF:
      raise ValueError("payload bytes must be between 0 and 255")
  send_msp(port, function, payload)


def send_vtx_config(port, channel_index, power=None, pitmode=None):
  if not 0 <= channel_index <= 47:
    raise ValueError("channel index must be between 0 and 47")
  payload = [channel_index, 0]
  if power is not None:
    if not 0 <= power <= 0xFF:
      raise ValueError("power must be between 0 and 255")
    payload.append(power)
    if pitmode is None:
      pitmode = 0
    if not 0 <= pitmode <= 0xFF:
      raise ValueError("pitmode must be between 0 and 255")
    payload.append(pitmode)
  elif pitmode is not None:
    raise ValueError("pitmode requires a power byte as well")
  send_msp(port, MSP_SET_VTX_CONFIG, payload)


def parse_int(token):
  return int(token, 0)


def parse_bool(token):
  value = token.strip().lower()
  if value in ("1", "on", "true", "enable", "enabled", "yes"):
    return True
  if value in ("0", "off", "false", "disable", "disabled", "no"):
    return False
  raise ValueError(f"invalid boolean value: {token}")


def parse_uid(token):
  cleaned = re.sub(r"[^0-9a-fA-F]", "", token)
  if len(cleaned) != 12:
    raise ValueError("UID must contain exactly 12 hex digits")
  return bytes.fromhex(cleaned)


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

  def flush_partial_text(self):
    return []

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
  prefix = f"Received {direction} {function_name(function)} [{status}]"
  if function == MSP_SET_VTX_CONFIG and payload:
    details = [f"index={payload[0]}"]
    if len(payload) >= 4:
      details.append(f"power={payload[2]}")
      details.append(f"pitmode={payload[3]}")
    return f"{prefix}: {' '.join(details)} raw={format_bytes(payload)}"
  if function == MSP_ELRS_GET_BACKPACK_VERSION and payload:
    version = payload.split(b"\0", 1)[0].decode("ascii", errors="replace")
    return f"{prefix}: version='{version}' raw={format_bytes(payload)}"
  if function == MSP_ELRS_BACKPACK_SET_PTR and len(payload) % 2 == 0 and payload:
    values = []
    for index in range(0, len(payload), 2):
      values.append(str(payload[index] | (payload[index + 1] << 8)))
    return f"{prefix}: ptr=[{', '.join(values)}]"
  if payload:
    return f"{prefix}: {format_bytes(payload)}"
  return prefix


def thread_function(port):
  reader = MspReader()
  while True:
    data = port.read(port.in_waiting or 1)
    if not data:
      continue
    RAW_QUEUE.put(data)
    for event in reader.feed(data):
      if event[0] == "text":
        print(f"Serial log: {event[1]}")
      else:
        _, frame, crc_valid = event
        RESPONSE_QUEUE.put((frame, crc_valid))
        print(describe_packet(frame, crc_valid))


def short_help():
  print("Command should be one of:")
  print("V | VERSION = request backpack version from Tx_main.cpp")
  print("WT | TXWIFI = switch the TX backpack into Wi-Fi mode")
  print("WV | VRXWIFI = forward VRX backpack Wi-Fi mode to peers")
  print("HT <0|1> = cache and forward head tracking state; valid values: 0=off, 1=on")
  print("BIND <uid_hex> = 6-byte UID/group address; exactly 12 hex digits, separators optional")
  print("TLM <mode> = telemetry mode; valid handled values: 0=off, 1=espnow, 2=wifi")
  print("CRSF <byte ...> = raw telemetry bytes; each byte 0-255, use hex like 0xEA if helpful")
  print("VTX <channel_index> [power] [pitmode] = channel_index 0-47; power/pitmode are optional bytes 0-255")
  print("PTR <value> [value ...] = 16-bit channel values; each value 0-65535")
  print("REC <0|1> [delay_seconds] = DVR state 0=stop, 1=start; delay 0-65535 seconds")
  print("RAW <function> [byte ...] = raw MSP function 0-65535 and payload bytes 0-255")
  print("H | HELP = print the full help message")
  print("Q | QUIT = exit")


def help():
  print()
  print("TX backpack test application")
  print()
  print("This sends MSP packets to src/Tx_main.cpp over the serial port.")
  print("Commands listed above are either handled directly there or forwarded to ESP-NOW peers.")
  print("Integer values accept decimal or hex, e.g. 17 or 0x11.")
  print("REC defaults to a delay of 0 seconds if omitted.")
  print("VTX channel_index uses the standard 48-channel layout: index = band*8 + channel offset, so 0-47.")
  print("TLM mode 3 exists in the enum as bluetooth, but Tx_main.cpp only handles 0, 1, and 2.")
  print("CRSF payloads are forwarded as-is; VRX_main.cpp expects at least 4 bytes and only decodes known telemetry frame types.")
  short_help()
  print()
  print("Examples:")
  print("VERSION")
  print("HT 1")
  print("BIND 01:23:45:67:89:AB")
  print("TLM 2")
  print("CRSF 0xEE 0x06 0x32")
  print("VTX 27 3 0")
  print("PTR 1500 1500 1000")
  print("REC 1 30")
  print("RAW 0x030D 0x01")


def handle_command(port, line):
  parts = line.strip().split()
  if not parts:
    return True

  command = parts[0].upper()
  try:
    if command in ("Q", "QUIT", "EXIT"):
      return False
    if command in ("H", "HELP", "?"):
      help()
    elif command in ("V", "VERSION"):
      if len(parts) != 1:
        raise ValueError("VERSION does not take any arguments")
      clear_pending_packets()
      send_get_version_request(port)
      response, remaining = wait_for_packet_raw(MSP_ELRS_GET_BACKPACK_VERSION, direction=">", timeout_seconds=1.5)
      if response is None:
        if remaining:
          print(f"Raw bytes seen: {format_bytes(remaining)}")
        print("No version response received within 1.5 seconds")
      else:
        frame, crc_valid = response
        print(describe_packet(frame, crc_valid))
    elif command in ("WT", "TXWIFI"):
      send_tx_wifi(port)
    elif command in ("WV", "VRXWIFI"):
      send_vrx_wifi(port)
    elif command == "HT":
      if len(parts) != 2:
        raise ValueError("HT requires exactly one argument")
      send_head_tracking(port, parse_bool(parts[1]))
    elif command == "REC":
      if len(parts) not in (2, 3):
        raise ValueError("REC requires 1 or 2 arguments")
      enabled = parse_bool(parts[1])
      delay_seconds = parse_int(parts[2]) if len(parts) == 3 else 0
      send_recording_state(port, enabled, delay_seconds)
    elif command == "BIND":
      if len(parts) != 2:
        raise ValueError("BIND requires exactly one UID argument")
      send_bind(port, parse_uid(parts[1]))
    elif command == "TLM":
      if len(parts) != 2:
        raise ValueError("TLM requires exactly one telemetry mode argument")
      send_backpack_tlm_mode(port, parse_int(parts[1]))
    elif command == "CRSF":
      if len(parts) < 2:
        raise ValueError("CRSF requires at least one payload byte")
      send_crsf_tlm(port, [parse_int(token) for token in parts[1:]])
    elif command == "PTR":
      if len(parts) < 2:
        raise ValueError("PTR requires at least one channel value")
      send_ptr(port, [parse_int(token) for token in parts[1:]])
    elif command == "VTX":
      if len(parts) not in (2, 3, 4):
        raise ValueError("VTX requires 1, 2, or 3 arguments")
      channel_index = parse_int(parts[1])
      power = parse_int(parts[2]) if len(parts) >= 3 else None
      pitmode = parse_int(parts[3]) if len(parts) == 4 else None
      send_vtx_config(port, channel_index, power, pitmode)
    elif command == "RAW":
      if len(parts) < 2:
        raise ValueError("RAW requires a function ID")
      function = parse_int(parts[1])
      payload = [parse_int(token) for token in parts[2:]]
      send_raw(port, function, payload)
    else:
      short_help()
  except ValueError as exc:
    print(f"Error: {exc}")
  return True


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
    description="Send TX backpack MSP commands over a serial port")
  parser.add_argument("-b", "--baud", type=int, default=460800,
    help="Baud rate for serial communication")
  parser.add_argument("-p", "--port", type=str,
    help="Override serial port autodetection and use PORT")
  args = parser.parse_args()

  if args.port is None:
    args.port = serials_find.get_serial_port()

  serial_port = serial.Serial(
    port=args.port,
    baudrate=args.baud,
    bytesize=8,
    parity='N',
    stopbits=1,
    timeout=1,
    xonxoff=0,
    rtscts=0)
  threading.Thread(target=thread_function, args=(serial_port,), daemon=True).start()

  help()
  for line in sys.stdin:
    if not handle_command(serial_port, line):
      break
