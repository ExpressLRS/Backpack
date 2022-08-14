#pragma once

#include <Arduino.h>

#define VRX_BOOT_DELAY  0

class ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void ModuleLoop();
};