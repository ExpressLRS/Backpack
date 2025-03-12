// rapidFIRE_SPI.cpp
// Copyright 2023 GOROman.

#include "rapidFIRE_SPI.h"

rapidFIRE_SPI::rapidFIRE_SPI(int pin_SCK, int pin_DATA, int pin_SS, int freq) {
  assert(SPI_freq < RAPIDFIRE_SPI_MAX_CLOCK);

  SPI_pin_SCK = pin_SCK;
  SPI_pin_DATA = pin_DATA;
  SPI_pin_SS = pin_SS;
  SPI_freq = freq;

  spi = new SPIClass;
}

rapidFIRE_SPI::~rapidFIRE_SPI() { delete spi; }

RF_RESULT rapidFIRE_SPI::begin() {
  // Init SPI
  pinMode(SPI_pin_SCK, OUTPUT);
  pinMode(SPI_pin_DATA, OUTPUT);
  pinMode(SPI_pin_SS, OUTPUT);

  // To enable SPI mode, set CS1, CS2, CS3 high, and then within 100-400ms set
  // them all low.
  digitalWrite(SPI_pin_SCK, HIGH);
  digitalWrite(SPI_pin_DATA, HIGH);
  digitalWrite(SPI_pin_SS, HIGH);

  delay(RAPIDFIRE_SPI_MODE_ENABLE_DELAY);

  digitalWrite(SPI_pin_SCK, LOW);
  digitalWrite(SPI_pin_DATA, LOW);
  digitalWrite(SPI_pin_SS, LOW);

  delay(10);

  digitalWrite(SPI_pin_SS, HIGH);

  return RF_RESULT_OK;
}

RF_RESULT rapidFIRE_SPI::end() { return RF_RESULT_OK; }

RF_RESULT rapidFIRE_SPI::recvCommand(uint8_t *data, size_t len) {
  assert(data != NULL);
  assert(len != 0);

  if (data == NULL) {
    return RF_RESULT_ERROR;
  }

  byte recv_len = 0x00;
  byte recv_sum = 0x00;

#if defined(PLATFORM_ESP32)
  spi->begin(SPI_pin_SCK, SPI_pin_DATA, -1, SPI_pin_SS);
#else
  spi->begin();
  spi->pins(SPI_pin_SCK, SPI_pin_DATA, -1, SPI_pin_SS);
#endif

  spi->setFrequency(SPI_freq);
  spi->setDataMode(SPI_MODE0);
  spi->setBitOrder(MSBFIRST);
  spi->setHwCs(true);

  recv_len = spi->transfer(0);
  recv_sum = spi->transfer(0);
  for (int i = 0; i < len; i++) {
    data[i] = spi->transfer(0);
  }

  spi->end();
  spi->setHwCs(false);

  byte csum = recv_len;
  for (int i = 0; i < len; ++i) {
    //        Serial.printf("Read[%d]:0x%02x(%3d) '\%c'\n", i, data[i], data[i],
    //        data[i]);
    csum += data[i];
  }

  Serial.printf("CSUM:recv:%02x==calc:%02x LEN:recv:%d==%d %s\n", recv_sum,
                csum, recv_len, len, recv_sum == csum ? "" : "NG!");

  if (recv_sum != csum) {
    //        return RF_RESULT_ERROR_CHECKSUM_MISSMATCH; // Checksum missmatch.
  }
  return RF_RESULT_OK;
}

RF_RESULT rapidFIRE_SPI::sendCommand(uint16_t command, byte *data, size_t len) {
  uint8_t csum = (command >> 8) + (command & 0xff) + len;

  for (int i = 0; i < len; ++i) {
    csum += data[i];
  }

#if defined(PLATFORM_ESP32)
  spi->begin(SPI_pin_SCK, SPI_pin_DATA, -1, SPI_pin_SS);
#else
  spi->begin();
  spi->pins(SPI_pin_SCK, SPI_pin_DATA, -1, SPI_pin_SS);
#endif
  spi->setFrequency(SPI_freq);
  spi->setDataMode(SPI_MODE0);
  spi->setBitOrder(MSBFIRST);
  spi->setHwCs(true);

  spi->write16(command);
  spi->write(len);
  spi->write(csum);
  spi->writeBytes(data, len);
  spi->end();

  spi->setHwCs(false);

#if 0
  Serial.printf("sendCommand: '%c'(%02x) '%c'(%02x) LEN:%d SUM:%02x DATA:[", command>>8, command>>8, command&0xff, command&0xff, len, csum);
  for (int i = 0; i < len; ++i)
  {
    Serial.printf(" %02x", data[i]);
  }
  Serial.printf("]\n");
#endif
  return RF_RESULT_OK;
}

RF_RESULT rapidFIRE_SPI::getFirmwareVersion(QUERY_FIRMWARE_VERSION *version) {
  assert(version);
  if (version == NULL) {
    return RF_RESULT_ERROR;
  }

  RF_RESULT res = sendCommand(RAPIDFIRE_CMD_FIRMWARE_VERSION);
  if (res != RF_RESULT_OK) {
    return res;
  }
  byte data[6] = {0};
  res = recvCommand(data, sizeof(data));
  if (res == RF_RESULT_OK) {
    for (int i = 0; i < 3; ++i) {
      version->oled[i] = data[0 + i];
      version->core[i] = data[3 + i];
    }
  }

  return res;
}

RF_RESULT rapidFIRE_SPI::getRSSI(QUERY_RSSI *rssi) {
  assert(rssi);
  if (rssi == NULL) {
    return RF_RESULT_ERROR;
  }
  RF_RESULT res = sendCommand(RAPIDFIRE_CMD_RSSI);
  if (res != RF_RESULT_OK) {
    return res;
  }
  byte data[8] = {0};
  res = recvCommand(data, sizeof(data));
  if (res == RF_RESULT_OK) {
    rssi->raw_rx1 = data[0] | data[1] << 8;
    rssi->raw_rx2 = data[2] | data[3] << 8;
    rssi->scaled_rx1 = data[4] | data[5] << 8;
    rssi->scaled_rx2 = data[6] | data[7] << 8;
  }
  return res;
}

RF_RESULT rapidFIRE_SPI::getVoltage(QUERY_VOLTAGE *voltage) {
  assert(voltage);
  if (voltage == NULL) {
    return RF_RESULT_ERROR;
  }

  RF_RESULT res = sendCommand(RAPIDFIRE_CMD_VOLTAGE);
  if (res != RF_RESULT_OK) {
    return res;
  }
  byte data[2] = {0};
  res = recvCommand(data, sizeof(data));
  if (res == RF_RESULT_OK) {
    int temp = data[0] | data[1] << 8;
    voltage->voltage = (float)temp / 1000.0f;
  }

  return res;
}

RF_RESULT rapidFIRE_SPI::buzzer() {
  return sendCommand(RAPIDFIRE_CMD_SOUND_BUZZER);
}

RF_RESULT rapidFIRE_SPI::setOSDUserText(char *text) {
  return sendCommand(RAPIDFIRE_CMD_SET_OSD_USER_TEXT, (byte *)text,
                     strlen(text));
}

RF_RESULT rapidFIRE_SPI::setOSDMode(OSDMODE mode) {
  byte data[] = {(byte)mode};
  return sendCommand(RAPIDFIRE_CMD_SET_OSD_MODE, data, sizeof(data));
}

RF_RESULT rapidFIRE_SPI::setRXModule(RXMODULE module) {
  byte data[] = {(byte)module};
  return sendCommand(RAPIDFIRE_CMD_SET_RX_MODULE, data, sizeof(data));
}

RF_RESULT rapidFIRE_SPI::setChannel(byte channel) {
  byte data[] = {channel};
  return sendCommand(RAPIDFIRE_CMD_SET_CHANNEL, data, sizeof(data));
}

RF_RESULT rapidFIRE_SPI::setBand(BAND band) {
  byte data[] = {band};
  return sendCommand(RAPIDFIRE_CMD_SET_BAND, data, sizeof(data));
}

RF_RESULT rapidFIRE_SPI::setRapidfireMode(RAPIDFIREMODE mode) {
  byte data[] = {(byte)mode};
  return sendCommand(RAPIDFIRE_CMD_SET_RAPIDFIRE_MODE, data, sizeof(data));
}
