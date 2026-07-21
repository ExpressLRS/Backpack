#pragma once
#include <Arduino.h>

#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
#endif

#include "config.h"
#include "common.h"

class RcMain
{
public:
    static RcMain& Instance();

    void Init();
    void SendCrsf(const uint8_t* data, uint8_t len);
    void SetPeerAddress(const uint8_t addr[6]);

private:
    bool IsTelemEnabled();
    uint8_t _peerAddr[6];
};