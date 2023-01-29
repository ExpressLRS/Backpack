#pragma once

#include <Arduino.h>

class ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void Loop(uint32_t now);
};