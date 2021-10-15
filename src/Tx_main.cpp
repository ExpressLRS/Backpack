#include <Arduino.h>
#include "ESP8266_WebUpdate.h"
#include <espnow.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "msp.h"
#include "msptypes.h"
#include "logging.h"
// #include "config.h"

/////////// DEFINES ///////////

#define EEPROM_ADDR_WIFI    0x00
#define EEPROM_MAC          0x01 // 0x01 to 0x06

/////////// GLOBALS ///////////

#ifdef MY_UID
uint8_t broadcastAddress[6] = {MY_UID};
#else
uint8_t broadcastAddress[6] = {0, 0, 0, 0, 0, 0};
#endif
uint8_t bindingAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool cacheFull = false;
bool sendCached = false;
uint8_t flashLED = false;
bool startWebUpdater = false;
uint8_t flashLedCounter = 0;

/////////// CLASS OBJECTS ///////////

MSP msp;
// TxBackpackConfig config;
mspPacket_t cachedVTXPacket;

/////////// FUNCTION DEFS ///////////

void RebootIntoWifi();
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len);
void ProcessMSPPacketFromPeer(mspPacket_t *packet);
void ProcessMSPPacketFromTX(mspPacket_t *packet);
void SetSoftMACAddress();
void sendMSPViaEspnow(mspPacket_t *packet);

/////////////////////////////////////

void RebootIntoWifi()
{
  DBGLN("Rebooting into wifi update mode...");
  startWebUpdater = true;
  EEPROM.put(EEPROM_ADDR_WIFI, startWebUpdater);
  EEPROM.commit();
  ESP.restart();
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
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len)
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
  flashLedCounter++;
}

void ProcessMSPPacketFromTX(mspPacket_t *packet)
{
  if (packet->function == MSP_ELRS_BIND)
  {
    DBG("MSP_ELRS_BIND = ");
    for (int i = 0; i < 6; i++)
    {
      EEPROM.put(EEPROM_MAC + i, packet->payload[i]);
      DBG("%x", packet->payload[i]); // Debug prints
      DBG(",");
    }
    DBG(""); // Extra line for serial output readability
    EEPROM.commit();  
    // delay(500); // delay may not be required
    sendMSPViaEspnow(packet);
    // delay(500); // delay may not be required
    ESP.restart(); // restart to set SetSoftMACAddress
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

  flashLedCounter++;
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
    EEPROM.get(EEPROM_MAC + i, broadcastAddress[i]);
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

  EEPROM.begin(512);
  EEPROM.get(0, startWebUpdater);
  
  if (startWebUpdater)
  {
    EEPROM.put(EEPROM_ADDR_WIFI, false);
    EEPROM.commit();  
    BeginWebUpdate();
  }
  else
  {
    SetSoftMACAddress();

    if (esp_now_init() != 0)
    {
      DBGLN("Error initializing ESP-NOW");
      ESP.restart();
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    esp_now_register_recv_cb(OnDataRecv);
  }

  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);
  
  flashLedCounter = 2;
  DBGLN("Setup completed");
} 

void loop()
{
  uint8_t buttonPressed = !digitalRead(PIN_BUTTON);
  
  if (startWebUpdater)
  {
    HandleWebUpdate();
    flashLedCounter = 1;
    
    // button press to exit wifi
    if (buttonPressed)
      ESP.restart();
  }
  else
  {
    uint32_t now = millis();
    
    // press the boot button to start webupdater
    if (buttonPressed)
      RebootIntoWifi();
  
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

  // TODO make LED non blocking
  if (flashLedCounter)
  {
    flashLedCounter--;
    
    digitalWrite(PIN_LED, LOW);
    startWebUpdater == true ? delay(50) : delay(100);
    digitalWrite(PIN_LED, HIGH);
    startWebUpdater == true ? delay(50) : delay(100);
  }
}
