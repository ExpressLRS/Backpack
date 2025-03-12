// rapidFIRE_SPI.h
// Copyright 2023 GOROman.

#pragma once
#include "rapidFIRE_SPI_Protocol.h"
#include <Arduino.h>
#include <SPI.h>

struct QUERY_RSSI {
  int16_t raw_rx1;
  int16_t raw_rx2;
  int16_t scaled_rx1;
  int16_t scaled_rx2;
};

struct QUERY_FIRMWARE_VERSION {
  byte oled[3]; // rapidFIRE OLED firmware version.
  byte core[3]; // rapidFIRE core firmware version.
};

struct QUERY_VOLTAGE {
  float voltage; // 4.222 V
};

typedef int RF_RESULT;

enum {
  RF_RESULT_OK = 0,
  RF_RESULT_ERROR = -1,
  RF_RESULT_ERROR_CHECKSUM_MISSMATCH = -2,
};

class SPIClass;

class rapidFIRE_SPI {
  int SPI_pin_SCK = -1;
  int SPI_pin_DATA = -1;
  int SPI_pin_SS = -1;
  int SPI_freq;

  SPIClass *spi = NULL;

public:
  enum BAND {
    BAND_F = 0x01, // - ImmersionRC / FatShark
    BAND_R = 0x02, // - RaceBand
    BAND_E = 0x03, // - Boscam E
    BAND_B = 0x04, // - Boscam B
    BAND_A = 0x05, // - Boscam A
    BAND_L = 0x06, // - LowRace
    BAND_X = 0x07, // - Band X
  };

  enum RXMODULE {
    BOTH = 0x00,  // - Both
    UPPER = 0x01, // - Upper
    LOWER = 0x02, // - Lower
  };

  enum OSDMODE {
    OFF = 0,             // - Off
    LOCKONLY = 1,        // - LockOnly
    DEFAULTMODE = 2,     // - Default
    LOCKANDSTANDARD = 3, // - LockAndStandard
    RSSIBARSLITE = 4,    // - RSSIBarsLite
    RSSIBARS = 5,        // - RSSIBars
    UNIQUIE_ID = 6,      // - Unique ID
    INTERNAL_USE = 7,    // - Internal use
    USERTEXT = 8,        // - UserText

  };
  enum RAPIDFIREMODE {
    RAPIDFIRE_1 = 0x00, // - RapidFIRE #1
    RAPIDFIRE_2 = 0x01, // - RapidFIRE #2
    LEGACY = 0x02,      // - Legacy
  };

private:
  RF_RESULT recvCommand(uint8_t *data, size_t len);
  RF_RESULT sendCommand(uint16_t command, byte *data = NULL, size_t len = 0);

public:
  rapidFIRE_SPI(int pin_SCK, int pin_DATA, int pin_SS, int freq = 60000);
  ~rapidFIRE_SPI();

  RF_RESULT begin();
  RF_RESULT end();

  // Query
  RF_RESULT getFirmwareVersion(QUERY_FIRMWARE_VERSION *version);
  RF_RESULT getRSSI(QUERY_RSSI *rssi);
  RF_RESULT getVoltage(QUERY_VOLTAGE *voltage);

  // Action
  RF_RESULT buzzer();

  // Command
  RF_RESULT setOSDUserText(char *text);
  RF_RESULT setOSDMode(OSDMODE mode);
  RF_RESULT setRXModule(RXMODULE module);
  RF_RESULT setChannel(byte channel);
  RF_RESULT setBand(BAND band);
  RF_RESULT setRapidfireMode(RAPIDFIREMODE mode);
};
