#include <Arduino.h>

#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
  #include <esp_wifi.h>
  #include <WiFi.h>
#endif

#include <ArduinoJson.h>

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

uint8_t raceBPAddress[6] = {238,254,186,226,83,164};

connectionState_e connectionState = starting;
unsigned long rebootTime = 0;

device_t *ui_devices[] = {
#ifdef PIN_LED
  &LED_device,
#endif
#ifdef PIN_BUTTON
  &Button_device,
#endif
  &WIFI_device,
};

#if defined(PLATFORM_ESP32)
// This seems to need to be global, as per this page,
// otherwise we get errors about invalid peer:
// https://rntlab.com/question/espnow-peer-interface-is-invalid/
esp_now_peer_info_t peerInfo;
#endif

DynamicJsonDocument jsonReceived(4096);
DynamicJsonDocument jsonPilotList(4096);
DynamicJsonDocument jsonActiveBackpackSubs(4096);
JsonArray subscribers = jsonActiveBackpackSubs.createNestedArray("subscribers");

const char* jsonTestSubs = "{\"subscribers\":[{\"name\":\"Test Pilot\",\"UID\":[10,20,30,40,50,60]}]}"; // TODO: Remove when no longer needed

/////////// CLASS OBJECTS ///////////

MSP msp;
ELRS_EEPROM eeprom;
TxBackpackConfig config;

/////////// FUNCTION DEFS ///////////

void sendMSPViaEspnow(uint8_t *address, mspPacket_t *packet);

/////////////////////////////////////

void SendChannelIndexToPeer(uint8_t *address, uint8_t vtxIdx)
{
  mspPacket_t packet;
  packet.reset();
  packet.makeCommand();
  packet.function = MSP_SET_VTX_CONFIG;
  packet.addByte(vtxIdx);
  sendMSPViaEspnow(address, &packet);
}

void SendRaceStateToPeer(uint8_t *address, JsonObject raceState)
{
  uint8_t round = raceState["Round"];
  uint8_t race = raceState["Race"];
  const char* state = raceState["State"];

  uint8_t stateLen = strlen(state);
  
  mspPacket_t packet;
  packet.reset();
  packet.makeCommand();
  packet.function = MSP_ELRS_RACE_STATE;
  packet.addByte(round);
  packet.addByte(race);
  packet.addByte(stateLen);
  for (uint8_t i = 0; i < stateLen; ++i)
  {
    packet.addByte(state[i]);
  }
  sendMSPViaEspnow(address, &packet);
}

void SendDetectionToPeer(uint8_t *address, JsonObject detection)
{
  uint8_t lapNumber = detection["LapNumber"];
  double time = detection["Time"];
  bool isLapEnd = detection["IsLapEnd"];
  bool isRaceEnd = detection["IsRaceEnd"];

  char timeAsText[20];
  int result = sprintf(timeAsText, "%.2f", time);
  if (result < 0)
  {
    DBGLN("Failed to convert time to text for detection");
    return;
  }

  uint8_t timeLen = strlen(timeAsText);
  
  mspPacket_t packet;
  packet.reset();
  packet.makeCommand();
  packet.function = MSP_ELRS_RACE_LAP_DETECTION;
  packet.addByte(lapNumber);
  packet.addByte(isLapEnd ? 1 : 0);
  packet.addByte(isRaceEnd ? 1 : 0);
  packet.addByte(timeLen);
  for (uint8_t i = 0; i < timeLen; ++i)
  {
    packet.addByte(timeAsText[i]);
  }
  sendMSPViaEspnow(address, &packet);
}

int16_t ParseVtxIdxFromPilotJson(JsonObject pilot)
{
  const char* band = pilot["ChannelBand"];
  uint8_t ch = pilot["ChannelNumber"];

  ch--; // one-based to zero-based

  // Convert to standard 48ch table index
  if (strcmp(band, "A") == 0)
  {
    ch += 0;
  }
  if (strcmp(band, "B") == 0)
  {
    ch += 8;
  }
  if (strcmp(band, "E") == 0)
  {
    ch += 16;
  }
  if (strcmp(band, "Fatshark") == 0)
  {
    ch += 24;
  }
  if (strcmp(band, "Raceband") == 0)
  {
    ch += 32;
  }
  if (strcmp(band, "LowBand") == 0)
  {
    ch += 40;
  }

  if (ch >= 0 || ch <= 48)
  {
    return ch;
  }
  else
  {
    return -1;
  }
}

void ParseUIDFromSubscriber(uint8_t* uid, JsonObject subscriber)
{
  JsonArray UID = subscriber["UID"];
  for (uint8_t i = 0; i < UID.size(); ++i)
  {
    uid[i] = UID[i].as<int>();
  }
}

void ProcessJsonPilotListFromTimer()
{
  // StaticJsonDocument<512> docTest; // TODO: Remove
  // deserializeJson(docTest, jsonTestSubs);
  // subscribers = docTest["subscribers"];
  // serializeJson(docTest, Serial);

  JsonArray pilots = jsonPilotList["Pilots"];

  #if defined(DEBUG_LOG) || defined(DEBUG_LOG_VERBOSE)
    DBGLN("Parse pilots from race timing software:");
    serializeJson(pilots, Serial);
    DBGLN("");
  #endif

  for (uint8_t i = 0; i < pilots.size(); ++i)
  {
    for (uint8_t j = 0; j < subscribers.size(); ++j)
    {
      if (subscribers[j]["name"] == pilots[i]["Name"])
      {
        const char* subName = subscribers[j]["name"];
        DBGLN("Got match in subscribers array from pilot list for sub: %s", subName);

        int16_t vtxIdx = ParseVtxIdxFromPilotJson(pilots[i]);
        if (vtxIdx == -1)
        {
          DBGLN("Failed to parse vtxIdx for subscriber from timing software");
          return;
        }

        uint8_t uid[6];
        ParseUIDFromSubscriber(uid, subscribers[j]);

        DBGLN("Sending channel index %d to %s", vtxIdx, subName);
        SendChannelIndexToPeer(uid, vtxIdx);
      }
    }
  }
}

void ProcessJsonRaceStateFromTimer()
{
  // Iterate the collection of subscribers
  for (uint8_t i = 0; i < subscribers.size(); ++i)
  {
    uint8_t uid[6];
    ParseUIDFromSubscriber(uid, subscribers[i]);
    
    SendRaceStateToPeer(uid, jsonReceived.as<JsonObject>());
  }
}

void ProcessJsonDetectionFromTimer()
{
  const char* pilotName = jsonReceived["PilotName"];
  double time = jsonReceived["Time"];
  DBG("Got detection for pilot %s with time ", pilotName);

  char timeAsText[20];
  sprintf(timeAsText, "%f", time);
  DBGLN(timeAsText);

  // Iterate the collection of subscribers
  for (uint8_t i = 0; i < subscribers.size(); ++i)
  {
    if (subscribers[i]["name"] == pilotName)
    {
      uint8_t uid[6];
      ParseUIDFromSubscriber(uid, subscribers[i]);

      SendDetectionToPeer(uid, jsonReceived.as<JsonObject>());
    }
  }
}

void ProcessSubscribePacket(mspPacket_t *packet)
{
  // function opcode:   MSP_ELRS_SUBSCRIBE_TO_RACE_BP
  // uint8_t[6]:        Subcriber's UID
  // uint8_t:           Pilot name length (n bytes)
  // char[n]            Subcriber's pilot name (c-string. string terminator NOT included)

  uint8_t uid[6];
  for (uint8_t i = 0; i < 6; ++i)
  {
    uid[i] = packet->readByte();
  }
  
  uint8_t pilotNameLen = packet->readByte();
  if (pilotNameLen == 0)
  {
    DBGLN("Subscriber pilot name cannot be blank");
    return;
  }

  char pilotName[pilotNameLen + 1];
  for (uint8_t i = 0; i < pilotNameLen; ++i)
  {
      pilotName[i] = packet->readByte();
  }
  pilotName[pilotNameLen] = '\0';

  // Iterate the collection of subscribers,
  // and if sub already exists, bail out
  for (uint8_t i = 0; i < subscribers.size(); ++i)
  {
    if (subscribers[i]["name"] == pilotName)
    {
      return;
    }
  }

  // Subscriber wasn't found in the collection,
  // add name + UID as a new entry
  JsonObject pilot = subscribers.createNestedObject();
  pilot["name"] = pilotName;
  JsonArray UID = pilot.createNestedArray("UID");
  for (uint8_t i = 0; i < 6; ++i)
  {
    UID.add(uid[i]);
  }

  // Add the new subscriber's UID as a ESPNOW peer
  memcpy(peerInfo.peer_addr, uid, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    DBGLN("ESP-NOW failed to add subscriber peer");
    return;
  }

  #if defined(DEBUG_LOG) || defined(DEBUG_LOG_VERBOSE)
    DBGLN("Successfully added new subscriber peer to collection:");
    serializeJson(subscribers, Serial);
    DBGLN("");
  #endif

  // Check the existing pilot list from the timer
  // and update subscriber's channel if available
  if (!jsonPilotList.isNull())
  {
    ProcessJsonPilotListFromTimer();
  }
}

void RebootIntoWifi()
{
  DBGLN("Rebooting into wifi update mode...");
  config.SetStartWiFiOnBoot(true);
  config.Commit();
  rebootTime = millis();
}

void ProcessMSPPacketFromPeer(mspPacket_t *packet)
{
  if (packet->function == MSP_ELRS_SUBSCRIBE_TO_RACE_BP)
  {
    ProcessSubscribePacket(packet);
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
      ProcessMSPPacketFromPeer(msp.getReceivedPacket());
      msp.markPacketReceived();
    }
  }
  blinkLED();
}

void ProcessMSPPacketFromTX(mspPacket_t *packet)
{
  
}

void sendMSPViaEspnow(uint8_t *address, mspPacket_t *packet)
{
  uint8_t packetSize = msp.getTotalPacketSize(packet);
  uint8_t nowDataOutput[packetSize];

  uint8_t result = msp.convertToByteArray(packet, nowDataOutput);

  if (!result)
  {
    // packet could not be converted to array, bail out
    return;
  }

  esp_now_send(address, (uint8_t *) &nowDataOutput, packetSize);

  blinkLED();
}

void SetSoftMACAddress()
{
  // MAC address can only be set with unicast, so first byte must be even, not odd
  raceBPAddress[0] = raceBPAddress[0] & ~0x01;

  WiFi.mode(WIFI_STA);
  WiFi.begin("network-name", "pass-to-network", 1);
  WiFi.disconnect();

  // Soft-set the MAC address to the passphrase UID for binding
  #if defined(PLATFORM_ESP8266)
    wifi_set_macaddr(STATION_IF, raceBPAddress);
  #elif defined(PLATFORM_ESP32)
    esp_wifi_set_mac(WIFI_IF_STA, raceBPAddress);
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

//////////////////

void setup()
{
  Serial.begin(115200);

  // #ifdef DEBUG_LOG
  //   Serial1.begin(115200);
  //   Serial.setDebugOutput(true);
  //   Serial2.begin(115200);
  // #endif

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
      esp_now_add_peer(raceBPAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    #elif defined(PLATFORM_ESP32)
      memcpy(peerInfo.peer_addr, raceBPAddress, 6);
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
    DeserializationError error = deserializeJson(jsonReceived, Serial);

    if (error)
    {
      DBG("deserializeJson() failed: ");
      DBGLN(error.c_str());
    }
    else
    {
      if (jsonReceived.containsKey("Pilots"))
      {
        jsonPilotList = jsonReceived;
        ProcessJsonPilotListFromTimer();
      }
      if (jsonReceived.containsKey("State"))
      {
        ProcessJsonRaceStateFromTimer();
      }
      if (jsonReceived.containsKey("PilotName"))
      {
        ProcessJsonDetectionFromTimer();
      }
    }
  }
}
