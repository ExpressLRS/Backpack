#include "test_stand.h"
#include "channels.h"
#include "logging.h"
#include <Arduino.h>

void
TestStand::Init()
{
    ModuleBase::Init();

#if defined(PIN_OLED_SDA)
    displayInit();
#endif

    DBGLN("Test Stand initialized");
}

void
TestStand::SendIndexCmd(uint8_t index)
{
    // For test stand, we don't send commands to a VRX.
    // We repurpose this as a "channel index updated" callback.
    if (index >= 48)
    {
        return;
    }

    if (index != m_currentChannelIndex)
    {
        m_currentChannelIndex = index;
        displayChannel();
    }
}

void
TestStand::Loop(uint32_t now)
{
    (void)now;
}

#if defined(PIN_OLED_SDA)
void
TestStand::displayInit()
{
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        DBGLN("SSD1306 allocation failed");
        return;
    }

    _display.setTextWrap(false);
    _display.clearDisplay();
    _display.setTextSize(2);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    _display.println("Test Stand");
    _display.println("Backpack");
    _display.display();
    delay(800);

    displayChannel();
}

void
TestStand::displayChannel()
{
    _display.clearDisplay();
    _display.setCursor(0, 0);

    if (m_currentChannelIndex == 255)
    {
        // No channel selected: show a small waiting message so user knows the
        // test-stand is running but hasn't received channel data yet.
        _display.clearDisplay();
        _display.setTextSize(2);
        _display.setCursor(0, (SCREEN_HEIGHT - 16) / 2);
        _display.println("Waiting...");
        _display.display();
        return;
    }

    // Calculate band and channel for standard 48ch table: A, B, E, F, R, L
    constexpr char kBandLetters[] = {'A', 'B', 'E', 'F', 'R', 'L'};
    uint8_t bandIndex = m_currentChannelIndex / 8; // 0..5
    uint8_t channel = m_currentChannelIndex % 8 + 1;
    char bandLetter = (bandIndex < (sizeof(kBandLetters) / sizeof(kBandLetters[0])))
                          ? kBandLetters[bandIndex]
                          : '?';

    // Per request: show the same format on screen as in logs: "L1 (freq)".
    uint16_t freq = GetFrequency(m_currentChannelIndex);

    _display.setTextSize(2);
    _display.setCursor(0, (SCREEN_HEIGHT - 16) / 2);
    _display.printf("%c%u (%u)", bandLetter, (unsigned)channel, (unsigned)freq);
    _display.display();

    DBGLN("%c%u (%u)", bandLetter, (unsigned)channel, (unsigned)freq);
}
#endif
