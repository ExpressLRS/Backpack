#include "steadyview.h"
#include <SPI.h>
#include "logging.h"

void
SteadyView::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);

    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_CS, HIGH);

    DBGLN("SPI config complete");
    delay(100);
    SetMode(ModeMix);
}

void
SteadyView::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    uint16_t f = frequencyTable[index];
    uint32_t data = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);
    uint32_t newRegisterData = SYNTHESIZER_REG_B  | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);

    uint32_t currentRegisterData = SYNTHESIZER_REG_B | (RX5808_WRITE_CTRL_BIT << 4) | rtc6705readRegister(SYNTHESIZER_REG_B);

    if (newRegisterData != currentRegisterData)
    {
        rtc6705WriteRegister(SYNTHESIZER_REG_A  | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
        rtc6705WriteRegister(newRegisterData);
    }
    currentIndex = index;
}

void
SteadyView::SetMode(videoMode_t mode)
{
    if (mode == ModeMix)
    {
        digitalWrite(PIN_CLK, HIGH);
        delay(100);
        digitalWrite(PIN_CLK, LOW);
        delay(500);
    }
    uint16_t f = frequencyTable[currentIndex];
    uint32_t data = ((((f - 479) / 2) / 32) << 7) | (((f - 479) / 2) % 32);
    uint32_t registerData = SYNTHESIZER_REG_B  | (RX5808_WRITE_CTRL_BIT << 4) | (data << 5);

    rtc6705WriteRegister(SYNTHESIZER_REG_A  | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
    delayMicroseconds(500);
    rtc6705WriteRegister(SYNTHESIZER_REG_A  | (RX5808_WRITE_CTRL_BIT << 4) | (0x8 << 5));
    rtc6705WriteRegister(registerData);
}

void
SteadyView::rtc6705WriteRegister(uint32_t buf)
{
    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    for (uint8_t i = 0; i < RX5808_PACKET_LENGTH; ++i)
    {
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);

        buf >>= 1;
    }

    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_CS, HIGH);
}

uint32_t
SteadyView::rtc6705readRegister(uint8_t readRegister)
{
    uint32_t buf = readRegister | (RX5808_READ_CTRL_BIT << 4);
    uint32_t registerData = 0;

    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    // Write register address and read bit
    for (uint8_t i = 0; i < RX5808_ADDRESS_R_W_LENGTH; ++i)
    {
        digitalWrite(PIN_MOSI, buf & 0x01);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);

        buf >>= 1;
    }

    // Change pin from output to input
    pinMode(PIN_MOSI, INPUT);

    // Read data 20 bits
    for (uint8_t i = 0; i < RX5808_DATA_LENGTH; i++)
    {
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 4);

        if (digitalRead(PIN_MOSI))
        {
            registerData = registerData | (1 << (5 + i));
        }

        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 2);
    }

    // Change pin back to output
    pinMode(PIN_MOSI, OUTPUT);

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CS, HIGH);

    return registerData;
}
