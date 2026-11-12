#pragma once

#if defined(PLATFORM_ESP32) && defined(BLE_TELEM_ENABLED)

#include <Arduino.h>
#include "device.h"

extern device_t BLE_device;

bool SendTxBackpackTelemetryViaBLE(const uint8_t *data, uint16_t size);

#endif
