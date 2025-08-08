#pragma once

#include "elrs_eeprom.h"

// Identifier for type of backpack
#define TX_BACKPACK_TYPE_ID     (0b01U)
#define VRX_BACKPACK_TYPE_ID    (0b10U)
#define TIMER_BACKPACK_TYPE_ID  (0b11U)

// CONFIG_MAGIC is ORed with CONFIG_VERSION in the version field
#define TX_BACKPACK_CONFIG_MAGIC    (TX_BACKPACK_TYPE_ID << 30)
#define VRX_BACKPACK_CONFIG_MAGIC   (VRX_BACKPACK_TYPE_ID << 30)
#define TIMER_BACKPACK_CONFIG_MAGIC (TIMER_BACKPACK_TYPE_ID << 30)

#define TX_BACKPACK_CONFIG_VERSION      4
#define VRX_BACKPACK_CONFIG_VERSION     5
#define TIMER_BACKPACK_CONFIG_VERSION   3


typedef enum {
    WIFI_SERVICE_UPDATE,
    WIFI_SERVICE_MAVLINK_TX,
} wifi_service_t;

typedef enum {
    BACKPACK_TELEM_MODE_OFF,
    BACKPACK_TELEM_MODE_ESPNOW,
    BACKPACK_TELEM_MODE_WIFI,
    BACKPACK_TELEM_MODE_BLUETOOTH,
} telem_mode_t;

#if defined(TARGET_TX_BACKPACK)

typedef struct {
    uint32_t          version;
    bool              startWiFi;
    char              ssid[33];
    char              password[65];
    uint8_t           address[6];
    wifi_service_t    wifiService;
    telem_mode_t      telemMode;
    uint16_t          mavlinkListenPort;
    uint16_t          mavlinkSendPort;
} tx_backpack_config_t;

class TxBackpackConfig
{
public:
    void Load();
    void Commit();

    // Getters
    bool     IsModified() const { return m_modified; }
    bool     GetStartWiFiOnBoot() { return m_config.startWiFi; }
    char    *GetSSID() { return m_config.ssid; }
    char    *GetPassword() { return m_config.password; }
    uint8_t *GetGroupAddress() { return m_config.address; }
    wifi_service_t GetWiFiService() { return m_config.wifiService; }
    telem_mode_t GetTelemMode() { return m_config.telemMode; }
    uint16_t GetMavlinkListenPort() const { return m_config.mavlinkListenPort; }
    uint16_t GetMavlinkSendPort() const { return m_config.mavlinkSendPort; }

    // Setters
    void SetStorageProvider(ELRS_EEPROM *eeprom);
    void SetDefaults();
    void SetStartWiFiOnBoot(bool startWifi);
    void SetSSID(const char *ssid);
    void SetPassword(const char *ssid);
    void SetGroupAddress(const uint8_t address[6]);
    void SetWiFiService(wifi_service_t service);
    void SetTelemMode(telem_mode_t mode);
    void SetMavlinkListenPort(uint16_t port);
    void SetMavlinkSendPort(uint16_t port);

private:
    tx_backpack_config_t    m_config;
    ELRS_EEPROM             *m_eeprom;
    bool                    m_modified;
};

extern TxBackpackConfig config;

#endif

///////////////////////////////////////////////////

#if defined(TARGET_VRX_BACKPACK)
typedef struct {
    uint32_t    version;
    uint8_t     bootCount;
    bool        startWiFi;
    char        ssid[33];
    char        password[65];
    uint8_t     address[6];
#if defined(HAS_HEADTRACKING)
    int         compassCalibration[3][2];
    float       imuCalibration[3];
    float       boardOrientation[3];
#endif
#if defined(AAT_BACKPACK)
    struct __attribute__((packed)) tagAatConfig {
        uint8_t     satelliteHomeMin;     // minimum number of satellites to establish home
        uint8_t     azimuthServoFastFlip; // 0=off, 1=on
        uint8_t     servoSmooth;          // 0-9 for min smoothing to most smoothing
        uint8_t     centerDir;            // Direction servo points at center position 0=N 2=E 4=S 6=W (can hold 45 degrees but only 90 is supported)
        uint8_t     project;              // FUTURE: 0=none, 1=projectAzim, 2=projectElev, 3=projectBoth
        uint8_t     units;                // FUTURE: 0=meters, anything else=also meters :-D
        uint8_t     servoMode;            // 0=2:1, 1=clip180, FUTURE: 180+flip servo
                                          // Also maybe invertAzim / invertElev servo bit or just swap low/high
        struct __attribute__((packed)) tagServoEndoint {
            uint16_t low;
            uint16_t high;
        } servoEndpoints[2];        // us endpoints for servos
    } aat;

    struct __attribute__((packed)) tagVbatConfig {
        uint16_t scale;
        int16_t offset;
    } vbat;
#endif
} vrx_backpack_config_t;

class VrxBackpackConfig
{
public:
    void Load();
    void Commit();

    // Getters
    bool     IsModified() const { return m_modified; }
    uint8_t  GetBootCount() { return m_config.bootCount; }
    bool     GetStartWiFiOnBoot() { return m_config.startWiFi; }
    char    *GetSSID() { return m_config.ssid; }
    char    *GetPassword() { return m_config.password; }
    uint8_t *GetGroupAddress() { return m_config.address; }

    // Setters
    void SetStorageProvider(ELRS_EEPROM *eeprom);
    void SetDefaults();
    void SetBootCount(uint8_t count);
    void SetStartWiFiOnBoot(bool startWifi);
    void SetSSID(const char *ssid);
    void SetPassword(const char *ssid);
    void SetGroupAddress(const uint8_t address[6]);

#if defined(HAS_HEADTRACKING)
    int     (*GetCompassCalibration())[3][2] { return &m_config.compassCalibration; }
    float   (*GetIMUCalibration())[3] { return &m_config.imuCalibration; }
    float   (*GetBoardOrientation())[3] { return &m_config.boardOrientation; }

    void SetCompassCalibration(const int calibration[3][2]);
    void SetIMUCalibration(const float calibration[3]);
    void SetBoardOrientation(const float orientation[3]);
#endif

#if defined(AAT_BACKPACK)
    uint8_t GetAatSatelliteHomeMin() const { return m_config.aat.satelliteHomeMin; }
    void SetAatSatelliteHomeMin(uint8_t val);
    uint8_t GetAatAzimuthServoFastFlip() const { return m_config.aat.azimuthServoFastFlip; }
    void SetAatAzimuthServoFastFlip(uint8_t val);
    uint8_t GetAatServoSmooth() const { return m_config.aat.servoSmooth; }
    void SetAatServoSmooth(uint8_t val);
    uint8_t GetAatServoMode() const { return m_config.aat.servoMode; }
    void SetAatServoMode(uint8_t val);
    uint8_t GetAatProject() const { return m_config.aat.project; }
    uint8_t GetAatCenterDir() const { return m_config.aat.centerDir; }
    void SetAatCenterDir(uint8_t val);
    uint16_t GetAatServoLow(uint8_t idx) const { return m_config.aat.servoEndpoints[idx].low; }
    void SetAatServoLow(uint8_t idx, uint16_t val);
    uint16_t GetAatServoHigh(uint8_t idx) const { return m_config.aat.servoEndpoints[idx].high; }
    void SetAatServoHigh(uint8_t idx, uint16_t val);
    bool GetAatServoEndpointsValid() const;

    uint16_t GetVbatScale() const { return m_config.vbat.scale; }
    void SetVbatScale(uint16_t val);
    int16_t GetVbatOffset() const { return m_config.vbat.offset; }
    void SetVbatOffset(int16_t val);
#endif

private:
    vrx_backpack_config_t   m_config;
    ELRS_EEPROM             *m_eeprom;
    bool                    m_modified;
};

extern VrxBackpackConfig config;

#endif

///////////////////////////////////////////////////

#if defined(TARGET_TIMER_BACKPACK)
typedef struct {
    uint32_t    version;
    uint8_t     bootCount;
    bool        startWiFi;
    char        ssid[33];
    char        password[65];
    uint8_t     address[6];
} timer_backpack_config_t;

class TimerBackpackConfig
{
public:
    void Load();
    void Commit();

    // Getters
    bool     IsModified() const { return m_modified; }
    uint8_t  GetBootCount() { return m_config.bootCount; }
    bool     GetStartWiFiOnBoot() { return m_config.startWiFi; }
    char    *GetSSID() { return m_config.ssid; }
    char    *GetPassword() { return m_config.password; }
    uint8_t *GetGroupAddress() { return m_config.address; }

    // Setters
    void SetStorageProvider(ELRS_EEPROM *eeprom);
    void SetDefaults();
    void SetBootCount(uint8_t count);
    void SetStartWiFiOnBoot(bool startWifi);
    void SetSSID(const char *ssid);
    void SetPassword(const char *ssid);
    void SetGroupAddress(const uint8_t address[6]);

private:
    timer_backpack_config_t m_config;
    ELRS_EEPROM             *m_eeprom;
    bool                    m_modified;
};

extern TimerBackpackConfig config;

#endif