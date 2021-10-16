#include "rx5808.h"
#include <SPI.h>
#include "logging.h"

void
RX5808::Init()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    DBGLN("SPI config complete");
}

void
RX5808::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    uint16_t f = frequencyTable[index];
    
    uint16_t registerData = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);

    uint32_t data = SYNTHESIZER_REG_B  | (WRITE_CTRL_BIT << 4) | (registerData << 5);

    SendSPI(data);
}

void
RX5808::SendSPI(uint32_t buf)
{
    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    for (int i = 0; i < 25; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        buf >>= 1; 
    }
    DBGLN("");

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);
}
