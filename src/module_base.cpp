#include "module_base.h"
#include <max7456.h>
#include <SPI.h>

void
ModuleBase::Init()
{
    delay(VRX_BOOT_DELAY);

    Max7456 osd;

    byte tab[] = {0xC8, 0xC9};

    SPI.begin();

    osd.init(4);
    osd.setDisplayOffsets(50, 10);
    osd.setBlinkParams(_8fields, _BT_BT);

    osd.activateOSD();
    osd.clearScreen();
}

void
ModuleBase::SendIndexCmd(uint8_t index)
{
}

void
ModuleBase::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
}

void
ModuleBase::SetOSD(mspPacket_t *packet)
{
}

void
ModuleBase::ProcessRaceDetection(mspPacket_t *packet)
{
    uint8_t lapNumber = packet->readByte();
    uint8_t isLapEnd = packet->readByte();
    uint8_t isRaceEnd = packet->readByte();
    uint8_t timeLen = packet->readByte();

    char timeAsText[timeLen + 1];
    for (uint8_t i = 0; i < timeLen; ++i)
    {
        timeAsText[i] = packet->readByte();
    }
    timeAsText[timeLen] = '\0';

    Max7456 osd;

    byte tab[] = {0xC8, 0xC9};

    SPI.begin();

    osd.init(4);
    osd.setDisplayOffsets(50, 10);
    osd.setBlinkParams(_8fields, _BT_BT);

    osd.activateOSD();
    osd.clearScreen();

    char buff[MAX_OSD_STR_LEN+1];
    sprintf(buff, "L%d:%s", lapNumber, timeAsText);

    Serial.println(buff);
    osd.print(buff, 1, 15);
}

void
ModuleBase::ProcessRaceState(mspPacket_t *packet)
{
    uint8_t round = packet->readByte();
    uint8_t race = packet->readByte();
    uint8_t stateLen = packet->readByte();

    char state[stateLen + 1];
    for (uint8_t i = 0; i < stateLen; ++i)
    {
        state[i] = packet->readByte();
    }
    state[stateLen] = '\0';
    
    // Serial.println(state);

    Max7456 osd;

    byte tab[] = {0xC8, 0xC9};

    SPI.begin();

    osd.init(4);
    osd.setDisplayOffsets(50, 10);
    osd.setBlinkParams(_8fields, _BT_BT);

    osd.activateOSD();
    osd.clearScreen();

    osd.print(state, 1, 15);
}

void
ModuleBase::Loop(uint32_t now)
{
}