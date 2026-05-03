#include <Arduino.h>

#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
  #include <esp_wifi.h>
  #include <esp_system.h>
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

#include "trainer_api.h"

#if defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
#define TRAINER_BACKPACK_SUPPORTED 1
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
void sendMSPViaWiFiUDP(mspPacket_t *packet);

/////////////////////////////////////

#if defined(PLATFORM_ESP32)
// This seems to need to be global, as per this page,
// otherwise we get errors about invalid peer:
// https://rntlab.com/question/espnow-peer-interface-is-invalid/
esp_now_peer_info_t peerInfo;
esp_now_peer_info_t bindingInfo;
#endif

#if defined(TRAINER_BACKPACK_SUPPORTED)
static bool trainerInPairing = false;
static bool trainerEspNowReady = false;
static uint32_t trainerPairingStartMs = 0;
static uint32_t lastTrainerPairBroadcastMs = 0;
static const uint32_t TRAINER_PAIRING_TIMEOUT_MS = 30000;
static const uint32_t TRAINER_PAIRING_BROADCAST_MS = 500;
static const uint32_t TRAINER_STATUS_PERIOD_MS = 1000;
static const uint8_t TRAINER_PEER_MAC_LAST_BYTE_INDEX = 5;
static uint32_t lastTrainerStatusSendMs = 0;
static trainer_mode_t trainerMode = TRAINER_MODE_OFF;
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

#if defined(TRAINER_BACKPACK_SUPPORTED)
static constexpr uint8_t TRAINER_ID_SIZE = 6;
static constexpr uint8_t TRAINER_PAIR_REQUEST_PAYLOAD_SIZE = 2 + TRAINER_ID_SIZE;
static constexpr uint8_t TRAINER_PAIR_ACK_PAYLOAD_SIZE = 2 + TRAINER_ID_SIZE + TRAINER_ID_SIZE;
static constexpr uint8_t TRAINER_FORGET_PAYLOAD_SIZE = 2 + TRAINER_ID_SIZE;
static constexpr uint8_t TRAINER_CHANNEL_COUNT = 16;
static constexpr uint8_t TRAINER_CHANNEL_PAYLOAD_SIZE = TRAINER_CHANNEL_COUNT * 2;
static constexpr uint8_t TRAINER_MAGIC_0 = 'T';
static constexpr uint8_t TRAINER_MAGIC_1 = 'R';
static uint8_t trainerLocalMac[TRAINER_ID_SIZE] = {};
static uint8_t trainerPeerEspNowMac[TRAINER_ID_SIZE] = {};
static bool trainerLocalMacReady = false;
static bool trainerPeerEspNowMacReady = false;
static void SendTrainerStatusToTX();
#if defined(PLATFORM_ESP32)
static portMUX_TYPE trainerPendingMux = portMUX_INITIALIZER_UNLOCKED;
#define TRAINER_CRITICAL_ENTER() portENTER_CRITICAL(&trainerPendingMux)
#define TRAINER_CRITICAL_EXIT()  portEXIT_CRITICAL(&trainerPendingMux)
#else
#define TRAINER_CRITICAL_ENTER() noInterrupts()
#define TRAINER_CRITICAL_EXIT()  interrupts()
#endif
struct trainerPendingPacket_t
{
    mspPacket_t packet;
    uint8_t sourceMac[TRAINER_ID_SIZE];
};

static trainerPendingPacket_t pendingTrainerPairRequestPacket;
static trainerPendingPacket_t pendingTrainerPairAckPacket;
static volatile bool pendingTrainerPairRequest = false;
static volatile bool pendingTrainerPairAck = false;
static uint8_t pendingTrainerChannelsPayload[TRAINER_CHANNEL_PAYLOAD_SIZE];
static volatile bool pendingTrainerChannels = false;
static constexpr uint8_t TRAINER_FORGET_RETRY_COUNT = 10;
static constexpr uint32_t TRAINER_FORGET_RETRY_MS = 100;
static uint8_t trainerForgetRetriesRemaining = 0;
static uint32_t lastTrainerForgetSendMs = 0;

static bool trainerAddPeer(const uint8_t *mac)
{
    if (!trainerEspNowReady)
    {
        return false;
    }

#if defined(PLATFORM_ESP8266)
    uint8_t *peer = const_cast<uint8_t *>(mac);
    if (!esp_now_is_peer_exist(peer))
    {
        int result = esp_now_add_peer(peer, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
        if (result != 0)
        {
            return false;
        }
    }
#elif defined(PLATFORM_ESP32)
    esp_now_peer_info_t info = {};
    memcpy(info.peer_addr, mac, TRAINER_ID_SIZE);
    info.channel = 0;
    info.encrypt = false;

    if (!esp_now_is_peer_exist(mac))
    {
        esp_err_t result = esp_now_add_peer(&info);
        if (result != ESP_OK && result != ESP_ERR_ESPNOW_EXIST)
        {
            return false;
        }
    }
#endif
    return true;
}

static bool sendTrainerMSPViaEspNow(const uint8_t *destMac, mspPacket_t *packet)
{
    if (!trainerEspNowReady)
    {
        return false;
    }

    static constexpr size_t TRAINER_MSP_BUFFER_SIZE = MSP_PORT_INBUF_SIZE + 9;
    const size_t packetSize = msp.getTotalPacketSize(packet);
    if (packetSize == 0 || packetSize > TRAINER_MSP_BUFFER_SIZE)
    {
        return false;
    }

    uint8_t buffer[TRAINER_MSP_BUFFER_SIZE];
    if (!msp.convertToByteArray(packet, buffer))
    {
        return false;
    }
#if defined(PLATFORM_ESP8266)
    int result = esp_now_send(const_cast<uint8_t *>(destMac), buffer, (int)packetSize);
    if (result != 0)
    {
        return false;
    }
#elif defined(PLATFORM_ESP32)
    esp_err_t result = esp_now_send(destMac, buffer, packetSize);
    if (result != ESP_OK)
    {
        return false;
    }
#endif
    return true;
}

static bool trainerHasAddress(const uint8_t *mac)
{
    for (uint8_t i = 0; i < TRAINER_ID_SIZE; i++)
    {
        if (mac[i] != 0) return true;
    }
    return false;
}

static bool trainerHasLocalMac()
{
    return trainerHasAddress(trainerLocalMac);
}

static const uint8_t *trainerEspNowLocalAddress()
{
    return firmwareOptions.uid;
}

static bool trainerIsLocalEspNowMac(const uint8_t *mac)
{
    return memcmp(mac, trainerEspNowLocalAddress(), TRAINER_ID_SIZE) == 0;
}

static bool trainerEspNowMacUsable(const uint8_t *mac)
{
    return mac && trainerHasAddress(mac) && memcmp(mac, bindingAddress, TRAINER_ID_SIZE) != 0 && !trainerIsLocalEspNowMac(mac);
}

static void CaptureTrainerLocalMac()
{
#if defined(PLATFORM_ESP8266)
    WiFi.macAddress(trainerLocalMac);
#elif defined(PLATFORM_ESP32)
    esp_efuse_mac_get_default(trainerLocalMac);
#endif
    trainerLocalMacReady = trainerHasLocalMac();
}

static bool trainerIsLocalMac(const uint8_t *mac)
{
    return trainerLocalMacReady && memcmp(mac, trainerLocalMac, TRAINER_ID_SIZE) == 0;
}

static bool trainerPayloadHasMagic(const mspPacket_t *packet)
{
    return packet->payloadSize >= 2 &&
           packet->payload[0] == TRAINER_MAGIC_0 &&
           packet->payload[1] == TRAINER_MAGIC_1;
}

static void addTrainerLocalMac(mspPacket_t *packet)
{
    for (uint8_t i = 0; i < TRAINER_ID_SIZE; i++)
    {
        packet->addByte(trainerLocalMac[i]);
    }
}

static bool getTrainerPairRequestPeer(const mspPacket_t *packet, const uint8_t **peerMac)
{
    if (packet->payloadSize != TRAINER_PAIR_REQUEST_PAYLOAD_SIZE)
    {
        return false;
    }
    if (!trainerPayloadHasMagic(packet))
    {
        return false;
    }

    const uint8_t *candidate = &packet->payload[2];
    if (trainerIsLocalMac(candidate))
    {
        return false;
    }

    *peerMac = candidate;
    return true;
}

static bool getTrainerPairAckPeer(const mspPacket_t *packet, const uint8_t **peerMac)
{
    if (packet->payloadSize != TRAINER_PAIR_ACK_PAYLOAD_SIZE)
    {
        return false;
    }
    if (!trainerPayloadHasMagic(packet))
    {
        return false;
    }
    if (memcmp(&packet->payload[2], trainerLocalMac, TRAINER_ID_SIZE) != 0)
    {
        return false;
    }

    const uint8_t *candidate = &packet->payload[2 + TRAINER_ID_SIZE];
    if (trainerIsLocalMac(candidate))
    {
        return false;
    }

    *peerMac = candidate;
    return true;
}

static bool trainerPayloadFromPairedPeer(const mspPacket_t *packet)
{
    return config.IsTrainerPaired() &&
           packet->payloadSize >= TRAINER_ID_SIZE &&
           memcmp(packet->payload, config.GetTrainerPeerMac(), TRAINER_ID_SIZE) == 0;
}

static const uint8_t *trainerEspNowPairAddress()
{
    return bindingAddress;
}

static void RememberTrainerPeerEspNowMac(const uint8_t *mac)
{
    if (!trainerEspNowMacUsable(mac))
    {
        return;
    }
    if (trainerPeerEspNowMacReady && memcmp(mac, trainerPeerEspNowMac, TRAINER_ID_SIZE) == 0)
    {
        return;
    }
    if (trainerAddPeer(mac))
    {
        memcpy(trainerPeerEspNowMac, mac, TRAINER_ID_SIZE);
        trainerPeerEspNowMacReady = true;
    }
}

static const uint8_t *trainerEspNowSendAddress()
{
    return trainerPeerEspNowMacReady ? trainerPeerEspNowMac : trainerEspNowPairAddress();
}

static void SendTrainerPairRequest()
{
    if (!trainerLocalMacReady || !trainerHasLocalMac())
    {
        return;
    }

    mspPacket_t pkt;
    pkt.reset();
    pkt.makeCommand();
    pkt.function = MSP_ELRS_BACKPACK_TRAINER_PAIR_REQ;
    pkt.addByte(TRAINER_MAGIC_0);
    pkt.addByte(TRAINER_MAGIC_1);
    addTrainerLocalMac(&pkt);
    sendTrainerMSPViaEspNow(trainerEspNowPairAddress(), &pkt);
}

static void SendTrainerPairAck(const uint8_t *peerMac)
{
    if (!trainerLocalMacReady || !trainerHasLocalMac())
    {
        return;
    }
    // Broadcast: after esp_wifi_set_mac() the master's STA MAC is the UID-derived
    // address shared by all bound devices. Unicast to the hardware MAC is dropped
    // by the master's WiFi filter. Broadcast is received by all; the master filters
    // via the AKTGT payload check (payload[2..7] == master's hardware MAC).

    mspPacket_t ack;
    ack.reset();
    ack.makeCommand();
    ack.function = MSP_ELRS_BACKPACK_TRAINER_PAIR_ACK;
    ack.addByte(TRAINER_MAGIC_0);
    ack.addByte(TRAINER_MAGIC_1);
    for (uint8_t i = 0; i < TRAINER_ID_SIZE; i++)
    {
        ack.addByte(peerMac[i]);
    }
    addTrainerLocalMac(&ack);
    sendTrainerMSPViaEspNow(trainerEspNowPairAddress(), &ack); // broadcast
}

static bool TrainerPeerMatchesConfig(const uint8_t *peerMac)
{
    return config.IsTrainerPaired() &&
           memcmp(peerMac, config.GetTrainerPeerMac(), TRAINER_ID_SIZE) == 0;
}

static void CompleteTrainerPairing(const uint8_t *peerMac, const uint8_t *peerEspNowMac)
{
    const bool samePeer = TrainerPeerMatchesConfig(peerMac);
    trainerPeerEspNowMacReady = false;
    RememberTrainerPeerEspNowMac(peerEspNowMac);
    if (!samePeer)
    {
        config.SetTrainerPeer(peerMac);
        config.Commit();
    }
    trainerInPairing = false;
    SendTrainerStatusToTX();
}

static void ProcessTrainerPairRequest(mspPacket_t *packet, const uint8_t *sourceMac)
{
    if (trainerMode != TRAINER_MODE_SLAVE)
    {
        return;
    }

    const uint8_t *peerMac;
    if (!getTrainerPairRequestPeer(packet, &peerMac)) return;

    if (!trainerInPairing && !TrainerPeerMatchesConfig(peerMac))
    {
        return;
    }

    SendTrainerPairAck(peerMac);
    CompleteTrainerPairing(peerMac, sourceMac);
}

static void ProcessTrainerPairAck(mspPacket_t *packet, const uint8_t *sourceMac)
{
    if (trainerMode != TRAINER_MODE_MASTER)
    {
        return;
    }

    const uint8_t *peerMac;
    if (!getTrainerPairAckPeer(packet, &peerMac)) return;

    if (!trainerInPairing && !TrainerPeerMatchesConfig(peerMac))
    {
        return;
    }

    CompleteTrainerPairing(peerMac, sourceMac);
}

// ESP-NOW receive callbacks run in the Wi-Fi task; process send/commit work from loop().
static void ClearPendingTrainerPairPackets()
{
    TRAINER_CRITICAL_ENTER();
    pendingTrainerPairRequest = false;
    pendingTrainerPairAck = false;
    TRAINER_CRITICAL_EXIT();
}

static void StopTrainerForWifi()
{
    trainerInPairing = false;
    trainerForgetRetriesRemaining = 0;
    ClearPendingTrainerPairPackets();
}

static void StorePendingTrainerPairPacket(mspPacket_t *packet, const uint8_t *sourceMac)
{
    TRAINER_CRITICAL_ENTER();
    if (packet->function == MSP_ELRS_BACKPACK_TRAINER_PAIR_REQ)
    {
        if (!pendingTrainerPairRequest)
        {
            pendingTrainerPairRequestPacket.packet = *packet;
            memcpy(pendingTrainerPairRequestPacket.sourceMac, sourceMac, TRAINER_ID_SIZE);
            pendingTrainerPairRequest = true;
        }
    }
    else if (packet->function == MSP_ELRS_BACKPACK_TRAINER_PAIR_ACK)
    {
        if (!pendingTrainerPairAck)
        {
            pendingTrainerPairAckPacket.packet = *packet;
            memcpy(pendingTrainerPairAckPacket.sourceMac, sourceMac, TRAINER_ID_SIZE);
            pendingTrainerPairAck = true;
        }
    }
    TRAINER_CRITICAL_EXIT();
}

static bool FetchPendingTrainerPairRequest(mspPacket_t *packet, uint8_t *sourceMac)
{
    bool hasPacket = false;

    TRAINER_CRITICAL_ENTER();
    if (pendingTrainerPairRequest)
    {
        *packet = pendingTrainerPairRequestPacket.packet;
        memcpy(sourceMac, pendingTrainerPairRequestPacket.sourceMac, TRAINER_ID_SIZE);
        pendingTrainerPairRequest = false;
        hasPacket = true;
    }
    TRAINER_CRITICAL_EXIT();

    return hasPacket;
}

static bool FetchPendingTrainerPairAck(mspPacket_t *packet, uint8_t *sourceMac)
{
    bool hasPacket = false;

    TRAINER_CRITICAL_ENTER();
    if (pendingTrainerPairAck)
    {
        *packet = pendingTrainerPairAckPacket.packet;
        memcpy(sourceMac, pendingTrainerPairAckPacket.sourceMac, TRAINER_ID_SIZE);
        pendingTrainerPairAck = false;
        hasPacket = true;
    }
    TRAINER_CRITICAL_EXIT();

    return hasPacket;
}

static void ProcessPendingTrainerPairPackets()
{
    mspPacket_t packet;
    uint8_t sourceMac[TRAINER_ID_SIZE];

    if (FetchPendingTrainerPairRequest(&packet, sourceMac))
    {
        ProcessTrainerPairRequest(&packet, sourceMac);
    }

    if (FetchPendingTrainerPairAck(&packet, sourceMac))
    {
        ProcessTrainerPairAck(&packet, sourceMac);
    }
}

static void FlushPendingTrainerChannels()
{
    if (!pendingTrainerChannels) return;

    uint8_t payload[TRAINER_CHANNEL_PAYLOAD_SIZE];
    TRAINER_CRITICAL_ENTER();
    memcpy(payload, pendingTrainerChannelsPayload, TRAINER_CHANNEL_PAYLOAD_SIZE);
    pendingTrainerChannels = false;
    TRAINER_CRITICAL_EXIT();

    mspPacket_t out;
    out.reset();
    out.makeCommand();
    out.function = MSP_ELRS_BACKPACK_TRAINER_CHANNELS;
    for (uint8_t i = 0; i < TRAINER_CHANNEL_PAYLOAD_SIZE; i++)
    {
        out.addByte(payload[i]);
    }
    msp.sendPacket(&out, &Serial);
}

static bool ProcessMSPPacketFromTrainerPeer(mspPacket_t *packet)
{
    if (packet->function == MSP_ELRS_BACKPACK_TRAINER_CHANNELS)
    {
        if (trainerMode != TRAINER_MODE_MASTER) return false;
        if (packet->payloadSize != TRAINER_ID_SIZE + TRAINER_CHANNEL_PAYLOAD_SIZE) return false;
        if (!trainerPayloadFromPairedPeer(packet)) return false;

        TRAINER_CRITICAL_ENTER();
        memcpy(pendingTrainerChannelsPayload, &packet->payload[TRAINER_ID_SIZE], TRAINER_CHANNEL_PAYLOAD_SIZE);
        pendingTrainerChannels = true;
        TRAINER_CRITICAL_EXIT();
        return true;
    }
    else if (packet->function == MSP_ELRS_BACKPACK_TRAINER_FORGET)
    {
        if (packet->payloadSize != TRAINER_FORGET_PAYLOAD_SIZE) return false;
        if (!trainerPayloadHasMagic(packet)) return false;

        const uint8_t *peerMac = &packet->payload[2];
        if (trainerIsLocalMac(peerMac)) return false;

        if (config.IsTrainerPaired())
        {
            if (memcmp(peerMac, config.GetTrainerPeerMac(), TRAINER_ID_SIZE) != 0) return false;
            config.ClearTrainerPeer();
            config.Commit();
            SendTrainerStatusToTX();
        }
        else if (!trainerInPairing)
        {
            return false;
        }

        trainerInPairing = false;
        return true;
    }

    return false;
}

static void sendTrainerChannelsViaEspNow(mspPacket_t *packet)
{
    if (!config.IsTrainerPaired()) return;
    if (!trainerLocalMacReady || !trainerHasLocalMac())
    {
        return;
    }
    if (packet->payloadSize + TRAINER_ID_SIZE > MSP_PORT_INBUF_SIZE)
    {
        return;
    }

    mspPacket_t out;
    out.reset();
    out.makeCommand();
    out.function = packet->function;
    addTrainerLocalMac(&out);
    for (uint8_t i = 0; i < packet->payloadSize; i++)
    {
        out.addByte(packet->payload[i]);
    }
    sendTrainerMSPViaEspNow(trainerEspNowSendAddress(), &out);
}

static bool StartTrainerPairing()
{
    if (!trainerEspNowReady)
    {
        return false;
    }
    if (trainerMode == TRAINER_MODE_OFF)
    {
        return false;
    }
    if (!trainerLocalMacReady || !trainerHasLocalMac())
    {
        return false;
    }
    if (!trainerAddPeer(trainerEspNowPairAddress()))
    {
        return false;
    }

    ClearPendingTrainerPairPackets();
    trainerInPairing = true;
    trainerPairingStartMs = millis();
    lastTrainerPairBroadcastMs = trainerPairingStartMs;
    if (trainerMode == TRAINER_MODE_MASTER)
    {
        SendTrainerPairRequest();
    }
    SendTrainerStatusToTX();
    return true;
}

bool TrainerStartPairing()
{
    return StartTrainerPairing();
}

static void SendTrainerForget()
{
    if (!trainerLocalMacReady || !trainerHasLocalMac())
    {
        return;
    }

    mspPacket_t pkt;
    pkt.reset();
    pkt.makeCommand();
    pkt.function = MSP_ELRS_BACKPACK_TRAINER_FORGET;
    pkt.addByte(TRAINER_MAGIC_0);
    pkt.addByte(TRAINER_MAGIC_1);
    addTrainerLocalMac(&pkt);
    sendTrainerMSPViaEspNow(trainerEspNowSendAddress(), &pkt);
}

void TrainerForgetPeer()
{
    trainerInPairing = false;
    trainerForgetRetriesRemaining = TRAINER_FORGET_RETRY_COUNT;
    lastTrainerForgetSendMs = millis();
    SendTrainerForget();
    if (trainerForgetRetriesRemaining > 0)
    {
        trainerForgetRetriesRemaining--;
    }
    trainerPeerEspNowMacReady = false;
    config.ClearTrainerPeer();
    config.Commit();
    SendTrainerStatusToTX();
}

static void TrainerForgetLoop(uint32_t now)
{
    if (trainerForgetRetriesRemaining == 0)
    {
        return;
    }
    if (now - lastTrainerForgetSendMs < TRAINER_FORGET_RETRY_MS)
    {
        return;
    }
    lastTrainerForgetSendMs = now;
    SendTrainerForget();
    trainerForgetRetriesRemaining--;
}

bool TrainerIsAvailable()
{
    return trainerEspNowReady;
}

bool TrainerIsPaired()
{
    return config.IsTrainerPaired();
}

bool TrainerIsPairing()
{
    return trainerInPairing;
}

const uint8_t *TrainerGetPeerMac()
{
    return config.GetTrainerPeerMac();
}

static void TrainerPairingLoop(uint32_t now)
{
    if (!trainerInPairing) return;
    if (now - trainerPairingStartMs >= TRAINER_PAIRING_TIMEOUT_MS)
    {
        trainerInPairing = false;
        SendTrainerStatusToTX();
        return;
    }
    if (trainerMode != TRAINER_MODE_MASTER) return;
    if (now - lastTrainerPairBroadcastMs > TRAINER_PAIRING_BROADCAST_MS)
    {
        lastTrainerPairBroadcastMs = now;
        SendTrainerPairRequest();
    }
}
#endif

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
      mspPacket_t *p = recv_msp.getReceivedPacket();
      bool fromBindPeer = (memcmp(mac_addr, firmwareOptions.uid, 6) == 0);
#if defined(TRAINER_BACKPACK_SUPPORTED)
      if (p->function == MSP_ELRS_BACKPACK_TRAINER_PAIR_REQ)
      {
          StorePendingTrainerPairPacket(p, mac_addr);
      }
      else if (p->function == MSP_ELRS_BACKPACK_TRAINER_PAIR_ACK)
      {
          if (trainerInPairing)
          {
              StorePendingTrainerPairPacket(p, mac_addr);
          }
      }
      else if (!ProcessMSPPacketFromTrainerPeer(p) && fromBindPeer)
      {
        ProcessMSPPacketFromPeer(p);
      }
#else
      if (fromBindPeer)
      {
        ProcessMSPPacketFromPeer(p);
      }
#endif
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
  if (connectionState == wifiUpdate)
  {
    return;
  }

  uint8_t key = packet->readByte();
  uint8_t value = packet->readByte();

  if (rebootTime != 0 && config.GetStartWiFiOnBoot() && key != MSP_ELRS_BACKPACK_CONFIG_TRAINER_MODE)
  {
    return;
  }

  switch (key)
  {
    case MSP_ELRS_BACKPACK_CONFIG_TLM_MODE:
    {
      const telem_mode_t previousTelemMode = config.GetTelemMode();
      const bool previousStartWiFiOnBoot = config.GetStartWiFiOnBoot();
      const wifi_service_t previousWiFiService = config.GetWiFiService();
      bool validTelemMode = true;
      switch (value)
      {
        case BACKPACK_TELEM_MODE_OFF:
          config.SetTelemMode(BACKPACK_TELEM_MODE_OFF);
          config.SetWiFiService(WIFI_SERVICE_UPDATE);
          config.SetStartWiFiOnBoot(false);
          break;
        case BACKPACK_TELEM_MODE_ESPNOW:
          config.SetTelemMode(BACKPACK_TELEM_MODE_ESPNOW);
          config.SetWiFiService(WIFI_SERVICE_UPDATE);
          config.SetStartWiFiOnBoot(false);
          break;
        case BACKPACK_TELEM_MODE_WIFI:
          config.SetTelemMode(BACKPACK_TELEM_MODE_WIFI);
          config.SetWiFiService(WIFI_SERVICE_MAVLINK_TX);
          config.SetStartWiFiOnBoot(true);
          break;
        default:
          validTelemMode = false;
          break;
      }
      if (validTelemMode)
      {
        const bool configChanged = previousTelemMode != (telem_mode_t)value ||
                                   previousStartWiFiOnBoot != config.GetStartWiFiOnBoot() ||
                                   previousWiFiService != config.GetWiFiService();
        config.Commit();
        if (configChanged)
        {
          rebootTime = millis() + 300;
        }
      }
      break;
    }
#if defined(TRAINER_BACKPACK_SUPPORTED)
    case MSP_ELRS_BACKPACK_CONFIG_TRAINER_MODE:
      trainerMode = value <= TRAINER_MODE_SLAVE ? (trainer_mode_t)value : TRAINER_MODE_OFF;
      config.SetTrainerMode(trainerMode);
      config.Commit();
      if (trainerMode == TRAINER_MODE_OFF)
      {
        trainerInPairing = false;
      }
      SendTrainerStatusToTX();
      break;
#endif
  }
}

#if defined(TRAINER_BACKPACK_SUPPORTED)
static void SendTrainerStatusToTX()
{
  const bool paired = config.IsTrainerPaired();

  mspPacket_t out;
  out.reset();
  out.makeResponse();
  out.function = MSP_ELRS_BACKPACK_TRAINER_STATUS;
  out.addByte(paired ? 1 : 0);
  out.addByte(trainerInPairing ? 1 : 0);
  out.addByte(paired
      ? config.GetTrainerPeerMac()[TRAINER_PEER_MAC_LAST_BYTE_INDEX]
      : 0);
  msp.sendPacket(&out, &Serial);
}

static void RebootIntoRuntime()
{
  trainerInPairing = false;
  config.SetStartWiFiOnBoot(false);
  config.SetWiFiService(WIFI_SERVICE_UPDATE);
  config.Commit();
  rebootTime = millis() + 300;
}
#endif

void ProcessMSPPacketFromTX(mspPacket_t *packet)
{
  if (connectionState == wifiUpdate)
  {
    if (packet->function == MSP_ELRS_GET_BACKPACK_VERSION)
    {
      SendVersionResponse();
    }
#if defined(TRAINER_BACKPACK_SUPPORTED)
    else if (packet->function == MSP_ELRS_BACKPACK_CONFIG)
    {
      if (packet->payloadSize >= 2 &&
          packet->payload[0] == MSP_ELRS_BACKPACK_CONFIG_TRAINER_MODE)
      {
        RebootIntoRuntime();
      }
    }
    else if (packet->function == MSP_ELRS_BACKPACK_TRAINER_PAIR_REQ ||
             packet->function == MSP_ELRS_BACKPACK_TRAINER_FORGET)
    {
      RebootIntoRuntime();
    }
#endif
    return;
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
#if defined(TRAINER_BACKPACK_SUPPORTED)
    SendTrainerStatusToTX();
    StopTrainerForWifi();
#endif
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
    if (config.GetTelemMode() == BACKPACK_TELEM_MODE_WIFI)
    {
      sendMSPViaWiFiUDP(packet);
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
    sendMSPViaEspnow(packet);
    break;

#if defined(TRAINER_BACKPACK_SUPPORTED)
  case MSP_ELRS_BACKPACK_TRAINER_CHANNELS:
    DBGLN("Processing TRAINER_CHANNELS...");
    sendTrainerChannelsViaEspNow(packet);
    break;

  case MSP_ELRS_BACKPACK_TRAINER_PAIR_REQ:
    DBGLN("Processing TRAINER_PAIR_REQ...");
    TrainerStartPairing();
    break;

  case MSP_ELRS_BACKPACK_TRAINER_FORGET:
    DBGLN("Processing TRAINER_FORGET...");
    TrainerForgetPeer();
    break;
#endif

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
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.begin("network-name", "pass-to-network", 1);
  WiFi.disconnect();
  #if defined(PLATFORM_ESP32)
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  #elif defined(PLATFORM_ESP8266)
    wifi_set_channel(1);
  #endif

  // Capture the HARDWARE MAC before overriding it with the UID.
  // Hardware MAC is unique per physical chip; UID MAC is shared by all devices
  // in the same binding group, which would cause RQSELF if used for trainer ID.
  #if defined(TRAINER_BACKPACK_SUPPORTED)
  CaptureTrainerLocalMac();
  #endif

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
  LOGGING_UART.begin(115200);
  LOGGING_UART.setDebugOutput(true);
#endif
  Serial.setRxBufferSize(4096);
  Serial.begin(460800);

  options_init();

  eeprom.Begin();
  config.SetStorageProvider(&eeprom);
  config.Load();
#if defined(TRAINER_BACKPACK_SUPPORTED)
  trainerMode = config.GetTrainerMode(); // restore persisted trainer mode (survives TLM_MODE reboots)
#endif

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
#if defined(TRAINER_BACKPACK_SUPPORTED)
    else
    {
      trainerEspNowReady = true;
    }
#endif

    #if defined(PLATFORM_ESP8266)
      esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      trainerAddPeer(firmwareOptions.uid);
      trainerAddPeer(trainerEspNowPairAddress());
    #elif defined(PLATFORM_ESP32)
      memcpy(peerInfo.peer_addr, firmwareOptions.uid, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      esp_err_t peerResult = esp_now_add_peer(&peerInfo);
      if (peerResult != ESP_OK && peerResult != ESP_ERR_ESPNOW_EXIST)
      {
        DBGLN("ESP-NOW failed to add peer");
      }
      memcpy(bindingInfo.peer_addr, bindingAddress, 6);
      bindingInfo.channel = 0;
      bindingInfo.encrypt = false;
      esp_err_t bindingResult = esp_now_add_peer(&bindingInfo);
      if (bindingResult != ESP_OK && bindingResult != ESP_ERR_ESPNOW_EXIST)
      {
        DBGLN("ESP-NOW failed to add binding peer");
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

  #if defined(TRAINER_BACKPACK_SUPPORTED)
    if (!trainerLocalMacReady && connectionState != wifiUpdate)
    {
      CaptureTrainerLocalMac();
    }
  #endif

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

  #if defined(TRAINER_BACKPACK_SUPPORTED)
  if (connectionState != wifiUpdate)
  {
    ProcessPendingTrainerPairPackets();
    FlushPendingTrainerChannels();
    TrainerPairingLoop(now);
    TrainerForgetLoop(now);
    if (now - lastTrainerStatusSendMs >= TRAINER_STATUS_PERIOD_MS)
    {
      lastTrainerStatusSendMs = now;
      SendTrainerStatusToTX();
    }
  }
  #endif
}
