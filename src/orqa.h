#pragma once

#include <channels.h>
#include <Arduino.h>
#include <crc.h>

#define VRX_UART_BAUD   100000
#define VRX_BOOT_DELAY  5000
#define GHST_CRC_POLY   0xD5


class Orqa 
{
public:
    Orqa();
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);
private:
    uint32_t lastGHSTPacketTime;
    uint16_t currentFrequency;
    uint8_t currentGHSTChannel;
    uint8_t GHSTChannel(uint8_t band, uint8_t channel);
    void SendGHSTUpdate(uint16_t freq, uint8_t ghstChannel);
};