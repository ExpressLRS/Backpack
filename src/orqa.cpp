#include "orqa.h"

Orqa::Orqa(Stream *port)
{
    m_port = port;
}

void Orqa::Init()
{

}

void Orqa::SendIndexCmd(uint8_t index)
{
    currentFrequency = GetFrequency(index);
    currentBand = GetBand(index);
    currentChannel = GetChannel(index);
    ghstChannel = GHSTChannel(currentBand, currentChannel);
}

void Orqa::Loop()
{

}

uint8_t Orqa::GHSTChannel(uint8_t band, uint8_t channel)
{
    // ELRS Bands:  A, B, E, I/F, R, L
    // Orqa/Rapidfire Bands: I/F, R, E, B, A, L, X
    uint8_t ghstChannel;
    switch(band)
    {
    case 0x01:
        ghstChannel &= 0x50;
        break;
    case 0x02:
        ghstChannel &= 0x40;
        break;
    case 0x03:
        ghstChannel &= 0x30;
        break;
    case 0x04:
        ghstChannel &= 0x10;
        break;
    case 0x05:
        ghstChannel &= 0x20;
        break;
    case 0x06:
        ghstChannel &= 0x60;
        break;
    }
    ghstChannel &= channel;

    return ghstChannel;
}