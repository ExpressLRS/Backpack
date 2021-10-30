#include "fusion.h"
#include "logging.h"

void
Fusion::Init()
{
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
    buf[pos++] = crc8(buf, pos); // crc

    for (uint8_t i = 0; i < pos; ++i)
    {
        Serial.print("0x"); Serial.print(buf[i], HEX); Serial.print(", ");
    }
    Serial.println("");
}

// CRC function
// Input: byte array, array length
// Output: crc byte
uint8_t
Fusion::crc8(uint8_t* buf, uint8_t bufLen)
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < bufLen; ++i)
  {
    sum += buf[i];
  }
  return sum & 0xFF;
}
