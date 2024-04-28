#pragma once

#include "elrs_eeprom.h"

// CONFIG_MAGIC is ORed with CONFIG_VERSION in the version field
#define TX_BACKPACK_CONFIG_MAGIC    (0b01 << 30)
#define VRX_BACKPACK_CONFIG_MAGIC   (0b10 << 30)
#define TIMER_BACKPACK_CONFIG_MAGIC (0b11 << 30)

#define TX_BACKPACK_CONFIG_VERSION      3
#define VRX_BACKPACK_CONFIG_VERSION     3
#define TIMER_BACKPACK_CONFIG_VERSION   3

#if defined(TARGET_TX_BACKPACK)
typedef struct {
    uint32_t    version;
    bool        startWiFi;
    char        ssid[33];
    char        password[65];
    uint8_t     address[6];
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

    // Setters
    void SetStorageProvider(ELRS_EEPROM *eeprom);
    void SetDefaults();
    void SetStartWiFiOnBoot(bool startWifi);
    void SetSSID(const char *ssid);
    void SetPassword(const char *ssid);
    void SetGroupAddress(const uint8_t address[6]);

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