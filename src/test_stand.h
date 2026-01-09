#pragma once

#include "msp.h"
#include "msptypes.h"
#include "module_base.h"
#include <Arduino.h>

#if defined(PIN_OLED_SDA)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#endif

class TestStand : public MSPModuleBase
{
public:
    TestStand(Stream *port) : MSPModuleBase(port) {};
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);

private:
    void displayInit();
    void displayChannel();

#if defined(PIN_OLED_SDA)
    Adafruit_SSD1306 _display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#endif

    uint8_t m_currentChannelIndex = 255;
};