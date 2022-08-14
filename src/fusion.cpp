#include "fusion.h"
#include "logging.h"
#include "crc.h"
#include "crsf_protocol.h"
#include <Arduino.h>

GENERIC_CRC8 crsf_crc(CRSF_CRC_POLY);

void
Fusion::Init()
{
    ModuleBase::Init();
    DBGLN("Fusion backpack init complete");
}

void
Fusion::SendIndexCmd(uint8_t index)
{  
    uint16_t f = frequencyTable[index];
    uint8_t buf[12];
    uint8_t pos = 0;

    // 0xC8, 0xA, 0x40, 0x0, 0x0, 0x10, 0xEC, 0xE, 0x16, 0x94, 0x1, 0xBC, 0xC8, 0x04, 0x28, 0x00, 0x0E, 0x7C

    buf[pos++] = 0xC8;      // framing byte
    buf[pos++] = 0x0A;      // len
    buf[pos++] = 0x40;      // some form of device / origin address ?
    buf[pos++] = 0x00;      // some form of device / origin address ?
    buf[pos++] = 0x00;      // some form of device / origin address ?
    buf[pos++] = 0x10;
    buf[pos++] = 0xEC;
    buf[pos++] = 0x0E;
    buf[pos++] = f >> 8;    // freq byte 1
    buf[pos++] = f & 0xFF;  // freq byte 2
    buf[pos++] = 0x01;

    uint8_t crc = crsf_crc.calc(&buf[2], pos - 2); // first 2 bytes not included in CRC
    buf[pos++] = crc;

    for (uint8_t i = 0; i < pos; ++i)
    {
        Serial.write(buf[i]);
    }

    // Leaving this in as its useful to debug
    // for (uint8_t i = 0; i < pos; ++i)
    // {
    //     Serial.print("0x"); Serial.print(buf[i], HEX); Serial.print(", ");
    // }
    // Serial.println("");
}
