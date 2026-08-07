#pragma once

#include "module_base.h"
#include "msp.h"
#include "msptypes.h"
#include "rapidFIRE_SPI.h"
#include "rapidFIRE_SPI_Protocol.h"

#include "logging.h"
#include <Arduino.h>
#include <queue>
#include <string>

#define VRX_BOOT_DELAY 2000       // 2 seconds delay before sending any packets
#define DELAY_BETWEEN_SPI_PKT 100 // 100ms delay between each SPI packet
#define SPAM_COUNT 3 // rapidfire sometimes missed a pkt, so send 3x

#define MAX_QUEUE_SIZE 10     // max number of packets to queue for display
#define DISPLAY_INTERVAL 1000 // 1 seconds
#define OSD_TIMEOUT 2500      // 2.5 seconds

class Rapidfire : public ModuleBase {
public:
  void Init();
  void Loop(uint32_t now);
  void SendIndexCmd(uint8_t index);
  void SendChannelCmd(uint8_t channel);
  void SendBandCmd(uint8_t band);
  void SendHeadTrackingEnableCmd(bool enable);
  void SetRecordingState(uint8_t recordingState, uint16_t delay);
  void SetOSD(mspPacket_t *packet);
  void SendLinkTelemetry(uint8_t *rawCrsfPacket);
  void SendBatteryTelemetry(uint8_t *rawCrsfPacket);
  void SendGpsTelemetry(crsf_packet_gps_t *packet);

private:
  uint32_t m_lastDisplayTime = 0;
  char m_textBuffer[RAPIDFIRE_MAX_LENGTH_TEXT];
  std::queue<std::string> m_displayQueue;

  void ShowNextMessage();
  void ClearOSD();
  bool BufferIsClear(char *payload, int len);
  int CountMessageParts(std::vector<char> &buffer, int len);
  void QueueMessageParts(std::vector<char> &buffer, int len, int numParts);
};
