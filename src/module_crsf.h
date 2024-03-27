#pragma once

#include "module_base.h"
#include "crsf_protocol.h"
#include "crc.h"

class CrsfModuleBase : public ModuleBase
{
public:
    CrsfModuleBase() = delete;
    CrsfModuleBase(Stream &port) :
        _port(port), _crc(CRSF_CRC_POLY),
        _rxBufPos(0)
        {};
    void Loop(uint32_t now);

    virtual void onCrsfPacketIn(const crsf_header_t *pkt) {};

protected:
    Stream &_port;

    static constexpr uint8_t CRSF_MAX_PACKET_SIZE = 64U;
    static constexpr uint8_t CRSF_MAX_PAYLOAD_LEN = (CRSF_MAX_PACKET_SIZE - 4U);

    GENERIC_CRC8 _crc;
    uint8_t _rxBufPos;
    uint8_t _rxBuf[CRSF_MAX_PACKET_SIZE];

    void processPacketIn(uint8_t len);
    void shiftRxBuffer(uint8_t cnt);
    void handleByteReceived();
};