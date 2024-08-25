#include <Arduino.h>

#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
  #include <esp_wifi.h>
  #include <WiFi.h>
#endif

#include "msp.h"
#include "msptypes.h"
#include "logging.h"
#include "config.h"
#include "common.h"
#include "options.h"
#include "helpers.h"

#include "device.h"
#include "devWIFI.h"
#include "devButton.h"
#include "devLED.h"

#if defined(MAVLINK_ENABLED)
#include <MAVLink.h>
#endif

/////////// GLOBALS ///////////

uint8_t bindingAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const uint8_t version[] = {LATEST_VERSION};

connectionState_e connectionState = starting;
// Assume we are in wifi update mode until we know otherwise
wifi_service_t wifiService = WIFI_SERVICE_UPDATE;
unsigned long rebootTime = 0;

bool cacheFull = false;
bool sendCached = false;

device_t *ui_devices[] = {
#ifdef PIN_LED
  &LED_device,
#endif
#ifdef PIN_BUTTON
  &Button_device,
#endif
  &WIFI_device,
};

/////////// CLASS OBJECTS ///////////

MSP msp;
ELRS_EEPROM eeprom;
TxBackpackConfig config;
mspPacket_t cachedVTXPacket;
mspPacket_t cachedHTPacket;
#if defined(MAVLINK_ENABLED)
MAVLink mavlink;
#endif

/////////// FUNCTION DEFS ///////////

void sendMSPViaEspnow(mspPacket_t *packet);

/////////////////////////////////////

#if defined(PLATFORM_ESP32)
// This seems to need to be global, as per this page,
// otherwise we get errors about invalid peer:
// https://rntlab.com/question/espnow-peer-interface-is-invalid/
esp_now_peer_info_t peerInfo;
#endif

void RebootIntoWifi(wifi_service_t service = WIFI_SERVICE_UPDATE)
{
  DBGLN("Rebooting into wifi update mode...");
  config.SetStartWiFiOnBoot(true);
#if defined(TARGET_TX_BACKPACK)
  // TODO it might be better to add wifi service to each type of backpack
  config.SetWiFiService(service);
#endif
  config.Commit();
  rebootTime = millis();
}

void ProcessMSPPacketFromPeer(mspPacket_t *packet)
{
  switch (packet->function) {
    case MSP_ELRS_REQU_VTX_PKT: {
      DBGLN("MSP_ELRS_REQU_VTX_PKT...");
      // request from the vrx-backpack to send cached VTX packet
      if (cacheFull)
      {
        sendCached = true;
      }
      break;
    }
    case MSP_ELRS_BACKPACK_SET_PTR: {
      DBGLN("MSP_ELRS_BACKPACK_SET_PTR...");
      msp.sendPacket(packet, &Serial);
      break;
    }
    case MSP_SET_VTX_CONFIG: {
      DBGLN("MSP_SET_VTX_CONFIG...");
      msp.sendPacket(packet, &Serial);
      break;
    }
  }
}

// espnow on-receive callback
#if defined(PLATFORM_ESP8266)
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len)
#elif defined(PLATFORM_ESP32)
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *data, int data_len)
#endif
{
  DBGLN("ESP NOW DATA:");
  for(int i = 0; i < data_len; i++)
  {
    if (msp.processReceivedByte(data[i]))
    {
      // Finished processing a complete packet
      // Only process packets from a bound MAC address
      if (firmwareOptions.uid[0] == mac_addr[0] &&
          firmwareOptions.uid[1] == mac_addr[1] &&
          firmwareOptions.uid[2] == mac_addr[2] &&
          firmwareOptions.uid[3] == mac_addr[3] &&
          firmwareOptions.uid[4] == mac_addr[4] &&
          firmwareOptions.uid[5] == mac_addr[5])
      {
        ProcessMSPPacketFromPeer(msp.getReceivedPacket());
      }
      msp.markPacketReceived();
    }
  }
  blinkLED();
}

void SendVersionResponse()
{
  mspPacket_t out;
  out.reset();
  out.makeResponse();
  out.function = MSP_ELRS_GET_BACKPACK_VERSION;
  for (size_t i = 0 ; i < sizeof(version) ; i++)
  {
    out.addByte(version[i]);
  }
  msp.sendPacket(&out, &Serial);
}

void HandleConfigMsg(mspPacket_t *packet)
{
  uint8_t key = packet->readByte();
  uint8_t value = packet->readByte();
  switch (key)
  {
    case MSP_ELRS_BACKPACK_CONFIG_TLM_MODE:
      switch (value)
      {
        case BACKPACK_TELEM_MODE_OFF:
          config.SetTelemMode(BACKPACK_TELEM_MODE_OFF);
          config.SetWiFiService(WIFI_SERVICE_UPDATE);
          config.SetStartWiFiOnBoot(false);
          config.Commit();
          break;
        case BACKPACK_TELEM_MODE_ESPNOW:
          config.SetTelemMode(BACKPACK_TELEM_MODE_ESPNOW);
          config.SetWiFiService(WIFI_SERVICE_UPDATE);
          config.SetStartWiFiOnBoot(false);
          config.Commit();
          break;
        case BACKPACK_TELEM_MODE_WIFI:
          config.SetTelemMode(BACKPACK_TELEM_MODE_WIFI);
          config.SetWiFiService(WIFI_SERVICE_MAVLINK_TX);
          config.SetStartWiFiOnBoot(true);
          config.Commit();
          break;
      }
      rebootTime = millis();
      break;
  }
}

void ProcessMSPPacketFromTX(mspPacket_t *packet)
{
  if (packet->function == MSP_ELRS_BIND)
  {
    config.SetGroupAddress(packet->payload);
    DBG("MSP_ELRS_BIND = ");
    for (int i = 0; i < 6; i++)
    {
      DBG("%x", packet->payload[i]); // Debug prints
      DBG(",");
    }
    DBG(""); // Extra line for serial output readability
    config.Commit();
    // delay(500); // delay may not be required
    sendMSPViaEspnow(packet);
    // delay(500); // delay may not be required
    rebootTime = millis(); // restart to set SetSoftMACAddress
  }

  switch (packet->function)
  {
  case MSP_SET_VTX_CONFIG:
    DBGLN("Processing MSP_SET_VTX_CONFIG...");
    cachedVTXPacket = *packet;
    cacheFull = true;
    // transparently forward MSP packets via espnow to any subscribers
    sendMSPViaEspnow(packet);
    break;
  case MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE:
    DBGLN("Processing MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE...");
    sendMSPViaEspnow(packet);
    break;
  case MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE:
    DBGLN("Processing MSP_ELRS_SET_TX_BACKPACK_WIFI_MODE...");
    RebootIntoWifi();
    break;
  case MSP_ELRS_GET_BACKPACK_VERSION:
    DBGLN("Processing MSP_ELRS_GET_BACKPACK_VERSION...");
    SendVersionResponse();
    break;
  case MSP_ELRS_BACKPACK_SET_HEAD_TRACKING:
    DBGLN("Processing MSP_ELRS_BACKPACK_SET_HEAD_TRACKING...");
    cachedHTPacket = *packet;
    cacheFull = true;
    sendMSPViaEspnow(packet);
    break;
  case MSP_ELRS_BACKPACK_CRSF_TLM:
    DBGLN("Processing MSP_ELRS_BACKPACK_CRSF_TLM...");
    if (config.GetTelemMode() != BACKPACK_TELEM_MODE_OFF)
    {
      sendMSPViaEspnow(packet);
    }
    break;
  case MSP_ELRS_BACKPACK_CONFIG:
    DBGLN("Processing MSP_ELRS_BACKPACK_CONFIG...");
    HandleConfigMsg(packet);
    break;
  default:
    // transparently forward MSP packets via espnow to any subscribers
    sendMSPViaEspnow(packet);
    break;
  }
}

void sendMSPViaEspnow(mspPacket_t *packet)
{
  uint8_t packetSize = msp.getTotalPacketSize(packet);
  uint8_t nowDataOutput[packetSize];

  uint8_t result = msp.convertToByteArray(packet, nowDataOutput);

  if (!result)
  {
    // packet could not be converted to array, bail out
    return;
  }

  if (packet->function == MSP_ELRS_BIND)
  {
    esp_now_send(bindingAddress, (uint8_t *) &nowDataOutput, packetSize); // Send Bind packet with the broadcast address
  }
  else
  {
    esp_now_send(firmwareOptions.uid, (uint8_t *) &nowDataOutput, packetSize);
  }

  blinkLED();
}

void SendCachedMSP()
{
  if (!cacheFull)
  {
    // nothing to send
    return;
  }

  if (cachedVTXPacket.type != MSP_PACKET_UNKNOWN)
  {
    sendMSPViaEspnow(&cachedVTXPacket);
  }
  if (cachedHTPacket.type != MSP_PACKET_UNKNOWN)
  {
    sendMSPViaEspnow(&cachedHTPacket);
  }
}

void SetSoftMACAddress()
{
  if (!firmwareOptions.hasUID)
  {
    memcpy(firmwareOptions.uid, config.GetGroupAddress(), 6);
  }
  DBG("EEPROM MAC = ");
  for (int i = 0; i < 6; i++)
  {
    DBG("%x", firmwareOptions.uid[i]); // Debug prints
    DBG(",");
  }
  DBGLN(""); // Extra line for serial output readability

  // MAC address can only be set with unicast, so first byte must be even, not odd
  firmwareOptions.uid[0] = firmwareOptions.uid[0] & ~0x01;

  WiFi.mode(WIFI_STA);
  #if defined(PLATFORM_ESP8266)
    WiFi.setOutputPower(20.5);
  #elif defined(PLATFORM_ESP32)
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
  #endif
  WiFi.begin("network-name", "pass-to-network", 1);
  WiFi.disconnect();

  // Soft-set the MAC address to the passphrase UID for binding
  #if defined(PLATFORM_ESP8266)
    wifi_set_macaddr(STATION_IF, firmwareOptions.uid);
  #elif defined(PLATFORM_ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, firmwareOptions.uid);
  #endif
}

#if defined(PLATFORM_ESP8266)
// Called from core's user_rf_pre_init() function (which is called by SDK) before setup()
RF_PRE_INIT()
{
    // Set whether the chip will do RF calibration or not when power up.
    // I believe the Arduino core fakes this (byte 114 of phy_init_data.bin)
    // to be 1, but the TX power calibration can pull over 300mA which can
    // lock up receivers built with a underspeced LDO (such as the EP2 "SDG")
    // Option 2 is just VDD33 measurement
    #if defined(RF_CAL_MODE)
    system_phy_set_powerup_option(RF_CAL_MODE);
    #else
    system_phy_set_powerup_option(2);
    #endif
}
#endif

void setup()
{
  #ifdef DEBUG_LOG
    Serial1.begin(115200);
    Serial1.setDebugOutput(true);
  #endif
  Serial.begin(460800);
  Serial.setRxBufferSize(4096);

  options_init();

  eeprom.Begin();
  config.SetStorageProvider(&eeprom);
  config.Load();

  devicesInit(ui_devices, ARRAY_SIZE(ui_devices));

  #ifdef DEBUG_ELRS_WIFI
    config.SetStartWiFiOnBoot(true);
  #endif

  
  if (config.GetStartWiFiOnBoot())
  {
    wifiService = config.GetWiFiService();
    if (wifiService == WIFI_SERVICE_UPDATE)
    {
      config.SetStartWiFiOnBoot(false);
      config.Commit();
    }
    connectionState = wifiUpdate;
    devicesTriggerEvent();
  }
  else
  {
    SetSoftMACAddress();

    if (esp_now_init() != 0)
    {
      DBGLN("Error initializing ESP-NOW");
      rebootTime = millis();
    }

    #if defined(PLATFORM_ESP8266)
      esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      esp_now_add_peer(firmwareOptions.uid, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    #elif defined(PLATFORM_ESP32)
      memcpy(peerInfo.peer_addr, firmwareOptions.uid, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      if (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        DBGLN("ESP-NOW failed to add peer");
        return;
      }
    #endif

    esp_now_register_recv_cb(OnDataRecv);
  }

  devicesStart();
  if (connectionState == starting)
  {
    connectionState = running;
  }
  DBGLN("Setup completed");
}

void loop()
{
  uint32_t now = millis();

  devicesUpdate(now);

  #if defined(PLATFORM_ESP8266) || defined(PLATFORM_ESP32)
    // If the reboot time is set and the current time is past the reboot time then reboot.
    if (rebootTime != 0 && now > rebootTime) {
      ESP.restart();
    }
  #endif

  if (Serial.available())
  {
    uint8_t c = Serial.read();

    // Try to parse MSP packets from the TX
    if (msp.processReceivedByte(c))
    {
      // Finished processing a complete packet
      ProcessMSPPacketFromTX(msp.getReceivedPacket());
      msp.markPacketReceived();
    }

  #if defined(MAVLINK_ENABLED)
    // Try to parse MAVLink packets from the TX
    mavlink.ProcessMAVLinkFromTX(c);
  #endif
  }

  if (cacheFull && sendCached)
  {
    SendCachedMSP();
    sendCached = false;
  }
}
