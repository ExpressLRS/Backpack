#pragma once

#include "module_base.h"
#include <Arduino.h>

#define VRX_BOOT_DELAY  1000
#define VRX_UART_BAUD   500000   // fusion uses 500k baud between the ESP8266 and the STM32

const uint16_t frequencyTable[48] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613  // L
};

class Fusion : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
};
