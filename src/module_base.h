#pragma once

#include <Arduino.h>
#include "msp.h"

#define VRX_BOOT_DELAY  0

class ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void SetOSD(mspPacket_t *packet);
    void Loop(uint32_t now);
};