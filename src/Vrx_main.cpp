#include <Arduino.h>
#include "ESP8266_WebUpdate.h"
#include <espnow.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "msp.h"
#include "msptypes.h"
#include "logging.h"
// #include "config.h"

#ifdef RAPIDFIRE_BACKPACK
  #include "rapidfire.h"
#endif

/////////// DEFINES ///////////

#define WIFI_PIN            0
#define LED_PIN             16

#define EEPROM_ADDR_WIFI    0x00
#define EEPROM_MAC          0x01 // 0x01 to 0x06
#define EEPROM_RST_COUNTER  0x07

#define BINDING_TIMEOUT     30000
#define BINDING_LED_PAUSE   1000

/////////// GLOBALS ///////////

#ifdef MY_UID
uint8_t broadcastAddress[6] = {MY_UID};
#else
uint8_t broadcastAddress[6] = {0, 0, 0, 0, 0, 0};
#endif

bool startWebUpdater = false;
bool bindingMode = false;
uint8_t bootCounter = 0;
uint8_t flashLedCounter = 0;
uint32_t nextBindingLedFlash = 0;

uint8_t cachedBand = 0;
uint8_t cachedChannel = 0;
bool sendChangesToVrx = false;
bool gotInitialPacket = false;
uint32_t lastSentRequest = 0;

/////////// CLASS OBJECTS ///////////

MSP msp;
// VrxBackpackConfig config;

#ifdef RAPIDFIRE_BACKPACK
  Rapidfire vrxModule;
#elif GENERIC_BACKPACK
  // other VRx backpack (i.e. reserved for FENIX or fusion etc.)
#endif

/////////// FUNCTION DEFS ///////////

void RebootIntoWifi();
void OnDataRecv(uint8_t * mac_addr, uint8_t *data, uint8_t data_len);
void ProcessMSPPacket(mspPacket_t *packet);
void SetSoftMACAddress();
void RequestVTXPacket();
void sendMSPViaEspnow(mspPacket_t *packet);
void resetBindingMode();
void checkIfInBindingMode();

/////////////////////////////////////

void RebootIntoWifi()
{
  DBGLN("Rebooting into wifi update mode...");
  startWebUpdater = true;
  EEPROM.put(EEPROM_ADDR_WIFI, startWebUpdater);
  EEPROM.commit();
  ESP.restart();
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
      if (bindingMode || 
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
        ProcessMSPPacket(msp.getReceivedPacket());
      }
      else
      {
        DBGLN("Failed MAC add check and not in bindingMode.");
      }
      msp.markPacketReceived();
    }
  }
  flashLedCounter++;
}

void ProcessMSPPacket(mspPacket_t *packet)
{
  if (bindingMode)
  {
    DBGLN("Processing Binding Packet...");
    if (packet->function == MSP_ELRS_BIND)
    {
      DBGLN("MSP_ELRS_BIND MAC = ");
      for (int i = 0; i < 6; i++)
      {
        EEPROM.put(EEPROM_MAC + i, packet->payload[i]);
        DBG("%x", packet->payload[i]); // Debug prints
        DBG(",");
      }
      DBG(""); // Extra line for serial output readability
      resetBindingMode();
      ESP.restart();
    }
    return;
  }

  switch (packet->function)
  {
  case MSP_SET_VTX_CONFIG:
    DBGLN("Processing MSP_SET_VTX_CONFIG...");
    gotInitialPacket = true;
    if (packet->payload[0] < 48) // Standard 48 channel VTx table size e.g. A, B, E, F, R, L
    {
      // only send new changes to the goggles
      // cache changes here, to be handled outside this callback, in the main loop
      uint8_t newBand = packet->payload[0] / 8 + 1;
      uint8_t newChannel = packet->payload[0] % 8;
      if (cachedBand != newBand)
      {
        cachedBand = newBand;
        sendChangesToVrx = true;
      }
      if (cachedChannel != newChannel)
      {
        cachedChannel = newChannel;
        sendChangesToVrx = true;
      }
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

void RequestVTXPacket()
{  
  mspPacket_t packet;
  packet.reset();
  packet.makeCommand();
  packet.function = MSP_ELRS_REQU_VTX_PKT;
  packet.addByte(0);  // empty byte

  sendMSPViaEspnow(&packet);
}

void sendMSPViaEspnow(mspPacket_t *packet)
{
  // Do not send while in binding mode.  The currently used broadcastAddress may be garbage.
  if (bindingMode)
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

void resetBindingMode()
{
  bootCounter = 0;
  bindingMode = false;
  EEPROM.put(EEPROM_RST_COUNTER, bootCounter);
  EEPROM.commit();
}

void checkIfInBindingMode()
{
  EEPROM.get(EEPROM_RST_COUNTER, bootCounter);
  bootCounter++;

  if (bootCounter > 2)
  {
    bindingMode = true;
  }
  else
  {
    EEPROM.put(EEPROM_RST_COUNTER, bootCounter);
    EEPROM.commit();
  }

  DBGLN(""); // blank line to clear below prints from serial boot garbage
  DBG("bootCounter = ");
  DBGLN("%x", bootCounter);
  DBG("bindingMode = ");
  DBGLN("%x", bindingMode);
}

void setup()
{
  Serial.begin(460800);

  EEPROM.begin(512);
  EEPROM.get(EEPROM_ADDR_WIFI, startWebUpdater);

  if (startWebUpdater)
  {
    EEPROM.put(EEPROM_ADDR_WIFI, false);
    EEPROM.commit();  
    BeginWebUpdate();
  }
  else
  {
    #ifndef MY_UID
      checkIfInBindingMode();
    #endif

    SetSoftMACAddress();

    if (esp_now_init() != 0)
    {
      DBGLN("Error initializing ESP-NOW");
      ESP.restart();
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    esp_now_register_recv_cb(OnDataRecv); 

    vrxModule.Init();
  }

  pinMode(WIFI_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  flashLedCounter = 2;
  DBGLN("Setup completed");
}

void loop()
{
  uint8_t buttonPressed = !digitalRead(WIFI_PIN);

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

    if (sendChangesToVrx)
    {
      sendChangesToVrx = false;
      // rapidfire sometimes misses pkts, so send each one 3x
      // Is spamming still required with reduced SPI freq?
      vrxModule.SendBandCmd(cachedBand);
      // vrxModule.SendBandCmd(cachedBand);
      // vrxModule.SendBandCmd(cachedBand);

      vrxModule.SendChannelCmd(cachedChannel);
      // vrxModule.SendChannelCmd(cachedChannel);
      // vrxModule.SendChannelCmd(cachedChannel);
    }

    // spam out a bunch of requests for the desired band/channel for the first 5s
    if (!gotInitialPacket && now < 5000 && now - lastSentRequest > 1000 && !bindingMode)
    {
      DBGLN("RequestVTXPacket...");
      RequestVTXPacket();
      lastSentRequest = now;
    }

    // Power cycle must be done within 30s.  Long timeout to allow goggles to boot and shutdown correctly e.g. Orqa.
    // If currently in binding the backpack will exit binding to prevent users getting caught out accidentally.
    if (now > BINDING_TIMEOUT && (bindingMode || bootCounter))
    {
      DBGLN("resetBindingMode...");
      resetBindingMode();
      digitalWrite(LED_PIN, HIGH);
    }
    
    if (bindingMode && now > nextBindingLedFlash)
    {
      nextBindingLedFlash += BINDING_LED_PAUSE;
      flashLedCounter = 2;
    }
  }

  // TODO make LED non blocking
  if (flashLedCounter)
  {
    flashLedCounter--;
    
    digitalWrite(LED_PIN, LOW);
    startWebUpdater == true ? delay(50) : delay(100);
    digitalWrite(LED_PIN, HIGH);
    startWebUpdater == true ? delay(50) : delay(100);
    if (bindingMode)
      digitalWrite(LED_PIN, LOW);
  }
}
