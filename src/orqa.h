#pragma once

#include "module_base.h"
#include <channels.h>
#include <crc.h>

#define VRX_UART_BAUD   100000
#define VRX_BOOT_DELAY  7000
#define GHST_CRC_POLY   0xD5

class Orqa : public ModuleBase
{
public:
    void SendIndexCmd(uint8_t index);
private:
    uint8_t GHSTChannel(uint8_t band, uint8_t channel);
    void SendGHSTUpdate(uint16_t freq, uint8_t ghstChannel);
};