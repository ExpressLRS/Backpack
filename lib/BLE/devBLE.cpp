#if defined(PLATFORM_ESP32) && defined(BLE_TELEM_ENABLED)

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "devBLE.h"
#include "common.h"
#include "config.h"
#include "logging.h"
#include "options.h"

extern TxBackpackConfig config;

// HM-10 compatible telemetry profile.
static const uint16_t TELEMETRY_SVC_UUID  = 0xFFE0;
static const uint16_t TELEMETRY_CRSF_UUID = 0xFFE1;

// Standard GATT Device Information Service.
static const uint16_t DEVICE_INFO_SVC_UUID       = 0x180A;
static const uint16_t MANUFACTURER_NAME_SVC_UUID = 0x2A29;
static const uint16_t MODEL_NUMBER_SVC_UUID      = 0x2A24;
static const uint16_t SERIAL_NUMBER_SVC_UUID     = 0x2A25;
static const uint16_t SOFTWARE_NUMBER_SVC_UUID   = 0x2A28;
static const uint16_t HARDWARE_NUMBER_SVC_UUID   = 0x2A27;

static constexpr uint16_t CRSF_BLE_MAX_PACKET_LEN = 64; // matches MSP_PORT_INBUF_SIZE

static NimBLEServer         *pServer = nullptr;
static NimBLECharacteristic *rcCRSF  = nullptr;

// Single-slot buffer; producer = MSP loop, consumer = timeout(), both on Arduino task.
static uint8_t           pendingFrame[CRSF_BLE_MAX_PACKET_LEN] = {0};
static volatile uint16_t pendingFrameLen = 0;

static bool     justConnected = false;
static uint32_t lastHeartbeat = 0;

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
    {
        justConnected = true;
        devicesTriggerEvent();
        DBGLN("BLE client connected");
    }

    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
    {
        DBGLN("BLE client disconnected - restarting advertising");
        NimBLEDevice::startAdvertising();
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo &connInfo) override
    {
        DBGLN("BLE MTU updated: %u", MTU);
    }
};
static ServerCallbacks serverCB;

static void bleStart()
{
    if (pServer != nullptr)
    {
        return;
    }

    // Last 3 UID bytes in device name so multiple backpacks are distinguishable in a scanner.
    char devName[24];
    snprintf(devName, sizeof(devName), "ELRS Backpack %02X%02X%02X",
             firmwareOptions.uid[3], firmwareOptions.uid[4], firmwareOptions.uid[5]);

    NimBLEDevice::init(devName);
    NimBLEDevice::setMTU(512);

    NimBLEServer *server = NimBLEDevice::createServer();
    server->setCallbacks(&serverCB);

    NimBLEService *rcService = server->createService(TELEMETRY_SVC_UUID);
    rcCRSF = rcService->createCharacteristic(
        TELEMETRY_CRSF_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY,
        CRSF_BLE_MAX_PACKET_LEN);

    char serial[18];
    snprintf(serial, sizeof(serial), "%02X:%02X:%02X:%02X:%02X:%02X",
             firmwareOptions.uid[0], firmwareOptions.uid[1], firmwareOptions.uid[2],
             firmwareOptions.uid[3], firmwareOptions.uid[4], firmwareOptions.uid[5]);

    const char *model = firmwareOptions.product_name[0] != '\0'
                        ? firmwareOptions.product_name
                        : "ELRS Backpack";

    NimBLEService *disService = server->createService(DEVICE_INFO_SVC_UUID);
    disService->createCharacteristic(MANUFACTURER_NAME_SVC_UUID, NIMBLE_PROPERTY::READ)
        ->setValue("ExpressLRS");
    disService->createCharacteristic(MODEL_NUMBER_SVC_UUID, NIMBLE_PROPERTY::READ)
        ->setValue(model);
    disService->createCharacteristic(SERIAL_NUMBER_SVC_UUID, NIMBLE_PROPERTY::READ)
        ->setValue(serial);
    disService->createCharacteristic(SOFTWARE_NUMBER_SVC_UUID, NIMBLE_PROPERTY::READ)
        ->setValue("ExpressLRS Backpack");
    disService->createCharacteristic(HARDWARE_NUMBER_SVC_UUID, NIMBLE_PROPERTY::READ)
        ->setValue("1.0");

    server->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(rcService->getUUID());
    pAdvertising->enableScanResponse(true);
    pAdvertising->setName(devName);
    pAdvertising->start();

    // Publish pServer last — used as the "BLE ready" gate by SendTxBackpackTelemetryViaBLE().
    pServer = server;

    DBGLN("BLE telemetry started as \"%s\"", devName);
}

bool SendTxBackpackTelemetryViaBLE(const uint8_t *data, uint16_t size)
{
    if (pServer == nullptr || data == nullptr)
    {
        return false;
    }
    if (size == 0 || size > CRSF_BLE_MAX_PACKET_LEN)
    {
        return false;
    }
    if (pServer->getConnectedCount() == 0)
    {
        return false;
    }
    memcpy(pendingFrame, data, size);
    pendingFrameLen = size;
    devicesTriggerEvent();
    return true;
}

static void initialize()
{
    pendingFrameLen = 0;
    justConnected   = false;
}

static int start()
{
    // Skip BLE during WiFi update mode — AP + AsyncTCP + BLE on one 2.4GHz radio is unstable.
    if (config.GetTelemMode() == BACKPACK_TELEM_MODE_BLUETOOTH && connectionState != wifiUpdate)
    {
        bleStart();
    }
    return DURATION_NEVER;
}

static int event()
{
    return DURATION_IMMEDIATELY;
}

static int timeout()
{
    if (pServer == nullptr || rcCRSF == nullptr)
    {
        return DURATION_NEVER;
    }

    if (pServer->getConnectedCount() == 0)
    {
        return DURATION_NEVER;
    }

    // First-connect test frame — lets the client confirm notify is wired up.
    if (justConnected)
    {
        const uint8_t testFrame[] = {0xBE, 0xEF};
        rcCRSF->setValue(testFrame, sizeof(testFrame));
        rcCRSF->notify();
        justConnected = false;
        lastHeartbeat = millis();
        return 2000;
    }

    if (pendingFrameLen > 0)
    {
        uint16_t offset = 0;
        while (offset < pendingFrameLen)
        {
            uint16_t chunk = pendingFrameLen - offset;
            if (chunk > 20) chunk = 20;
            rcCRSF->setValue(pendingFrame + offset, chunk);
            rcCRSF->notify();
            offset += chunk;
        }
        pendingFrameLen = 0;
        lastHeartbeat   = millis();
        return DURATION_NEVER;
    }

    // 2s heartbeat keeps the link alive when no CRSF data is flowing.
    if (millis() - lastHeartbeat >= 2000)
    {
        const uint8_t heartbeat[] = {0xBE, 0xEF};
        rcCRSF->setValue(heartbeat, sizeof(heartbeat));
        rcCRSF->notify();
        lastHeartbeat = millis();
    }
    return 2000;
}

device_t BLE_device = {
    .initialize = initialize,
    .start      = start,
    .event      = event,
    .timeout    = timeout,
};

#endif
