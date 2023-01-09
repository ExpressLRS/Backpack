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
    void Loop(uint32_t now);
};

class MSPModuleBase : public ModuleBase
{
public:
    MSPModuleBase(Stream *port) : m_port(port) {};
    void Loop(uint32_t);

    void sendResponse(uint16_t function, const uint8_t *response, uint32_t responseSize);

    Stream *m_port;
    MSP msp;
};