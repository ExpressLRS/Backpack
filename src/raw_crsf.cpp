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

    // If the GPS data is <= 10 seconds old, keep spamming it out at 10hz
    bool gpsIsValid = (now < lastUpdated + 10000);

    if (now > lastSent + 100 && gpsIsValid)
    {
        lastSent = now;
    }
}
