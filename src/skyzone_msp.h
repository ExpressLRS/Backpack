#pragma once

#include "msp.h"
#include "msptypes.h"
#include "module_base.h"
#include <Arduino.h>

#define VRX_BOOT_DELAY              2000
#define VRX_RESPONSE_TIMEOUT        500
#define VRX_UART_BAUD               115200  // skyzone uses 115k baud between the ESP32-PICO and their MCU

#define CHANNEL_INDEX_UNKNOWN       255
#define VRX_DVR_RECORDING_ACTIVE    1
#define VRX_DVR_RECORDING_INACTIVE  0
#define VRX_DVR_RECORDING_UNKNOWN   255

class SkyzoneMSP : public ModuleBase
{
public:
    SkyzoneMSP(Stream *port);
    void Init();
    void SendIndexCmd(uint8_t index);
    uint8_t GetChannelIndex();
    void SetChannelIndex(uint8_t index);
    uint8_t GetRecordingState();
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void SetOSD(mspPacket_t *packet);
    void Loop(uint32_t now);

private:
    void SendRecordingState();

    Stream      *m_port;
    uint8_t     m_recordingState;
    uint16_t    m_delay;
    uint32_t    m_delayStartMillis;
};
