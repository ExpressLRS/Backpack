#if defined(TARGET_MAVLINK_COMPANION)

#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "MAVLink.h"
#include "options.h"
#include "logging.h"

/////////// CONSTANTS ///////////

#define ESPNOW_MAX_PAYLOAD 250

/////////// GLOBALS ///////////

uint8_t txBackpackAddress[6];
esp_now_peer_info_t txPeerInfo;

// Uplink: UART -> MAVLink message queue -> ESP-NOW
mavlink_message_t uplinkMsgBuf[MAVLINK_BUF_SIZE];
uint8_t           uplinkMsgCount = 0;
unsigned long     lastUplinkFlush = 0;

// Downlink: ESP-NOW -> ring buffer -> MAVLink frame -> UART
uint8_t downlinkBuf[1024];
volatile uint16_t downlinkHead = 0;
volatile uint16_t downlinkTail = 0;

/////////// ESP-NOW CALLBACKS ///////////

// Downlink: ESP-NOW -> ring buffer (unsafe to call Serial.write from WiFi task on USB CDC)
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  if (memcmp(mac_addr, txBackpackAddress, 6) != 0)
  {
    return;
  }
  for (int i = 0; i < data_len; i++)
  {
    uint16_t nextHead = (downlinkHead + 1) % sizeof(downlinkBuf);
    if (nextHead == downlinkTail)
    {
      break; // buffer full, drop remaining bytes
    }
    downlinkBuf[downlinkHead] = data[i];
    downlinkHead = nextHead;
  }
}

/////////// MAC ADDRESS SETUP ///////////

void SetCompanionMACAddress()
{
  uint8_t mac[6];
  memcpy(mac, firmwareOptions.uid, 6);

  // First byte must be even for unicast
  mac[0] = mac[0] & ~0x01;
  // Differentiate from TX backpack by XOR on last byte
  mac[5] ^= 0x01;

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
  WiFi.begin("network-name", "pass-to-network", 1);
  WiFi.disconnect();

  esp_wifi_set_mac(WIFI_IF_STA, mac);

  // TX backpack address is the standard UID-derived MAC (first byte even, no XOR)
  memcpy(txBackpackAddress, firmwareOptions.uid, 6);
  txBackpackAddress[0] = txBackpackAddress[0] & ~0x01;
}

/////////// UPLINK FLUSH ///////////

void flushUplinkBuf()
{
  if (uplinkMsgCount == 0)
  {
    return;
  }

  uint8_t buf[ESPNOW_MAX_PAYLOAD];
  uint8_t bufLen = 0;

  uint8_t tmpBuf[MAVLINK_MAX_PACKET_LEN];
  for (uint8_t i = 0; i < uplinkMsgCount; i++)
  {
    uint16_t msgLen = mavlink_msg_to_send_buffer(tmpBuf, &uplinkMsgBuf[i]);

    // If adding this message would overflow, send current buffer first
    if (bufLen + msgLen > sizeof(buf) && bufLen > 0)
    {
      esp_now_send(txBackpackAddress, buf, bufLen);
      bufLen = 0;
    }

    // If a single message exceeds 250 bytes, fragment it
    if (msgLen > sizeof(buf))
    {
      uint16_t offset = 0;
      while (offset < msgLen)
      {
        uint16_t chunkLen = min((uint16_t)(sizeof(buf)), (uint16_t)(msgLen - offset));
        esp_now_send(txBackpackAddress, tmpBuf + offset, chunkLen);
        offset += chunkLen;
      }
    }
    else
    {
      memcpy(buf + bufLen, tmpBuf, msgLen);
      bufLen += msgLen;
    }
  }

  if (bufLen > 0)
  {
    esp_now_send(txBackpackAddress, buf, bufLen);
  }

  uplinkMsgCount = 0;
}

/////////// SETUP ///////////

void setup()
{
  Serial.setRxBufferSize(4096);
  Serial.begin(460800);

  options_init();

  SetCompanionMACAddress();

  if (esp_now_init() != ESP_OK)
  {
    DBGLN("Error initializing ESP-NOW");
    ESP.restart();
  }

  memcpy(txPeerInfo.peer_addr, txBackpackAddress, 6);
  txPeerInfo.channel = 0;
  txPeerInfo.encrypt = false;
  if (esp_now_add_peer(&txPeerInfo) != ESP_OK)
  {
    DBGLN("ESP-NOW failed to add TX backpack peer");
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv);

  DBGLN("MAVLink companion started");
}

/////////// LOOP ///////////

void loop()
{
  uint32_t now = millis();

  // Drain downlink ring buffer, frame MAVLink, write complete messages to Serial
  {
    mavlink_status_t dlStatus;
    mavlink_message_t dlMsg;
    uint8_t dlBuf[MAVLINK_MAX_PACKET_LEN];
    while (downlinkHead != downlinkTail)
    {
      uint8_t c = downlinkBuf[downlinkTail];
      downlinkTail = (downlinkTail + 1) % sizeof(downlinkBuf);

      if (mavlink_frame_char(MAVLINK_COMM_1, c, &dlMsg, &dlStatus) != MAVLINK_FRAMING_INCOMPLETE)
      {
        uint16_t len = mavlink_msg_to_send_buffer(dlBuf, &dlMsg);
        Serial.write(dlBuf, len);
      }
    }
  }

  // Parse MAVLink from UART, queue complete messages
  {
    mavlink_status_t ulStatus;
    mavlink_message_t ulMsg;
    while (Serial.available())
    {
      uint8_t c = Serial.read();

      if (mavlink_frame_char(MAVLINK_COMM_0, c, &ulMsg, &ulStatus) != MAVLINK_FRAMING_INCOMPLETE)
      {
        if (uplinkMsgCount < MAVLINK_BUF_SIZE)
        {
          uplinkMsgBuf[uplinkMsgCount++] = ulMsg;
        }
      }
    }
  }

  // Flush using same thresholds as Tx_main.cpp
  bool thresholdHit = uplinkMsgCount >= MAVLINK_BUF_THRESHOLD;
  bool timeoutHit   = uplinkMsgCount > 0 && (now - lastUplinkFlush) > MAVLINK_BUF_TIMEOUT;
  if (thresholdHit || timeoutHit)
  {
    flushUplinkBuf();
    lastUplinkFlush = now;
  }
}

#endif
