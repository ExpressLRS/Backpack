#include "raw_crsf.h"

CRSFBackPack::CRSFBackPack(HardwareSerial *port) :
    m_port(port),
    lastSent(0),
    lastUpdated(0)
{
}

void
CRSFBackPack::SendRawTelemetry(uint8_t *rawCrsfPacket, uint16_t size)
{
    lastUpdated = millis();
    m_port->write(rawCrsfPacket, size);
}

void
CRSFBackPack::Loop(uint32_t now)
{
    ModuleBase::Loop(now);
}
