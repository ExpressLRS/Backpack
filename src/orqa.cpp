#include "orqa.h"

GENERIC_CRC8 ghst_crc(GHST_CRC_POLY);

void Orqa::SendIndexCmd(uint8_t index)
{
    uint8_t band = GetBand(index);
    uint8_t channel = GetChannel(index);
    uint8_t currentGHSTChannel = GHSTChannel(band, channel);
    uint16_t currentFrequency = GetFrequency(index);
    SendGHSTUpdate(currentFrequency, currentGHSTChannel);
}


void Orqa::SendGHSTUpdate(uint16_t freq, uint8_t ghstChannel)
{
    uint8_t packet[] = {
        0x82, 0x0C, 0x20,                                       // Header
        0x00, (uint8_t)(freq & 0xFF), (uint8_t)(freq >> 8),     // Frequency
        0x01, 0x00, ghstChannel,                                // Band & Channel
        0x00, 0x00, 0x00, 0x00,                                 // Power Level?
        0x00 };                                                 // CRC
    uint8_t crc = ghst_crc.calc(&packet[2], 11);
    packet[13] = crc;

    for(uint8_t i = 0; i < 14; i++)
    {
        Serial.write(packet[i]);
    }
}

uint8_t Orqa::GHSTChannel(uint8_t band, uint8_t channel)
{
    // ELRS Bands:  A, B, E, I/F, R, L
    // Orqa/Rapidfire Bands: I/F, R, E, B, A, L, X
    uint8_t ghstChannel = 0x00;

    switch(band)
    {
    case 0x01:
        ghstChannel |= 0x50;
        break;
    case 0x02:
        ghstChannel |= 0x40;
        break;
    case 0x03:
        ghstChannel |= 0x30;
        break;
    case 0x04:
        ghstChannel |= 0x10;
        break;
    case 0x05:
        ghstChannel |= 0x20;
        break;
    case 0x06:
        ghstChannel |= 0x60;
        break;
    }
    ghstChannel |= channel;

    return ghstChannel;
}