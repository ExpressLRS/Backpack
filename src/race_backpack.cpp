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

DynamicJsonDocument docPilotList(2048);
DynamicJsonDocument docTransient(2048);
DynamicJsonDocument docActiveBackpackSubs(2048);
JsonArray subscribers = docActiveBackpackSubs.createNestedArray("subscribers");
const char* testJson = "{\"subscribers\":[{\"name\":\"snipes\",\"UID\":[158,182,18,235,59,206]}]}";

/////////// CLASS OBJECTS ///////////

MSP msp;
ELRS_EEPROM eeprom;
TxBackpackConfig config;
mspPacket_t cachedVTXPacket;

/////////// FUNCTION DEFS ///////////

void sendMSPViaEspnow(uint8_t *address, mspPacket_t *packet);

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
  if (packet->function == MSP_ELRS_SUBSCRIBE_TO_RACE_BP)
  {
    uint8_t uid[6];
    for (uint8_t i = 0; i < 6; ++i)
    {
      uid[i] = packet->readByte();
    }
    
    uint8_t pilotNameLen = packet->readByte();
    char pilotName[pilotNameLen + 1];

    Serial.print("pilotNameLen = "); Serial.println(pilotNameLen);

    for (uint8_t i = 0; i < pilotNameLen; ++i)
    {
        pilotName[i] = packet->readByte();
        Serial.println(pilotName[i]);
    }

    pilotName[pilotNameLen] = '\0';

    // Iterate the collection of subscribers,
    // and if sub already exists, bail out
    Serial.print("subscribers.size() = "); Serial.println(subscribers.size());
    for (uint8_t i = 0; i < subscribers.size(); ++i)
    {
      if (subscribers[i]["name"] == pilotName)
      {
        Serial.println("subscribers[i][\"name\"] == pilotName");
        // JsonArray UID = subscribers[i].createNestedArray("UID");
        // for (uint8_t j = 0; j < 6; ++j)
        // {
        //   UID.add(uid[j]);
        // }
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

    serializeJson(subscribers, Serial);
    memcpy(peerInfo.peer_addr, uid, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      DBGLN("ESP-NOW failed to add peer");
      return;
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
  Serial.println("sent");

  blinkLED();
}

void SendCachedMSP()
{
  
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

Serial2.begin(115200);

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
    DeserializationError error = deserializeJson(docTransient, Serial);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    }
    else
    {
      // StaticJsonDocument<512> docTest;
      // deserializeJson(docTest, testJson);

      bool isPilotList = docTransient.containsKey("Pilots");
      bool isRaceState = docTransient.containsKey("State");
      bool isDetection = docTransient.containsKey("PilotName");

      if (isPilotList)
      {
        docPilotList = docTransient;

        // serializeJson(docTest, Serial);
        // serializeJson(docPilotList, Serial);
        // subscribers = docTest["subscribers"];

        JsonArray pilots = docPilotList["Pilots"];

        serializeJson(pilots, Serial);
        Serial.println(pilots.size());
        Serial.println(subscribers.size());

        for (uint8_t i = 0; i < pilots.size(); ++i)
        {
          for (uint8_t j = 0; j < subscribers.size(); ++j)
          {
            const char* pilotName = pilots[i]["Name"];
            const char* subName = subscribers[j]["name"];
            Serial.println(pilotName);
            Serial.println(subName);
            if (subscribers[j]["name"] == pilots[i]["Name"])
            {
              Serial.println("Got match in subscribers array for pilot list");

              const char* band = pilots[i]["ChannelBand"];
              uint8_t ch = pilots[i]["ChannelNumber"];

              ch--; // one-based to zero-based

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

              Serial.println(ch);
              Serial.println("===");

              uint8_t uid[6];

              JsonArray UID = subscribers[j]["UID"];
              for (uint8_t uidIndex = 0; uidIndex < UID.size(); ++uidIndex)
              {
                uid[uidIndex] = UID[uidIndex].as<int>();
                Serial.println(uid[uidIndex]);
              }

              mspPacket_t packet;
              packet.reset();
              packet.makeCommand();
              packet.function = MSP_SET_VTX_CONFIG;
              packet.addByte(ch);
              sendMSPViaEspnow(uid, &packet);
            }
          }
        }
      }

      if (isDetection)
      {
        const char* pilotName = docTransient["PilotName"];
        double time = docTransient["Time"];
        Serial.print("Got detection for ");
        Serial.print(pilotName);
        Serial.println(" with time: ");
        Serial.println(time);

        // Iterate the collection of subscribers
        for (uint8_t i = 0; i < subscribers.size(); ++i)
        {
          if (subscribers[i]["name"] == pilotName)
          {
            Serial.println("got match in subscribers array");

            for (uint8_t j = 0; j < 6; ++j)
            {
              uint8_t val = subscribers[i]["UID"][j];
              Serial.println(val);
            }
          }
        }
      }
      // serializeJson(docTransient, Serial);
    }
  }

  if (cacheFull && sendCached)
  {
    SendCachedMSP();
    sendCached = false;
  }
}
