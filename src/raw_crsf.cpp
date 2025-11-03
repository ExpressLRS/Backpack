#include "raw_crsf.h"

CrsfRawBackpack::CrsfRawBackpack(HardwareSerial *port) :
    m_port(port),
    lastSent(0),
    lastUpdated(0)
{
}

void
CrsfRawBackpack::SendRawTelemetry(uint8_t *rawCrsfPacket, uint16_t size)
{
    lastUpdated = millis();
    m_port->write(rawCrsfPacket, size);
}

void
CrsfRawBackpack::Loop(uint32_t now)
{
    ModuleBase::Loop(now);
}
