#pragma once

#include <Arduino.h>
#include <channels.h>

#define VRX_UART_BAUD   100000
#define VRX_BOOT_DELAY  5000



class Orqa 
{
public:
    Orqa(Stream *port);
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop();
private:
    Stream *m_port;
    uint16_t currentFrequency;
    uint8_t currentBand;
    uint8_t currentChannel;
    uint8_t ghstChannel;
    uint8_t GHSTChannel(uint8_t band, uint8_t channel);
};