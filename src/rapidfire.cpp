#include "rapidfire.h"

static rapidFIRE_SPI rapidfire = rapidFIRE_SPI(PIN_CLK, PIN_MOSI, PIN_CS);

void Rapidfire::Init() {
  ModuleBase::Init();

  delay(VRX_BOOT_DELAY);

  rapidfire.begin();

  m_lastDisplayTime = 0;

  DBGLN("Rapid Fire init");
}

bool Rapidfire::BufferIsClear(char *payload, int len) {
  for (int i = 0; i < len; i++) {
    if (payload[i] != 0) {
      return false;
    }
  }

  return true;
}

void Rapidfire::SetOSD(mspPacket_t *packet) {
  int len = packet->payloadSize;

  if (len < 4) {
    return;
  }

  packet->readByte(); // skip
  packet->readByte(); // row, not used
  packet->readByte(); // col, not used
  packet->readByte(); // skip

  len -= 4; // subtract the first 4 bytes

  std::vector<char> messageBuffer(len);
  for (int i = 0; i < len; i++) { // Read entire payload into temporary buffer
    messageBuffer[i] = packet->readByte();
  }

  if (BufferIsClear(messageBuffer.data(), len)) {
    return;
  }

  QueueMessageParts(messageBuffer, len, CountMessageParts(messageBuffer, len));
}

int Rapidfire::CountMessageParts(std::vector<char> &buffer, int len) {
  int numParts = 0;
  for (int offset = 0; offset < len; offset += RAPIDFIRE_MAX_LENGTH_TEXT) {
    int partSize = std::min(RAPIDFIRE_MAX_LENGTH_TEXT, len - offset);
    if (partSize > 0 && !BufferIsClear(buffer.data() + offset, partSize)) {
      numParts++;
    }
  }
  return numParts;
}

void Rapidfire::QueueMessageParts(std::vector<char> &buffer, int len,
                                  int numParts) {
  int currentPart = 0;
  for (int offset = 0; offset < len; offset += RAPIDFIRE_MAX_LENGTH_TEXT) {
    int partSize = std::min(RAPIDFIRE_MAX_LENGTH_TEXT, len - offset);

    if (partSize <= 0 || BufferIsClear(buffer.data() + offset, partSize)) {
      continue;
    }

    std::string message;
    if (numParts > 1) {
      currentPart++;
      message = "[" + std::to_string(currentPart) + "/" +
                std::to_string(numParts) + "] ";
      partSize =
          std::min(partSize, RAPIDFIRE_MAX_LENGTH_TEXT - (int)message.length());
    }

    message += std::string(buffer.begin() + offset,
                           buffer.begin() + offset + partSize);

    if (m_displayQueue.size() >= MAX_QUEUE_SIZE) {
      m_displayQueue.pop();
    }

    m_displayQueue.push(message);
  }
}

void Rapidfire::SetRecordingState(uint8_t recordingState, uint16_t _delay) {
  if (recordingState != 0) {
    m_displayQueue.push("Recording: On");
  } else {
    m_displayQueue.push("Recording: Off");
  }
}

void Rapidfire::SendHeadTrackingEnableCmd(bool enable) {
  if (enable) {
    m_displayQueue.push("Head Tracking: Enabled");
  } else {
    m_displayQueue.push("Head Tracking: Disabled");
  }
}

void Rapidfire::SendLinkTelemetry(uint8_t *rawCrsfPacket) {
#ifdef TELEMETRY_ON_OSD

  uint8_t rssi = rawCrsfPacket[3] * -1;
  uint8_t lq = rawCrsfPacket[5] / 3;
  uint8_t snr = rawCrsfPacket[6]; // actually int8_t

  m_displayQueue.push("RSSI: " + std::to_string(rssi) + "dB, LQ: " +
                      std::to_string(lq) + "%, SNR: " + std::to_string(snr));
#endif
}

void Rapidfire::SendBatteryTelemetry(uint8_t *rawCrsfPacket) {
#ifdef TELEMETRY_ON_OSD

  uint16_t voltage = ((uint16_t)rawCrsfPacket[3] << 8 | rawCrsfPacket[4]);
  uint16_t amperage = ((uint16_t)rawCrsfPacket[5] << 8 | rawCrsfPacket[6]);
  uint32_t mah = ((uint32_t)rawCrsfPacket[7] << 16 |
                  (uint32_t)rawCrsfPacket[8] << 8 | rawCrsfPacket[9]);
  uint8_t percentage = rawCrsfPacket[10];

  m_displayQueue.push("Battery: " + std::to_string(voltage) + "V, " +
                      std::to_string(amperage) + "A, " + std::to_string(mah) +
                      "mAh, " + std::to_string(percentage) + "%");
#endif
}

void Rapidfire::SendGpsTelemetry(crsf_packet_gps_t *packet) {
#ifdef TELEMETRY_ON_OSD

  int32_t rawLat = be32toh(packet->p.lat); // Convert to host byte order
  int32_t rawLon = be32toh(packet->p.lon); // Convert to host byte order

  m_displayQueue.push("GPS: " + std::to_string(static_cast<double>(rawLat)) +
                      ", " + std::to_string(static_cast<double>(rawLon)));
#endif
}

void Rapidfire::SendIndexCmd(uint8_t index) {
  uint8_t newBand = index / 8 + 1;
  uint8_t newChannel = index % 8;

  newChannel++; // ELRS channel is zero based, need to add 1

  rapidFIRE_SPI::BAND imrcBand; // convert ELRS band index to IMRC band index:
  switch (newBand) {
  case 0x01: // Boscam A
    imrcBand = rapidFIRE_SPI::BAND::BAND_A;
    m_displayQueue.push("Band A:" + std::to_string(newChannel));
    break;
  case 0x02: // Boscam B
    imrcBand = rapidFIRE_SPI::BAND::BAND_B;
    m_displayQueue.push("Band B:" + std::to_string(newChannel));
    break;
  case 0x03: // Boscam E
    imrcBand = rapidFIRE_SPI::BAND::BAND_E;
    m_displayQueue.push("Band E:" + std::to_string(newChannel));
    break;
  case 0x04: // ImmersionRC/FatShark
    imrcBand = rapidFIRE_SPI::BAND::BAND_F;
    m_displayQueue.push("Band F:" + std::to_string(newChannel));
    break;
  case 0x05: // RaceBand
    imrcBand = rapidFIRE_SPI::BAND::BAND_R;
    m_displayQueue.push("RaceBand:" + std::to_string(newChannel));
    break;
  case 0x06: // LowRace
    imrcBand = rapidFIRE_SPI::BAND::BAND_L;
    m_displayQueue.push("LowRace:" + std::to_string(newChannel));
    break;
  default: // ImmersionRC/FatShark
    imrcBand = rapidFIRE_SPI::BAND::BAND_F;
    m_displayQueue.push("Band F:" + std::to_string(newChannel));
    break;
  }

  for (int i = 0; i < SPAM_COUNT; i++) {
    rapidfire.setBand(imrcBand);
    delay(DELAY_BETWEEN_SPI_PKT);
    rapidfire.setChannel(newChannel);
    delay(DELAY_BETWEEN_SPI_PKT);
  }
}

void Rapidfire::ShowNextMessage() {
  if (m_displayQueue.empty()) {
    return;
  }

  if (m_lastDisplayTime == 0) { // Only set OSD if not already set
    for (int i = 0; i < SPAM_COUNT; i++) {
      rapidfire.setOSDMode(rapidFIRE_SPI::OSDMODE::USERTEXT);
      delay(DELAY_BETWEEN_SPI_PKT);
    }
  }

  memset(m_textBuffer, 0, RAPIDFIRE_MAX_LENGTH_TEXT);
  strncpy(m_textBuffer, m_displayQueue.front().c_str(),
          RAPIDFIRE_MAX_LENGTH_TEXT);
  m_displayQueue.pop();

  rapidfire.setOSDUserText(m_textBuffer);

  m_lastDisplayTime = millis();
}

void Rapidfire::ClearOSD() {
  for (int i = 0; i < SPAM_COUNT; i++) {
    rapidfire.setOSDMode(rapidFIRE_SPI::OSDMODE::OFF);
    delay(DELAY_BETWEEN_SPI_PKT);
  }

  m_lastDisplayTime = 0;
}

void Rapidfire::Loop(uint32_t now) {
  ModuleBase::Loop(now);

  if (!m_displayQueue.empty()) {
    // Show next message if display is inactive or enough time has passed
    if (now - m_lastDisplayTime >= DISPLAY_INTERVAL) {
      ShowNextMessage();
    }
  } else if (m_lastDisplayTime != 0 &&
             (now - m_lastDisplayTime >= OSD_TIMEOUT)) {
    // Clear display after timeout if queue is empty and display is active
    ClearOSD();
  }
}
