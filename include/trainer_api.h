#pragma once

#include <stdint.h>

#if defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)

bool TrainerIsAvailable();
bool TrainerIsPaired();
bool TrainerIsPairing();
const uint8_t *TrainerGetPeerMac();
bool TrainerStartPairing();
void TrainerForgetPeer();

#endif
