#pragma once

#include "module_base.h"
#include <Arduino.h>

#define BIT_BANG_FREQ                               10000

#define SYNTHESIZER_REG_A                           0x00
#define SYNTHESIZER_REG_B                           0x01
#define SYNTHESIZER_REG_C                           0x02
#define SYNTHESIZER_REG_D                           0x03
#define VCO_SWITCH_CAP_CONTROL_REGISTER             0x04
#define DFC_CONTROL_REGISTER                        0x05
#define SIXM_AUDIO_DEMODULATOR_CONTROL_REGISTER     0x06
#define SIXM5_AUDIO_DEMODULATOR_CONTROL_REGISTER    0x07
#define RECEIVER_CONTROL_REGISTER_1                 0x08
#define RECEIVER_CONTROL_REGISTER_2                 0x09
#define POWER_DOWN_CONTROL_REGISTER                 0x0A
#define STATE_REGISTER                              0x0F

#define RX5808_READ_CTRL_BIT                        0x00
#define RX5808_WRITE_CTRL_BIT                       0x01
#define RX5808_ADDRESS_R_W_LENGTH                   5
#define RX5808_DATA_LENGTH                          20
#define RX5808_PACKET_LENGTH                        25

const uint16_t frequencyTable[48] = {
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // A
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // B
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // E
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // F
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917, // R
    5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613  // L
};

typedef enum {
    ModeMix = 0,
    ModeDiversity
} videoMode_t;

class SteadyView : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetMode(videoMode_t mode);

private:
    void rtc6705WriteRegister(uint32_t buf);
    uint32_t rtc6705readRegister(uint8_t readRegister);
    uint8_t currentIndex;
};
