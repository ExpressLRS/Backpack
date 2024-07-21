#pragma once

#include <Arduino.h>
#include "crsf_protocol.h"
#include "msp.h"

class ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void SetOSD(mspPacket_t *packet);
    void SendHeadTrackingEnableCmd(bool enable);
    void SetRTC();
    void SendLinkTelemetry(uint8_t *rawCrsfPacket);
    void SendBatteryTelemetry(uint8_t *rawCrsfPacket);
    void SendGpsTelemetry(crsf_packet_gps_t *packet) {}
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