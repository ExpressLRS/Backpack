#pragma once

#include <Arduino.h>

// Standard 48 channel VTx table size e.g. A, B, E, F, R, L
uint16_t FREQUENCIES[] = {
    // Band 1       2       3       4       5       6       7       8
    // A
            5865,   5845,   5825,   5805,   5785,   5765,   5745,   5725, 
    // B
            5733,   5752,   5771,   5790,   5809,   5828,   5847,   5866,
    // E
            5705,   5685,   5665,   5645,   5885,   5905,   5925,   5945,
    // F
            5740,   5760,   5780,   5800,   5820,   5840,   5860,   5880,
    // R
            5658,   5695,   5732,   5769,   5806,   5843,   5880,   5917,
    // L
            5333,   5373,   5413,   5453,   5493,   5533,   5573,   5613
};

uint16_t GetFrequency(uint8_t index);
uint8_t GetBand(uint8_t index);
uint8_t GetChannel(uint8_t index);