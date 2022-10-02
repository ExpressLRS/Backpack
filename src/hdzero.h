#pragma once

#include "msp.h"
#include "msptypes.h"
#include "module_base.h"
#include <Arduino.h>

#define VRX_BOOT_DELAY              0
#define VRX_RESPONSE_TIMEOUT        500
#define VRX_UART_BAUD               115200  // hdzero uses 115k baud between the ESP8285 and the STM32

#define CHANNEL_INDEX_UNKNOWN       255
#define VRX_DVR_RECORDING_ACTIVE    1
#define VRX_DVR_RECORDING_INACTIVE  0
#define VRX_DVR_RECORDING_UNKNOWN   255

class HDZero : public ModuleBase
{
public:
    HDZero(Stream *port);
    void Init();
    void SendIndexCmd(uint8_t index);
    uint8_t GetChannelIndex();
    void SetChannelIndex(uint8_t index);
    uint8_t GetRecordingState();
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    void SetOSD(mspPacket_t *packet);

private:
    Stream *m_port;
};
