#include "rx5808.h"
#include <SPI.h>
#include "logging.h"

void
RX5808::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);

    #if defined(PIN_CS_2)
        pinMode(PIN_CS_2, INPUT);
    #endif

    DBGLN("RX5808 init complete");
}

void
RX5808::EnableSPIMode()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    #if defined(PIN_CS_2)
        pinMode(PIN_CS_2, OUTPUT);
        digitalWrite(PIN_CS_2, HIGH);
    #endif

    SPIModeEnabled = true;

    DBGLN("SPI config complete");
}

void
RX5808::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    uint16_t f = frequencyTable[index];
    
    uint32_t data = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);

    uint32_t newRegisterData = SYNTHESIZER_REG_B  | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);

    uint32_t currentRegisterData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | rtc6705readRegister(SYNTHESIZER_REG_B);

    if (newRegisterData != currentRegisterData)
    {
        rtc6705WriteRegister(newRegisterData);
    }
}

void
RX5808::rtc6705WriteRegister(uint32_t buf)
{
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    digitalWrite(PIN_CS, LOW);
    #if defined(PIN_CS_2)
        digitalWrite(PIN_CS_2, LOW);
    #endif
    delayMicroseconds(periodMicroSec);

    for (uint8_t i = 0; i < RX5808_PACKET_LENGTH; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        buf >>= 1; 
    }

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);
    #if defined(PIN_CS_2)
        digitalWrite(PIN_CS_2, HIGH);
    #endif
}

uint32_t
RX5808::rtc6705readRegister(uint8_t readRegister)
{
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    uint32_t buf = readRegister | (RX5808_READ_CTRL_BIT << 4);
    uint32_t registerData = 0;

    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    // Write register address and read bit
    for (uint8_t i = 0; i < RX5808_ADDRESS_R_W_LENGTH; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        buf >>= 1; 
    }

    // Change pin from output to input
    pinMode(PIN_MOSI, INPUT);

    // Read data 20 bits
    for (uint8_t i = 0; i < RX5808_DATA_LENGTH; i++)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);

        if (digitalRead(PIN_MOSI))
        {
            registerData = registerData | (1 << (5 + i));
        }

        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);
    }

    // Change pin back to output
    pinMode(PIN_MOSI, OUTPUT);

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    return registerData;
}
