#include "tbs_fusion.h"
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

void
Fusion::SendLinkTelemetry(uint8_t *rawCrsfPacket)
{
    // uplink_RSSI_1
    uint8_t rssi = rawCrsfPacket[3] * -1;
    // uplink_Link_quality
    uint8_t lq = rawCrsfPacket[5] / 3;
    // SNR
    uint8_t snr = rawCrsfPacket[6]; // actually int8_t
    // Construct our outbound buffer to the Fusion STM32
    uint8_t buf[] = {
        0xC8, // Flight controller
        0x0F, // Overall length of extended packet
        0x40, // Extended frametype
        0x00,
        0x00,
        0x14, // CRSF_FRAMETYPE_LINK_STATISTICS
        0x1D, // ?
        rssi,
        lq,
        snr,
        0xFD, // ?
        0x02,
        0x02,
        0x1B,
        0x64,
        0x47,
        0x00 // CRC
    };
    // Calculate & write CRC
    buf[sizeof(buf) - 1] = crsf_crc.calc(&buf[2], sizeof(buf) - 3);
    for (uint8_t i = 0; i < sizeof(buf); i++)
    {
        Serial.write(buf[i]);
    }
}

void
Fusion::SendBatteryTelemetry(uint8_t *rawCrsfPacket)
{
    uint16_t voltage = ((uint16_t)rawCrsfPacket[3] << 8 | rawCrsfPacket[4]);
    uint16_t amperage = ((uint16_t)rawCrsfPacket[5] << 8 | rawCrsfPacket[6]);
    uint32_t mah = ((uint32_t)rawCrsfPacket[7] << 16 | (uint32_t)rawCrsfPacket[8] << 8 | rawCrsfPacket[9]);
    uint8_t percentage = rawCrsfPacket[10];
    // Construct our outbound buffer to the Fusion STM32
    uint8_t buf[] = {
        0xC8, // Flight controller
        0x0D, // Overall length of extended packet
        0x40, // Extended frametype
        0x00,
        0x00,
        0x08, // CRSF_FRAMETYPE_BATTERY_SENSOR
        (uint8_t)(voltage >> 8),
        (uint8_t)(voltage & 0xFF),
        (uint8_t)(amperage >> 8),
        (uint8_t)(amperage & 0xFF),
        (uint8_t)(mah >> 16),
        (uint8_t)(mah >> 8),
        (uint8_t)(mah & 0xFF),
        percentage,
        0x00 // CRC
    };
    // Calculate & write CRC
    buf[sizeof(buf) - 1] = crsf_crc.calc(&buf[2], sizeof(buf) - 3);
    for (uint8_t i = 0; i < sizeof(buf); i++)
    {
        Serial.write(buf[i]);
    }
}