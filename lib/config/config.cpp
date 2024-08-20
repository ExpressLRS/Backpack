#include "config.h"
#include "logging.h"

#if defined(TARGET_TX_BACKPACK)

void
TxBackpackConfig::Load()
{
    m_eeprom->Get(0, m_config);
    m_modified = 0;

    // Check if version number matches
    if (m_config.version != (uint32_t)(TX_BACKPACK_CONFIG_VERSION | TX_BACKPACK_CONFIG_MAGIC))
    {
        // If not, revert to defaults for this version
        DBGLN("EEPROM version mismatch! Resetting to defaults...");
        SetDefaults();
    }
}

void
TxBackpackConfig::Commit()
{
    if (!m_modified)
    {
        // No changes
        return;
    }
    // Write the struct to eeprom
    m_eeprom->Put(0, m_config);
    m_eeprom->Commit();

    m_modified = false;
}

// Setters
void
TxBackpackConfig::SetStorageProvider(ELRS_EEPROM *eeprom)
{
    if (eeprom)
    {
        m_eeprom = eeprom;
    }
}

void
TxBackpackConfig::SetDefaults()
{
    m_config.version = TX_BACKPACK_CONFIG_VERSION | TX_BACKPACK_CONFIG_MAGIC;
    m_config.startWiFi = false;
    m_config.ssid[0] = 0;
    m_config.password[0] = 0;
    memset(m_config.address, 0, 6);
    m_config.wifiService = WIFI_SERVICE_UPDATE;
    m_modified = true;
    Commit();
}

void
TxBackpackConfig::SetStartWiFiOnBoot(bool startWifi)
{
    m_config.startWiFi = startWifi;
    m_modified = true;
}

void
TxBackpackConfig::SetSSID(const char *ssid)
{
    strcpy(m_config.ssid, ssid);
    m_modified = true;
}

void
TxBackpackConfig::SetPassword(const char *password)
{
    strcpy(m_config.password, password);
    m_modified = true;
}

void
TxBackpackConfig::SetGroupAddress(const uint8_t address[6])
{
    memcpy(m_config.address, address, 6);
    m_modified = true;
}

void
TxBackpackConfig::SetWiFiService(wifi_service_t service)
{
    m_config.wifiService = service;
    m_modified = true;
}

void
TxBackpackConfig::SetTelemMode(telem_mode_t mode)
{
    m_config.telemMode = mode;
    m_modified = true;
}
#endif

/////////////////////////////////////////////////////

#if defined(TARGET_VRX_BACKPACK)

#define CONFIG_MOD_CHECK(fld, val) {    \
        if (val == fld)                 \
            return;                     \
        fld = val;                      \
        m_modified = true;              \
    }

void
VrxBackpackConfig::Load()
{
    // Populate the struct from eeprom
    m_eeprom->Get(0, m_config);

    // Check if version number matches
    if (m_config.version != (uint32_t)(VRX_BACKPACK_CONFIG_VERSION | VRX_BACKPACK_CONFIG_MAGIC))
    {
        // If not, revert to defaults for this version
        DBGLN("EEPROM version mismatch! Resetting to defaults...");
        SetDefaults();
    }

    m_modified = false;
}

void
VrxBackpackConfig::Commit()
{
    if (!m_modified)
    {
        // No changes
        return;
    }

    // Write the struct to eeprom
    m_eeprom->Put(0, m_config);
    m_eeprom->Commit();

    m_modified = false;
}

// Setters
void
VrxBackpackConfig::SetStorageProvider(ELRS_EEPROM *eeprom)
{
    if (eeprom)
    {
        m_eeprom = eeprom;
    }
}

void
VrxBackpackConfig::SetDefaults()
{
    memset(&m_config, 0, sizeof(m_config));
    m_config.version = VRX_BACKPACK_CONFIG_VERSION | VRX_BACKPACK_CONFIG_MAGIC;

#if defined(AAT_BACKPACK)
    m_config.aat.satelliteHomeMin = 5;
    m_config.aat.project = 0xff;
    m_config.aat.servoSmooth = 5;
    m_config.aat.centerDir = 0; // N
    m_config.aat.servoEndpoints[0].low = 500; // AZIM
    m_config.aat.servoEndpoints[0].high = 2500;
    m_config.aat.servoEndpoints[1].low = 1000; // ELEV
    m_config.aat.servoEndpoints[1].high = 2000;

    m_config.vbat.scale = 292;
    m_config.vbat.offset = -2;
#endif

    m_modified = true;
    Commit();
}

void
VrxBackpackConfig::SetSSID(const char *ssid)
{
    strcpy(m_config.ssid, ssid);
    m_modified = true;
}

void
VrxBackpackConfig::SetPassword(const char *password)
{
    strcpy(m_config.password, password);
    m_modified = true;
}

void
VrxBackpackConfig::SetGroupAddress(const uint8_t address[6])
{
    memcpy(m_config.address, address, 6);
    m_modified = true;
}

void
VrxBackpackConfig::SetBootCount(uint8_t count)
{
    m_config.bootCount = count;
    m_modified = true;
}

void
VrxBackpackConfig::SetStartWiFiOnBoot(bool startWifi)
{
    m_config.startWiFi = startWifi;
    m_modified = true;
}

#if defined(AAT_BACKPACK)

void
VrxBackpackConfig::SetAatServoSmooth(uint8_t val)
{
    CONFIG_MOD_CHECK(m_config.aat.servoSmooth, val);
}

void
VrxBackpackConfig::SetAatServoLow(uint8_t idx, uint16_t val)
{
    CONFIG_MOD_CHECK(m_config.aat.servoEndpoints[idx].low, val);
}

void
VrxBackpackConfig::SetAatServoHigh(uint8_t idx, uint16_t val)
{
    CONFIG_MOD_CHECK(m_config.aat.servoEndpoints[idx].high, val);
}

void
VrxBackpackConfig::SetVbatScale(uint16_t val)
{
    CONFIG_MOD_CHECK(m_config.vbat.scale, val);
}

void
VrxBackpackConfig::SetVbatOffset(int16_t val)
{
    CONFIG_MOD_CHECK(m_config.vbat.offset, val);
}

void
VrxBackpackConfig::SetAatCenterDir(uint8_t val)
{
    CONFIG_MOD_CHECK(m_config.aat.centerDir, val);
}

void
VrxBackpackConfig::SetAatServoMode(uint8_t val)
{
    CONFIG_MOD_CHECK(m_config.aat.servoMode, val);
}

/**
 * @brief: Validate that the endpoints have a valid range, i.e. low/high not the same
*/
bool
VrxBackpackConfig::GetAatServoEndpointsValid() const
{
    return (m_config.aat.servoEndpoints[0].low != m_config.aat.servoEndpoints[0].high)
        && (m_config.aat.servoEndpoints[1].low != m_config.aat.servoEndpoints[1].high);
}

#endif /* defined(AAT_BACKPACK) */

#endif /* TARGET_VRX_BACKPACK */

/////////////////////////////////////////////////////

#if defined(TARGET_TIMER_BACKPACK)

void
TimerBackpackConfig::Load()
{
    m_eeprom->Get(0, m_config);
    m_modified = 0;

    // Check if version number matches
    if (m_config.version != (uint32_t)(TIMER_BACKPACK_CONFIG_VERSION | TIMER_BACKPACK_CONFIG_MAGIC))
    {
        // If not, revert to defaults for this version
        DBGLN("EEPROM version mismatch! Resetting to defaults...");
        SetDefaults();
    }

    m_modified = false;
}

void
TimerBackpackConfig::Commit()
{
    if (!m_modified)
    {
        // No changes
        return;
    }
    // Write the struct to eeprom
    m_eeprom->Put(0, m_config);
    m_eeprom->Commit();

    m_modified = false;
}

// Setters
void
TimerBackpackConfig::SetStorageProvider(ELRS_EEPROM *eeprom)
{
    if (eeprom)
    {
        m_eeprom = eeprom;
    }
}

void
TimerBackpackConfig::SetDefaults()
{
    m_config.version = TIMER_BACKPACK_CONFIG_VERSION | TIMER_BACKPACK_CONFIG_MAGIC;
    m_config.bootCount = 0;
    m_config.startWiFi = false;
    m_config.ssid[0] = 0;
    m_config.password[0] = 0;
    memset(m_config.address, 0, 6);
    m_modified = true;
    Commit();
}

void
TimerBackpackConfig::SetStartWiFiOnBoot(bool startWifi)
{
    m_config.startWiFi = startWifi;
    m_modified = true;
}

void
TimerBackpackConfig::SetSSID(const char *ssid)
{
    strcpy(m_config.ssid, ssid);
    m_modified = true;
}

void
TimerBackpackConfig::SetPassword(const char *password)
{
    strcpy(m_config.password, password);
    m_modified = true;
}

void
TimerBackpackConfig::SetGroupAddress(const uint8_t address[6])
{
    memcpy(m_config.address, address, 6);
    m_modified = true;
}

void
TimerBackpackConfig::SetBootCount(uint8_t count)
{
    m_config.bootCount = count;
    m_modified = true;
}

#endif
