#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "msp.h"
#include "msptypes.h"
#include "options.h"

namespace {

constexpr uint32_t SERIAL_BAUD = 460800;
constexpr uint8_t ESPNOW_CHANNEL = 1;

const char *packetTypeName(mspPacketType_e type)
{
  switch (type)
  {
    case MSP_PACKET_COMMAND:
      return "command";
    case MSP_PACKET_RESPONSE:
      return "response";
    default:
      return "unknown";
  }
}

const char *functionName(uint16_t function)
{
  switch (function)
  {
    case MSP_SET_VTX_CONFIG:
      return "MSP_SET_VTX_CONFIG";
    case MSP_ELRS_BIND:
      return "MSP_ELRS_BIND";
    case MSP_ELRS_REQU_VTX_PKT:
      return "MSP_ELRS_REQU_VTX_PKT";
    case MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE:
      return "MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE";
    case MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE:
      return "MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE";
    case MSP_ELRS_GET_BACKPACK_VERSION:
      return "MSP_ELRS_GET_BACKPACK_VERSION";
    case MSP_ELRS_BACKPACK_CRSF_TLM:
      return "MSP_ELRS_BACKPACK_CRSF_TLM";
    case MSP_ELRS_SET_OSD:
      return "MSP_ELRS_SET_OSD";
    case MSP_ELRS_BACKPACK_CONFIG:
      return "MSP_ELRS_BACKPACK_CONFIG";
    case MSP_ELRS_BACKPACK_SET_RECORDING_STATE:
      return "MSP_ELRS_BACKPACK_SET_RECORDING_STATE";
    case MSP_ELRS_BACKPACK_SET_OSD_ELEMENT:
      return "MSP_ELRS_BACKPACK_SET_OSD_ELEMENT";
    case MSP_ELRS_BACKPACK_SET_HEAD_TRACKING:
      return "MSP_ELRS_BACKPACK_SET_HEAD_TRACKING";
    case MSP_ELRS_BACKPACK_SET_RTC:
      return "MSP_ELRS_BACKPACK_SET_RTC";
    case MSP_ELRS_BACKPACK_SET_PTR:
      return "MSP_ELRS_BACKPACK_SET_PTR";
    default:
      return nullptr;
  }
}

void printMac(const uint8_t *mac)
{
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
      mac[0],
      mac[1],
      mac[2],
      mac[3],
      mac[4],
      mac[5]);
}

void printBytes(const uint8_t *data, size_t len)
{
  for (size_t i = 0; i < len; ++i)
  {
    if (i != 0)
    {
      Serial.write(' ');
    }
    Serial.printf("%02X", data[i]);
  }
}

void printPacket(const uint8_t *mac, const mspPacket_t *packet)
{
  Serial.print("MSP ");
  Serial.print(packetTypeName(packet->type));
  Serial.print(" from ");
  printMac(mac);
  Serial.print(": function=");

  const char *name = functionName(packet->function);
  if (name != nullptr)
  {
    Serial.print(name);
    Serial.print(" (");
  }

  Serial.printf("0x%04X", packet->function);

  if (name != nullptr)
  {
    Serial.print(')');
  }

  Serial.printf(", payload=%u", packet->payloadSize);

  if (packet->payloadSize != 0)
  {
    Serial.print(", bytes=");
    printBytes(packet->payload, packet->payloadSize);
  }

  Serial.println();
}

bool setSoftMacAddress()
{
  if (!firmwareOptions.hasUID)
  {
    Serial.println("No flashed UID found. Reflash this target with a UID first.");
    return false;
  }

  Serial.print("Flashed UID: ");
  printMac(firmwareOptions.uid);
  Serial.println();

  // MAC address can only be set with unicast, so first byte must be even, not odd.
  firmwareOptions.uid[0] &= ~0x01;

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
  WiFi.begin("network-name", "pass-to-network", ESPNOW_CHANNEL);
  delay(10);
  WiFi.disconnect();

  if (esp_wifi_set_mac(WIFI_IF_STA, firmwareOptions.uid) != ESP_OK)
  {
    Serial.println("Failed to set receiver soft MAC");
    return false;
  }

  Serial.print("Receiver STA SoftMAC: ");
  Serial.println(WiFi.macAddress());
  return true;
}

void setupEspNow()
{
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init failed");
    ESP.restart();
  }
}

void onDataRecv(const uint8_t *mac, const uint8_t *data, int len)
{
  MSP recvMsp;
  bool parsed = false;

  Serial.print("RAW from ");
  printMac(mac);
  Serial.printf(" (%d bytes): ", len);
  printBytes(data, static_cast<size_t>(len));
  Serial.println();

  for (int i = 0; i < len; ++i)
  {
    if (!recvMsp.processReceivedByte(data[i]))
    {
      continue;
    }

    parsed = true;
    printPacket(mac, recvMsp.getReceivedPacket());
    recvMsp.markPacketReceived();
  }

  if (!parsed)
  {
    Serial.println("No complete MSP packet decoded from frame.");
  }

  Serial.println();
}

} // namespace

void setup()
{
  Serial.begin(SERIAL_BAUD);
  delay(250);
  Serial.println();
  Serial.printf("ESP-NOW RX debug at %lu baud", SERIAL_BAUD);
  Serial.println();

  options_init();
  if (!setSoftMacAddress())
  {
    return;
  }

  setupEspNow();
  esp_now_register_recv_cb(onDataRecv);
  Serial.printf("Listening on channel %u", ESPNOW_CHANNEL);
  Serial.println();
}

void loop()
{
  delay(1000);
}
