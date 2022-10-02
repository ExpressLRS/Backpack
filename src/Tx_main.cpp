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
#include "helpers.h"

#include "device.h"
#include "devWIFI.h"
#include "devButton.h"
#include "devLED.h"

/////////// GLOBALS ///////////

#ifdef MY_UID
uint8_t broadcastAddress[6] = {MY_UID};
#else
uint8_t broadcastAddress[6] = {0, 0, 0, 0, 0, 0};
#endif
uint8_t bindingAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

connectionState_e connectionState = starting;
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

/////////// FUNCTION DEFS ///////////

void sendMSPViaEspnow(mspPacket_t *packet);

/////////////////////////////////////

#if defined(PLATFORM_ESP32)
// This seems to need to be global, as per this page,
// otherwise we get errors about invalid peer:
// https://rntlab.com/question/espnow-peer-interface-is-invalid/
esp_now_peer_info_t peerInfo;
#endif

void RebootIntoWifi()
{
  DBGLN("Rebooting into wifi update mode...");
  config.SetStartWiFiOnBoot(true);
  config.Commit();
  rebootTime = millis();
}

void ProcessMSPPacketFromPeer(mspPacket_t *packet)
{
  if (packet->function == MSP_ELRS_REQU_VTX_PKT)
  {
    DBGLN("MSP_ELRS_REQU_VTX_PKT...");
    // request from the vrx-backpack to send cached VTX packet
    if (cacheFull)
    {
      sendCached = true;
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
      if (broadcastAddress[0] == mac_addr[0] &&
          broadcastAddress[1] == mac_addr[1] &&
          broadcastAddress[2] == mac_addr[2] &&
          broadcastAddress[3] == mac_addr[3] &&
          broadcastAddress[4] == mac_addr[4] &&
          broadcastAddress[5] == mac_addr[5])
      {
        ProcessMSPPacketFromPeer(msp.getReceivedPacket());
      }
      msp.markPacketReceived();
    }
  }
  blinkLED();
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
    esp_now_send(broadcastAddress, (uint8_t *) &nowDataOutput, packetSize);
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

  sendMSPViaEspnow(&cachedVTXPacket);
}

void SetSoftMACAddress()
{
  DBGLN("EEPROM MAC = ");
  for (int i = 0; i < 6; i++)
  {
    #ifndef MY_UID
    memcpy(broadcastAddress, config.GetGroupAddress(), 6);
    #endif
    DBG("%x", broadcastAddress[i]); // Debug prints
    DBG(",");
  }
  DBGLN(""); // Extra line for serial output readability

  // MAC address can only be set with unicast, so first byte must be even, not odd
  broadcastAddress[0] = broadcastAddress[0] & ~0x01;

  WiFi.mode(WIFI_STA);
  WiFi.begin("network-name", "pass-to-network", 1);
  WiFi.disconnect();

  // Soft-set the MAC address to the passphrase UID for binding
  #if defined(PLATFORM_ESP8266)
    wifi_set_macaddr(STATION_IF, broadcastAddress);
  #elif defined(PLATFORM_ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, broadcastAddress);
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
#ifdef AXIS_THOR_TX_BACKPACK
  Serial.begin(115200);
#else
  Serial.begin(115200);
#endif

  eeprom.Begin();
  config.SetStorageProvider(&eeprom);
  config.Load();

  devicesInit(ui_devices, ARRAY_SIZE(ui_devices));

  #ifdef DEBUG_ELRS_WIFI
    config.SetStartWiFiOnBoot(true);
  #endif

  if (config.GetStartWiFiOnBoot())
  {
    config.SetStartWiFiOnBoot(false);
    config.Commit();
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
      esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    #elif defined(PLATFORM_ESP32)
      memcpy(peerInfo.peer_addr, broadcastAddress, 6);
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

  if (connectionState == wifiUpdate)
  {
    return;
  }

  if (Serial.available())
  {
    uint8_t c = Serial.read();

    if (msp.processReceivedByte(c))
    {
      // Finished processing a complete packet
      ProcessMSPPacketFromTX(msp.getReceivedPacket());
      msp.markPacketReceived();
    }
  }

  if (cacheFull && sendCached)
  {
    SendCachedMSP();
    sendCached = false;
  }
}
