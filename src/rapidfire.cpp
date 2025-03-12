#include "rapidfire.h"
#include "logging.h"

static rapidFIRE_SPI rapidfire = rapidFIRE_SPI(PIN_CLK, PIN_MOSI, PIN_CS);

void Rapidfire::Init() {
  ModuleBase::Init();

  delay(VRX_BOOT_DELAY);

  rapidfire.begin();

  DBGLN("Rapid Fire init");
}

void Rapidfire::SetOSD(mspPacket_t *packet) {
  int len = packet->payloadSize;
  if (len < 4) {
    return;
  }

  for (int i = 0; i < 4; i++) {   //payload = [0x03, row, col, 0]
    packet->readByte(); // skip the first 4 bytes
  }
  len -= 4; // subtract the first 4 bytes

  if (len > MAX_LENGTH_TEXT) {
    len = MAX_LENGTH_TEXT;
  }

  for (int i = 0; i < len; i++) {
    m_textBuffer[i] = packet->readByte();
  }

  DisplayTextBuffer();
}

void Rapidfire::DisplayTextBuffer() {
  if (m_displayStartMillis == 0) {
    for (int i = 0; i < SPAM_COUNT; i++) {
      rapidfire.setOSDMode(rapidFIRE_SPI::OSDMODE::USERTEXT);
      delay(DELAY_BETWEEN_SPI_PKT);
    }
  }

  // only send the text once to the rapidfire to avoid failure
  rapidfire.setOSDUserText(m_textBuffer); // send the text to the rapidfire

  memset(m_textBuffer, 0, MAX_LENGTH_TEXT);

  m_displayStartMillis = millis();
}

void Rapidfire::SetRecordingState(uint8_t recordingState, uint16_t _delay){
  sprintf(m_textBuffer, "Is Recording %5d", recordingState);

  DisplayTextBuffer();
}

void Rapidfire::SendHeadTrackingEnableCmd(bool enable) {
  sprintf(m_textBuffer, "Head Tracking %5d", enable);

  DisplayTextBuffer();
}

void Rapidfire::SendIndexCmd(uint8_t index) {
  uint8_t newBand = index / 8 + 1;
  uint8_t newChannel = index % 8;

  rapidFIRE_SPI::BAND imrcBand; // convert ELRS band index to IMRC band index:
  switch (newBand) {
    case 0x01: // Boscam A
      imrcBand = rapidFIRE_SPI::BAND::BAND_A;
      break;
    case 0x02: // Boscam B
      imrcBand = rapidFIRE_SPI::BAND::BAND_B;
      break;
    case 0x03: // Boscam E
      imrcBand = rapidFIRE_SPI::BAND::BAND_E;
      break;
    case 0x04: // ImmersionRC/FatShark
      imrcBand = rapidFIRE_SPI::BAND::BAND_F;
      break;
    case 0x05: // RaceBand
      imrcBand = rapidFIRE_SPI::BAND::BAND_R;
      break;
    case 0x06: // LowRace
      imrcBand = rapidFIRE_SPI::BAND::BAND_L;
      break;
    default: // ImmersionRC/FatShark
      imrcBand = rapidFIRE_SPI::BAND::BAND_F;
      break;
  }

  newChannel++;      // ELRS channel is zero based, need to add 1

  for (int i = 0; i < SPAM_COUNT; i++) {
    rapidfire.setBand(imrcBand);
    delay(DELAY_BETWEEN_SPI_PKT);
    rapidfire.setChannel(newChannel);
    delay(DELAY_BETWEEN_SPI_PKT);
  }
}

void
Rapidfire::Loop(uint32_t now)
{
    ModuleBase::Loop(now);

    if (m_displayStartMillis != 0)    {
        if (now - m_displayStartMillis >= TIMEOUT_SET_OSD)
        {
            for (int i = 0; i < SPAM_COUNT; i++) {
              rapidfire.setOSDMode(rapidFIRE_SPI::OSDMODE::DEFAULTMODE);
              delay(DELAY_BETWEEN_SPI_PKT);
            }
            m_displayStartMillis = 0;
        }
    }
}
