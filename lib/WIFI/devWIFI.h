#pragma once

#include "device.h"

#if defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
extern device_t WIFI_device;
#define HAS_WIFI

extern const char *VERSION;

#if defined(TARGET_TX_BACKPACK)
bool SendTxBackpackTelemetryViaUDP(const uint8_t *data, uint16_t size);
#endif
#endif
