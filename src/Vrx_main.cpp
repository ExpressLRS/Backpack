#include <Arduino.h>
#include <espnow.h>

#include <ESP8266WiFi.h>
#include "msp.h"
#include "msptypes.h"
#include "logging.h"
#include "helpers.h"
#include "common.h"
#include "config.h"

#include "device.h"
#include "devWIFI.h"
#include "devButton.h"
#include "devLED.h"

#ifdef RAPIDFIRE_BACKPACK
  #include "rapidfire.h"
#elif defined(RX5808_BACKPACK)
  #include "rx5808.h"
#elif defined(STEADYVIEW_BACKPACK)
  #include "steadyview.h"
#endif

/////////// DEFINES ///////////

#define BINDING_TIMEOUT     30000
#define NO_BINDING_TIMEOUT  120000
#define BINDING_LED_PAUSE   1000

/////////// GLOBALS ///////////

#ifdef MY_UID
uint8_t broadcastAddress[6] = {MY_UID};
#else
uint8_t broadcastAddress[6] = {0, 0, 0, 0, 0, 0};
#endif

connectionState_e connectionState = starting;
unsigned long rebootTime = 0;

uint8_t cachedIndex = 0;
bool sendChangesToVrx = false;
bool gotInitialPacket = false;
uint32_t lastSentRequest = 0;

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
VrxBackpackConfig config;

#ifdef RAPIDFIRE_BACKPACK
  Rapidfire vrxModule;
#elif defined(RX5808_BACKPACK)
  RX5808 vrxModule;
#elif defined(STEADYVIEW_BACKPACK)
  SteadyView vrxModule;
#endif

/////////// FUNCTION DEFS ///////////

void ProcessMSPPacket(mspPacket_t *packet);
void sendMSPViaEspnow(mspPacket_t *packet);
void resetBootCounter();

/////////////////////////////////////

void RebootIntoWifi()
{
  DBGLN("Rebooting into wifi update mode...");
  config.SetStartWiFiOnBoot(true);
  config.SetBootCount(0);
  config.Commit();
  rebootTime = millis();
}

// espnow on-receive callback
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len)
{
  DBGLN("ESP NOW DATA:");
  for(int i = 0; i < data_len; i++)
  {
    DBG("%x", data[i]); // Debug prints
    DBG(",");

    if (msp.processReceivedByte(data[i]))
    {
      DBGLN(""); // Extra line for serial output readability
      // Finished processing a complete packet
      // Only process packets from a bound MAC address
      if (connectionState == binding ||
            (
            broadcastAddress[0] == mac_addr[0] &&
            broadcastAddress[1] == mac_addr[1] &&
            broadcastAddress[2] == mac_addr[2] &&
            broadcastAddress[3] == mac_addr[3] &&
            broadcastAddress[4] == mac_addr[4] &&
            broadcastAddress[5] == mac_addr[5]
            )
          )
      {
        gotInitialPacket = true;
        ProcessMSPPacket(msp.getReceivedPacket());
      }
      else
      {
        DBGLN("Failed MAC add check and not in bindingMode.");
      }
      msp.markPacketReceived();
    }
  }
  blinkLED();
}

void ProcessMSPPacket(mspPacket_t *packet)
{
  if (connectionState == binding)
  {
    DBGLN("Processing Binding Packet...");
    if (packet->function == MSP_ELRS_BIND)
    {
      config.SetGroupAddress(packet->payload);
      DBGLN("MSP_ELRS_BIND MAC = ");
      for (int i = 0; i < 6; i++)
      {
        DBG("%x", packet->payload[i]); // Debug prints
        DBG(",");
      }
      DBG(""); // Extra line for serial output readability
      resetBootCounter();
      rebootTime = millis();
    }
    return;
  }

  switch (packet->function)
  {
  case MSP_SET_VTX_CONFIG:
    DBGLN("Processing MSP_SET_VTX_CONFIG...");
    if (packet->payload[0] < 48) // Standard 48 channel VTx table size e.g. A, B, E, F, R, L
    {
      // cache changes here, to be handled outside this callback, in the main loop
      cachedIndex = packet->payload[0];;
      sendChangesToVrx = true;
    }
    else
    {
      return; // Packets containing frequency in MHz are not yet supported.
    }
    break;
  case MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE:
    DBGLN("Processing MSP_ELRS_SET_VRX_BACKPACK_WIFI_MODE...");
    RebootIntoWifi();
    break;
  default:
    break;
  }
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

  // Soft-set the MAC address to the passphrase UID for binding
  wifi_set_macaddr(STATION_IF, broadcastAddress);
}

void RequestVTXPacket()
{
  mspPacket_t packet;
  packet.reset();
  packet.makeCommand();
  packet.function = MSP_ELRS_REQU_VTX_PKT;
  packet.addByte(0);  // empty byte

  blinkLED();
  sendMSPViaEspnow(&packet);
}

void sendMSPViaEspnow(mspPacket_t *packet)
{
  // Do not send while in binding mode.  The currently used broadcastAddress may be garbage.
  if (connectionState == binding)
    return;

  uint8_t packetSize = msp.getTotalPacketSize(packet);
  uint8_t nowDataOutput[packetSize];

  uint8_t result = msp.convertToByteArray(packet, nowDataOutput);

  if (!result)
  {
    // packet could not be converted to array, bail out
    return;
  }

  esp_now_send(broadcastAddress, (uint8_t *) &nowDataOutput, packetSize);
}

void resetBootCounter()
{
  config.SetBootCount(0);
  config.Commit();
}

void checkIfInBindingMode()
{
  uint8_t bootCounter = config.GetBootCount();
  bootCounter++;

  if (bootCounter > 2)
  {
    resetBootCounter();

    #ifdef MY_UID
    RebootIntoWifi();
    #else
    connectionState = binding;
    #endif
  }
  else
  {
    config.SetBootCount(bootCounter);
    config.Commit();
  }

  DBGLN(""); // blank line to clear below prints from serial boot garbage
  DBG("bootCounter = ");
  DBGLN("%x", bootCounter);
  DBG("bindingMode = ");
  DBGLN("%x", connectionState == binding);
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
  Serial.begin(460800);

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
    checkIfInBindingMode();

    SetSoftMACAddress();

    if (esp_now_init() != 0)
    {
      DBGLN("Error initializing ESP-NOW");
      turnOffLED();
      ESP.restart();
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    esp_now_register_recv_cb(OnDataRecv);

    vrxModule.Init();
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
      turnOffLED();
      ESP.restart();
    }
  #endif

  if (connectionState == wifiUpdate)
  {
    return;
  }

  if (connectionState == binding && now > NO_BINDING_TIMEOUT)
  {
    RebootIntoWifi();
  }

  if (sendChangesToVrx)
  {
    sendChangesToVrx = false;
    vrxModule.SendIndexCmd(cachedIndex);
  }

  // spam out a bunch of requests for the desired band/channel for the first 5s
  if (!gotInitialPacket && now < 5000 && now - lastSentRequest > 1000 && connectionState != binding)
  {
    DBGLN("RequestVTXPacket...");
    RequestVTXPacket();
    lastSentRequest = now;
  }

  // Power cycle must be done within 30s.  Long timeout to allow goggles to boot and shutdown correctly e.g. Orqa.
  if (now > BINDING_TIMEOUT && config.GetBootCount() > 0)
  {
    DBGLN("resetBootCounter...");
    resetBootCounter();
  }
}