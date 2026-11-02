#include <Arduino.h>

#if defined(HAS_BLE_TRAINER)

#include <NimBLEDevice.h>
#include <esp_attr.h>
#include <esp_coexist.h>

#include "BleTrainer.h"
#include "common.h"
#include "config.h"
#include "device.h"
#include "msp.h"
#include "msptypes.h"

extern MSP msp;
extern bool headTrackingEnabled;
extern wifi_service_t wifiService;

// ------------------------------------------------------------------
//  FrSky Bluetooth trainer framing parser (same as BTWifiModule)
//    0x7E delimiter, 0x7D byte-stuff (XOR 0x20), frame 0x80 = 8 channels
//  The trainer frame is 0x80 + 12 packed channel bytes + XOR checksum. The
//  CRC and end delimiter are sent verbatim (not byte-stuffed); decode one
//  complete report per BLE notification and ignore the remaining padding.
// ------------------------------------------------------------------
#define FRSKY_CHANNELS 8
#define FRSKY_TRAINER_PACKET_SIZE 14
static constexpr uint16_t FRSKY_PWM_MIN_US = 988;
static constexpr uint16_t FRSKY_PWM_MAX_US = 2012;
static constexpr uint16_t FRSKY_CRSF_MIN_VALUE = 172;
static constexpr uint16_t FRSKY_CRSF_MAX_VALUE = 1811;

static volatile uint16_t frskyChannels[FRSKY_CHANNELS];
static volatile bool s_gotFrame = false;

static void frskyHandleFrame(const uint8_t *frame, uint8_t len)
{
    if (len < 2)
        return;

    uint8_t crc = 0;
    for (uint8_t i = 0; i < len - 1; i++)
        crc ^= frame[i];
    if (crc != frame[len - 1])
        return;

    if (len == FRSKY_TRAINER_PACKET_SIZE && frame[0] == 0x80)
    {
        for (uint8_t ch = 0, i = 1; ch < FRSKY_CHANNELS; ch += 2, i += 3)
        {
            frskyChannels[ch] = frame[i] + ((frame[i + 1] & 0xF0) << 4);
            frskyChannels[ch + 1] = ((frame[i + 1] & 0x0F) << 4) +
                                    ((frame[i + 2] & 0xF0) >> 4) +
                                    ((frame[i + 2] & 0x0F) << 8);
        }

        s_gotFrame = true;
    }
}

// Each BLE notification contains one complete FrSky trainer report followed by
// padding. Decode within that notification and ignore the padding rather than
// carrying parser state across notifications.
static void frskyProcessNotification(const uint8_t *data, size_t len)
{
    if (len < 2 || data[0] != 0x7E)
        return;

    uint8_t frame[FRSKY_TRAINER_PACKET_SIZE];
    size_t pos = 1;
    // The type and 12 packed channel bytes are byte-stuffed; the CRC is not.
    for (uint8_t i = 0; i < FRSKY_TRAINER_PACKET_SIZE - 1; i++)
    {
        if (pos >= len)
            return;

        uint8_t b = data[pos++];
        if (b == 0x7D)
        {
            if (pos >= len)
                return;
            b = data[pos++] ^ 0x20;
        }
        else if (b == 0x7E)
            return;
        frame[i] = b;
    }

    if (pos >= len)
        return;
    frame[FRSKY_TRAINER_PACKET_SIZE - 1] = data[pos++];
    if (pos >= len || data[pos] != 0x7E)
        return;

    frskyHandleFrame(frame, FRSKY_TRAINER_PACKET_SIZE);
}

// ------------------------------------------------------------------
//  BLE central (NimBLE)
// ------------------------------------------------------------------
// The trainer source never queues more than one notification: it gives its send
// semaphore back from the notification-complete callback, so its update rate is
// capped at one frame per connection event (its own thread runs at 80 Hz). It
// also asks for its own preferred parameters from its connection callback and
// accepts whatever the central asks for, so the interval requested here has to
// be small enough not to throttle the stream.
#define TRAINER_MIN_INTERVAL_MS 7
#define TRAINER_MAX_INTERVAL_MS 1000
// Long enough to ride out a transient RF blockage (a few hundred ms of missed
// connection events next to a 1 W RF module) instead of dropping the link, which
// would cost a reconnect and make the trainer report it as lost.
#define TRAINER_SUPERVISION_TIMEOUT 400
// Let the trainer's own parameter request land before ours.
#define TRAINER_CONN_PARAM_DELAY_MS 1200

static NimBLEScan *s_scan = nullptr;
static NimBLEClient *s_client = nullptr;
static volatile bool s_connected = false;
static volatile bool s_found = false;
static volatile bool s_forget = false;
static int s_failCount = 0;
static NimBLEAddress s_foundAddr;
static int s_bestRssi = -127;

// Retained-RAM diagnostic: the Nomad has no console, so the count of forwarded
// trainer packets survives reboots and is read from /config.
#define TRAINER_FX_MAGIC 0x48544658UL
RTC_NOINIT_ATTR static uint32_t s_setPtrMagic;
RTC_NOINIT_ATTR static uint32_t s_setPtrCount;

static void setPtrTick()
{
    if (s_setPtrMagic != TRAINER_FX_MAGIC)
    {
        s_setPtrMagic = TRAINER_FX_MAGIC;
        s_setPtrCount = 0;
    }
    s_setPtrCount++;
}

static void getRequestedConnParams(uint16_t &minU, uint16_t &maxU)
{
    uint16_t iv = config.GetTrainerIntervalMs();
    if (iv < TRAINER_MIN_INTERVAL_MS) iv = TRAINER_MIN_INTERVAL_MS;
    if (iv > TRAINER_MAX_INTERVAL_MS) iv = TRAINER_MAX_INTERVAL_MS;
    // min/max interval are in 1.25 ms units, supervision timeout in 10 ms units.
    minU = (uint16_t)(iv * 4 / 5);
    if (minU < 6) minU = 6;
    maxU = (uint16_t)((iv + 5) * 4 / 5);
    if (maxU < minU) maxU = minU;
}

static void setInitialConnParams()
{
    uint16_t minU, maxU;
    getRequestedConnParams(minU, maxU);
    // NimBLE otherwise starts at its defaults (20-50 ms, 2560 ms timeout).
    // Set these before connect so the first connection event uses the trainer rate.
    s_client->setConnectionParams(minU, maxU, 0, TRAINER_SUPERVISION_TIMEOUT);
}

static void applyConnParams()
{
    uint16_t minU, maxU;
    getRequestedConnParams(minU, maxU);
    s_client->updateConnParams(minU, maxU, 0, TRAINER_SUPERVISION_TIMEOUT);
}

static void onNotify(NimBLERemoteCharacteristic *, uint8_t *data, size_t len, bool)
{
    frskyProcessNotification(data, len);
}

class TrainerScanCb : public NimBLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice *dev) override
    {
        if (dev->isAdvertisingService(NimBLEUUID((uint16_t)0xFFF0)))
        {
            const int rssi = dev->getRSSI();
            if (rssi > s_bestRssi)
            {
                s_bestRssi = rssi;
                s_foundAddr = dev->getAddress();
            }
            s_found = true;
        }
    }
};

class TrainerClientCb : public NimBLEClientCallbacks
{
    void onDisconnect(NimBLEClient *) override
    {
        s_connected = false;
    }
};

static bool subscribeTrainer()
{
    NimBLERemoteService *svc = s_client->getService(NimBLEUUID((uint16_t)0xFFF0));
    if (!svc)
        return false;
    NimBLERemoteCharacteristic *ch = svc->getCharacteristic(NimBLEUUID((uint16_t)0xFFF6));
    if (!ch || !ch->canNotify())
        return false;
    ch->subscribe(true, onNotify);
    return true;
}

static void bleTask(void *)
{
    for (;;)
    {
        if (s_forget)
        {
            s_forget = false;
            config.ClearTrainerPeerMac();
            config.Commit();
            if (s_client->isConnected())
                s_client->disconnect();
            s_connected = false;
        }

        if (!s_connected)
        {
            bool ok = false;
            if (config.IsTrainerPaired())
            {
                NimBLEAddress addr(config.GetTrainerPeerMac(), config.GetTrainerPeerType());
                ok = s_client->connect(addr);
                if (ok)
                {
                    s_failCount = 0;
                }
                else if (++s_failCount >= 2)
                {
                    // Peer unreachable: drop it and go back to pairing mode.
                    s_failCount = 0;
                    config.ClearTrainerPeerMac();
                    config.Commit();
                }
            }
            else
            {
                // Only scan while there is no paired peer (pairing mode).
                s_found = false;
                s_bestRssi = -127;
                s_scan->start(3, false);
                if (s_found)
                {
                    ok = s_client->connect(s_foundAddr);
                    if (ok)
                    {
                        // Store the address in canonical (MSB-first) order: the
                        // NimBLEAddress(uint8_t[6]) constructor reverses it back.
                        uint8_t mac[6];
                        const uint8_t *raw = s_foundAddr.getNative();
                        for (int i = 0; i < 6; i++) mac[i] = raw[5 - i];
                        config.SetTrainerPeerMac(mac);
                        config.SetTrainerPeerType(s_foundAddr.getType());
                        config.Commit();
                    }
                }
            }

            if (ok && subscribeTrainer())
            {
                s_gotFrame = false;
                s_connected = true;
                // The trainer requests its own parameters from its connection
                // callback, so reapply ours after that request has landed.
                vTaskDelay(pdMS_TO_TICKS(TRAINER_CONN_PARAM_DELAY_MS));
                applyConnParams();
            }
            else
            {
                if (s_client->isConnected())
                    s_client->disconnect();
                vTaskDelay(pdMS_TO_TICKS(2000));
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

static void initBle()
{
    // When the backpack also runs the MAVLink WiFi AP, give the radio arbiter a
    // WiFi preference and use a lower BLE TX power: on the Nomad the module's PA
    // is next to the C3, and the added BLE airtime/power destabilized the AP.
    if (wifiService == WIFI_SERVICE_MAVLINK_TX)
    {
        esp_coex_preference_set(ESP_COEX_PREFER_WIFI);
    }

    NimBLEDevice::init("ELRS-BP");
    // Max TX power in normal use: the backpack sits next to the radio's RF module.
    // Reduce it while sharing the radio with the MAVLink WiFi AP.
    NimBLEDevice::setPower(wifiService == WIFI_SERVICE_MAVLINK_TX ? ESP_PWR_LVL_P3 : ESP_PWR_LVL_P9);
    s_scan = NimBLEDevice::getScan();
    s_scan->setAdvertisedDeviceCallbacks(new TrainerScanCb(), false);
    s_scan->setActiveScan(true);
    s_scan->setInterval(0x50);
    s_scan->setWindow(0x30);
    s_client = NimBLEDevice::createClient();
    setInitialConnParams();
    s_client->setClientCallbacks(new TrainerClientCb(), true);
    xTaskCreate(bleTask, "bletrainer", 4096, nullptr, 1, nullptr);
}

void bleTrainerForgetPeer()
{
    s_forget = true;
}

bool bleTrainerConnected()
{
    return s_connected;
}

uint32_t bleTrainerSetPtrCount()
{
    return s_setPtrMagic == TRAINER_FX_MAGIC ? s_setPtrCount : 0;
}

// ------------------------------------------------------------------
//  device_t
// ------------------------------------------------------------------
static void initialize()
{
}
static int start()
{
    // Keep BLE off for the firmware updater, but allow it alongside the MAVLink
    // WiFi service. That mode starts WiFi without initializing ESP-NOW.
    if (connectionState == wifiUpdate && wifiService != WIFI_SERVICE_MAVLINK_TX)
        return DURATION_NEVER;
    initBle();
    return DURATION_IMMEDIATELY;
}

static int event()
{
    return DURATION_IGNORE;
}

static int timeout()
{
    static uint32_t lastSend = 0;

    // Republish the latest channels at a steady rate so EdgeTX's trainer input
    // does not time out between (irregular) BLE notifications. The module only
    // forwards a CRSF channel frame per received SET_PTR packet.
    // The backpack restarts when MAVLink WiFi is selected, but the TX module
    // can retain its HT-enable state and therefore not resend the edge-triggered
    // enable MSP. In that service, forward BLE data and let the TX module enforce
    // its own enable/override state.
    const bool trainerOutputEnabled = headTrackingEnabled || wifiService == WIFI_SERVICE_MAVLINK_TX;
    if (config.GetTrainerSource() != TRAINER_SOURCE_ESPNOW && trainerOutputEnabled && s_connected &&
        s_gotFrame && (millis() - lastSend >= 20))
    {
        lastSend = millis();

        mspPacket_t packet;
        packet.reset();
        packet.makeCommand();
        packet.function = MSP_ELRS_BACKPACK_SET_PTR;
        for (int i = 0; i < FRSKY_CHANNELS; i++)
        {
            const uint16_t pwmUs = frskyChannels[i];
            uint16_t v;
            if (pwmUs <= FRSKY_PWM_MIN_US)
                v = FRSKY_CRSF_MIN_VALUE;
            else if (pwmUs >= FRSKY_PWM_MAX_US)
                v = FRSKY_CRSF_MAX_VALUE;
            else
            {
                const uint32_t spanUs = FRSKY_PWM_MAX_US - FRSKY_PWM_MIN_US;
                const uint32_t spanCrsf = FRSKY_CRSF_MAX_VALUE - FRSKY_CRSF_MIN_VALUE;
                v = FRSKY_CRSF_MIN_VALUE +
                    ((uint32_t)(pwmUs - FRSKY_PWM_MIN_US) * spanCrsf + spanUs / 2) / spanUs;
            }
            packet.addByte(v & 0xFF);
            packet.addByte(v >> 8);
        }
        msp.sendPacket(&packet, &Serial);
        setPtrTick();
    }
    return DURATION_IMMEDIATELY;
}

device_t BleTrainer_device = {
    .initialize = initialize,
    .start = start,
    .event = event,
    .timeout = timeout
};

#endif // HAS_BLE_TRAINER
