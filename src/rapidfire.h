#pragma once

#include "module_base.h"
#include <Arduino.h>

#define VRX_BOOT_DELAY  2000

#define BIT_BANG_FREQ       1000
#define SPAM_COUNT          3

#define RF_API_DIR_GRTHAN   0x3E    // '>'
#define RF_API_DIR_EQUAL    0x3D    // '='
#define RF_API_BEEP_CMD     0x53    // 'S'
#define RF_API_CHANNEL_CMD  0x43    // 'C'
#define RF_API_BAND_CMD     0x42    // 'B'

class Rapidfire : public ModuleBase
{
public:
    void Init();
    void SendBuzzerCmd();
    void SendIndexCmd(uint8_t index);
    void SendChannelCmd(uint8_t channel);
    void SendBandCmd(uint8_t band);

private:
    void SendSPI(uint8_t* buf, uint8_t bufLen);
    void EnableSPIMode();
    uint8_t crc8(uint8_t* buf, uint8_t bufLen);
    bool SPIModeEnabled = false;
};
