#pragma once

#include "msp.h"
#include "msptypes.h"
#include <Arduino.h>

#define CHANNEL_INDEX_UNKNOWN   255

class HDZero
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    uint8_t GetChannelIndex();
    void SetChannelIndex(uint8_t index);

private:
    void SendMSPViaSerial(mspPacket_t *packet);
};
