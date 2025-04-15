#pragma once

#include "module_base.h"
#include "msptypes.h"

#define VRX_UART_BAUD           115200

class CRSFBackPack : public ModuleBase
{
public:
CRSFBackPack(HardwareSerial *port);
    void SendRawTelemetry(uint8_t *rawCrsfPacket, uint16_t size);
    void Loop(uint32_t now);

private:
    void SendHeartbeat();
    void SendGpsRawInt();
    void SendGlobalPositionInt();

    HardwareSerial *m_port;
    uint32_t lastSent;
    uint32_t lastUpdated;
};
