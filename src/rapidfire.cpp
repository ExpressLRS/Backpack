#include "rapidfire.h"
#include <SPI.h>
#include "logging.h"

void
Rapidfire::Init()
{
    delay(2000);  // wait for rapidfire to boot

    // Set pins to output to configure rapidfire in SPI mode
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);

    // put the RF into SPI mode by pulling all 3 pins high,
    // then low within 100-400ms
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CS, HIGH);
    delay(200);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, LOW);
    delay(200);
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);

    DBGLN("SPI config complete");
}

void
Rapidfire::SendBuzzerCmd()
{
    DBGLN("Beep!");
    
    uint8_t cmd[4];
    cmd[0] = RF_API_BEEP_CMD;   // 'S'
    cmd[1] = RF_API_DIR_GRTHAN; // '>'
    cmd[2] = 0x00;              // len
    cmd[3] = crc8(cmd, 3);

    // rapidfire sometimes missed a pkt, so send 3x
    SendSPI(cmd, 4);
    SendSPI(cmd, 4);
    SendSPI(cmd, 4);
}

void
Rapidfire::SendChannelCmd(uint8_t channel)
{
    // ELRS channel is zero based, need to add 1
    channel++;

    DBG("Setting channel ");
    DBGLN("%x", channel);

    uint8_t cmd[5];
    cmd[0] = RF_API_CHANNEL_CMD;    // 'C'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = channel;               // temporarily set byte 4 to channel for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = channel;               // assign channel to correct byte 5

    
    SendSPI(cmd, 5);
}

void
Rapidfire::SendBandCmd(uint8_t band)
{
    DBG("Setting band ");
    DBGLN("%x", band);

    // ELRS bands
    // 0x01 - Boscam A
    // 0x02 - Boscam B
    // 0x03 - Boscam E
    // 0x04 - ImmersionRC/FatShark
    // 0x05 - RaceBand
    // 0x06 - LowRace

    // rapidfire bands
    // 0x01 - ImmersionRC/FatShark
    // 0x02 - RaceBand
    // 0x03 - Boscam E
    // 0x04 - Boscam B
    // 0x05 - Boscam A
    // 0x06 - LowRace
    // 0x07 - Band X

    // convert ELRS band index to IMRC band index:
    uint8_t imrcBand;
    switch (band)
    {
    case 0x01:
        imrcBand = 0x05;
        break;
    case 0x02:
        imrcBand = 0x04;
        break;
    case 0x03:
        imrcBand = 0x03;
        break;
    case 0x04:
        imrcBand = 0x01;
        break;
    case 0x05:
        imrcBand = 0x02;
        break;
    case 0x06:
        imrcBand = 0x06;
        break;
    default:
        imrcBand = 0x01;
        break;
    }

    uint8_t cmd[5];
    cmd[0] = RF_API_BAND_CMD;       // 'C'
    cmd[1] = RF_API_DIR_EQUAL;      // '='
    cmd[2] = 0x01;                  // len
    cmd[3] = imrcBand;              // temporarily set byte 4 to band for crc calc
    cmd[3] = crc8(cmd, 4);          // reset byte 4 to crc
    cmd[4] = imrcBand;              // assign band to correct byte 5

    SendSPI(cmd, 5);
}

void
Rapidfire::SendSPI(uint8_t* buf, uint8_t bufLen)
{
    uint32_t periodMicroSec = 1000000 / BIT_BANG_FREQ;

    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    delayMicroseconds(periodMicroSec);

    digitalWrite(PIN_CS, LOW);
    delay(100);

    // debug code for printing SPI pkt
    for (int i = 0; i < bufLen; ++i)
    {
        uint8_t bufByte = buf[i];

        DBG("%x", bufByte);
        DBG(",");

        for (uint8_t k = 0; k < 8; k++)
        {
            // digitalWrite takes about 0.5us, so it is not taken into account with delays.
            digitalWrite(PIN_CLK, LOW);
            delayMicroseconds(periodMicroSec / 4);
            digitalWrite(PIN_MOSI, bufByte & 0x80);
            delayMicroseconds(periodMicroSec / 4);
            digitalWrite(PIN_CLK, HIGH);
            delayMicroseconds(periodMicroSec / 2);

            bufByte <<= 1; 
        }
    }
    DBGLN("");

    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);
    delay(100);
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);
}

// CRC function for IMRC rapidfire API
// Input: byte array, array length
// Output: crc byte
uint8_t
Rapidfire::crc8(uint8_t* buf, uint8_t bufLen)
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < bufLen; ++i)
  {
    sum += buf[i];
  }
  return sum & 0xFF;
}
