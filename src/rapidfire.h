#pragma once

#include <Arduino.h>

#define BIT_BANG_FREQ       1000
#define SPAM_COUNT          3

#define RF_API_DIR_GRTHAN   0x3E    // '>'
#define RF_API_DIR_EQUAL    0x3D    // '='
#define RF_API_BEEP_CMD     0x53    // 'S'
#define RF_API_CHANNEL_CMD  0x43    // 'C'
#define RF_API_BAND_CMD     0x42    // 'B'

class Rapidfire
{
public:
    void Init();
    void SendBuzzerCmd();
    void SendIndexCmd(uint8_t index);
    void SendChannelCmd(uint8_t channel);
    void SendBandCmd(uint8_t band);

private:
    uint8_t cachedIndex = 0xFF;
    uint8_t cachedBand = 0xFF;
    uint8_t cachedChannel = 0xFF;

    void setCachedIndex(uint8_t idx){cachedIndex = idx;};
    void setCachedBand(uint8_t band){cachedBand = band;};
    void setCachedChannel(uint8_t channel){cachedChannel = channel;};
    uint8_t getCachedIndex(){return cachedIndex;}; 
    uint8_t getCachedBand(){return cachedBand;}; 
    uint8_t getCachedChannel(){return cachedChannel;}; 
    void SendSPI(uint8_t* buf, uint8_t bufLen);
    uint8_t crc8(uint8_t* buf, uint8_t bufLen);
};
