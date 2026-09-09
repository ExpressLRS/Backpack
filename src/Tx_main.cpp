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
#include "ESPNOW_Helpers.h"
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

const uint8_t version[] = {LATEST_VERSION};

connectionState_e connectionState = starting;
// Assume we are in wifi update mode until we know otherwise
wifi_service_t wifiService = WIFI_SERVICE_UPDATE;
unsigned long rebootTime = 0;

bool cacheFull = false;
bool sendCached = false;
bool espnowStarted = false;

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

void sendMSPViaWiFiUDP(mspPacket_t *packet);

/////////////////////////////////////

#if defined(PLATFORM_ESP32)
// This seems to need to be global, as per this page,
// otherwise we get errors about invalid peer:
// https://rntlab.com/question/espnow-peer-interface-is-invalid/
esp_now_peer_info_t peerInfo;
esp_now_peer_info_t bindingInfo;
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
  MSP recv_msp;
  DBGLN("ESP NOW DATA:");
  for(int i = 0; i < data_len; i++)
  {
    if (recv_msp.processReceivedByte(data[i]))
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
        ProcessMSPPacketFromPeer(recv_msp.getReceivedPacket());
      }
      recv_msp.markPacketReceived();
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
  switch (packet->function)
  {
  case MSP_SET_VTX_CONFIG:
    DBGLN("Processing MSP_SET_VTX_CONFIG...");
    cachedVTXPacket = *packet;
    cacheFull = true;
    // transparently forward MSP packets via espnow to any subscribers
    ESPNOW::sendMSPViaEspnow(packet);
    break;

  case MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE:
    DBGLN("Processing MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE...");
    ESPNOW::sendMSPViaEspnow(packet);
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
    ESPNOW::sendMSPViaEspnow(packet);
    break;

  case MSP_ELRS_BACKPACK_CRSF_TLM:
    DBGLN("Processing MSP_ELRS_BACKPACK_CRSF_TLM...");
    if (config.GetTelemMode() == BACKPACK_TELEM_MODE_WIFI)
    {
      sendMSPViaWiFiUDP(packet);
    }
    if (config.GetTelemMode() != BACKPACK_TELEM_MODE_OFF)
    {
      ESPNOW::sendMSPViaEspnow(packet);
    }
    break;

  case MSP_ELRS_BACKPACK_CONFIG:
    DBGLN("Processing MSP_ELRS_BACKPACK_CONFIG...");
    HandleConfigMsg(packet);
    break;

  case MSP_ELRS_BIND:
    DBG("MSP_ELRS_BIND = ");
    for (int i = 0; i < 6; i++)
    {
      DBG("%x", packet->payload[i]); // Debug prints
      DBG(",");
    }
    DBG(""); // Extra line for serial output readability

    // If the BIND address is different to our current one,
    // then we save it and reboot so it can take effect
    if (memcmp(packet->payload, config.GetGroupAddress(), 6) != 0)
    {
      config.SetGroupAddress(packet->payload);
      config.Commit();
      rebootTime = millis(); // restart to set SetSoftMACAddress
      return;
    }
    ESPNOW::sendMSPViaEspnow(packet);
    break;

  default:
    // transparently forward MSP packets via espnow to any subscribers
    ESPNOW::sendMSPViaEspnow(packet);
    break;
  }
}

void sendMSPViaWiFiUDP(mspPacket_t *packet)
{
  uint8_t packetSize = msp.getTotalPacketSize(packet);
  uint8_t dataOutput[packetSize];

  uint8_t result = msp.convertToByteArray(packet, dataOutput);
  if (!result)
  {
    return;
  }

  SendTxBackpackTelemetryViaUDP(dataOutput, packetSize);
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
    ESPNOW::sendMSPViaEspnow(&cachedVTXPacket);
  }
  if (cachedHTPacket.type != MSP_PACKET_UNKNOWN)
  {
    ESPNOW::sendMSPViaEspnow(&cachedHTPacket);
  }
}

// Resolve the UID we peer with over ESP-NOW.  Does not touch the radio, so it is
// safe to call while WiFi is already up.
void ResolveUID()
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
}

void SetSoftMACAddress()
{
  ResolveUID();

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

// Bring up ESP-NOW on whatever interface WiFi is currently using.  Safe to call
// repeatedly: it only does work the first time, or after WiFi has moved to a
// different interface/channel and the peers need re-binding.
//
// Caveat for WiFi mode: ESP-NOW has to share the radio's single channel with WiFi.
// In AP mode we own the channel, so peers running plain ESP-NOW (channel 1) hear us.
// In STA mode we are pinned to the home router's channel, so unless that router is
// also on channel 1 an ESP-NOW peer such as an antenna tracker will not receive
// anything.  Fixing that properly needs the peers to follow us, which is out of
// scope here -- AP mode is the usual MAVLink-over-WiFi setup.
bool StartEspNow()
{
  #if defined(PLATFORM_ESP8266)
    // Peers must sit on the channel the radio is actually on.  In WiFi mode that is
    // whatever the AP/station settled on, not the ESP-NOW default of 1.
    uint8_t channel = wifi_get_channel();
    if (channel == 0)
    {
      channel = 1;
    }
    const uint8_t boundTo = channel;
  #elif defined(PLATFORM_ESP32)
    // When WiFi is running as a soft-AP the station interface is down, and sending on
    // it fails with ESP_ERR_ESPNOW_IF.  Bind the peers to the interface that is up.
    const wifi_interface_t ifidx = ((int)WiFi.getMode() & (int)WIFI_MODE_AP) ? WIFI_IF_AP : WIFI_IF_STA;
    const uint8_t boundTo = (uint8_t)ifidx;
  #else
    const uint8_t boundTo = 0;
  #endif

  static uint8_t espnowBoundTo = 0;
  if (espnowStarted)
  {
    if (espnowBoundTo == boundTo)
    {
      return true;
    }
    // WiFi moved (e.g. a station that could not associate fell back to AP mode).
    // Tear ESP-NOW down so the peers get re-added against the interface that is up.
    DBGLN("WiFi interface changed, restarting ESP-NOW");
    esp_now_deinit();
    espnowStarted = false;
  }

  if (esp_now_init() != 0)
  {
    DBGLN("Error initializing ESP-NOW");
    return false;
  }

  #if defined(PLATFORM_ESP8266)
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_add_peer(firmwareOptions.uid, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
    esp_now_add_peer((uint8_t *)bindingAddress, ESP_NOW_ROLE_COMBO, channel, NULL, 0);
  #elif defined(PLATFORM_ESP32)
    memcpy(peerInfo.peer_addr, firmwareOptions.uid, 6);
    peerInfo.channel = 0; // 0 == use the current channel
    peerInfo.encrypt = false;
    peerInfo.ifidx = ifidx;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      DBGLN("ESP-NOW failed to add peer");
      return false;
    }
    memcpy(bindingInfo.peer_addr, bindingAddress, 6);
    bindingInfo.channel = 0;
    bindingInfo.encrypt = false;
    bindingInfo.ifidx = ifidx;
    if (esp_now_add_peer(&bindingInfo) != ESP_OK)
    {
      DBGLN("ESP-NOW failed to add binding peer");
      return false;
    }
  #endif

  esp_now_register_recv_cb(OnDataRecv);
  espnowStarted = true;
  espnowBoundTo = boundTo;
  DBGLN("ESP-NOW started");
  return true;
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
  LOGGING_UART.begin(115200);
  LOGGING_UART.setDebugOutput(true);
#endif
  Serial.setRxBufferSize(4096);
  Serial.begin(460800);

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
    // ESP-NOW cannot be started here: devWIFI takes the radio down and back up as it
    // brings the AP/station online, which would tear it straight back down.  It is
    // started from devWIFI once WiFi has settled.  Resolve the UID now so the peer
    // address (and the MAVLink AP SSID, which is derived from it) is valid.
    ResolveUID();
    connectionState = wifiUpdate;
    devicesTriggerEvent();
  }
  else
  {
    SetSoftMACAddress();

    if (!StartEspNow())
    {
      rebootTime = millis();
    }
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

  while (Serial.available())
  {
    const uint8_t c = Serial.read();

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
