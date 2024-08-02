#include <Arduino.h>
#include "options.h"

#include <ArduinoJson.h>
#include <StreamString.h>
#include "EspFlashStream.h"
#if defined(PLATFORM_ESP8266)
#include <FS.h>
#else
#include <SPIFFS.h>
#endif
#if defined(PLATFORM_ESP32)
#include <esp_partition.h>
#include "esp_ota_ops.h"
#endif

#define QUOTE(arg) #arg
#define STR(macro) QUOTE(macro)
const unsigned char target_name[] = "\xBE\xEF\xCA\xFE" STR(TARGET_NAME);
const uint8_t target_name_size = sizeof(target_name);

const char PROGMEM compile_options[] = {
#ifdef MY_BINDING_PHRASE
    "-DMY_BINDING_PHRASE=\"" STR(MY_BINDING_PHRASE) "\" "
#endif
};

firmware_options_t firmwareOptions;

// Discriminator value used to determine if the device has been reflashed and therefore
// the SPIFSS settings are obsolete and the flashed settings should be used in preference
uint32_t flash_discriminator;

static StreamString builtinOptions;
String& getOptions()
{
    return builtinOptions;
}

void saveOptions(Stream &stream, bool customised)
{
    JsonDocument doc;

    if (firmwareOptions.hasUID)
    {
        JsonArray uid = doc.createNestedArray("uid");
        copyArray(firmwareOptions.uid, sizeof(firmwareOptions.uid), uid);
    }
    if (firmwareOptions.home_wifi_ssid[0])
    {
        doc["wifi-ssid"] = firmwareOptions.home_wifi_ssid;
        doc["wifi-password"] = firmwareOptions.home_wifi_password;
    }
    doc["product-name"] = firmwareOptions.product_name;
    doc["flash-discriminator"] = flash_discriminator;

    serializeJson(doc, stream);
}

void saveOptions()
{
    File options = SPIFFS.open("/options.json", "w");
    saveOptions(options, true);
    options.close();
}

/**
 * @brief:  Checks if the strmFlash currently is pointing to something that looks like
 *          a string (not all 0xFF). Position in the stream will not be changed.
 * @return: true if appears to have a string
 */
bool options_HasStringInFlash(EspFlashStream &strmFlash)
{
    uint32_t firstBytes;
    size_t pos = strmFlash.getPosition();
    strmFlash.readBytes((uint8_t *)&firstBytes, sizeof(firstBytes));
    strmFlash.setPosition(pos);

    return firstBytes != 0xffffffff;
}

/**
 * @brief:  Internal read options from either the flash stream at the end of the sketch or the options.json file
 *          Fills the firmwareOptions variable
 * @return: true if either was able to be parsed
 */
static void options_LoadFromFlashOrFile(EspFlashStream &strmFlash)
{
    JsonDocument flashDoc;
    JsonDocument spiffsDoc;
    bool hasFlash = false;
    bool hasSpiffs = false;

    strmFlash.setPosition(0);
    if (options_HasStringInFlash(strmFlash))
    {
        DeserializationError error = deserializeJson(flashDoc, strmFlash);
        if (error)
        {
            return;
        }
        hasFlash = true;
    }

    // load options.json from the SPIFFS partition
    File file = SPIFFS.open("/options.json", "r");
    if (file && !file.isDirectory())
    {
        DeserializationError error = deserializeJson(spiffsDoc, file);
        if (!error)
        {
            hasSpiffs = true;
        }
    }

    JsonDocument &doc = flashDoc;
    if (hasFlash && hasSpiffs)
    {
        if (flashDoc["flash-discriminator"] == spiffsDoc["flash-discriminator"])
        {
            doc = spiffsDoc;
        }
    }
    else if (hasSpiffs)
    {
        doc = spiffsDoc;
    }

    if (doc["uid"].is<JsonArray>())
    {
        copyArray(doc["uid"], firmwareOptions.uid, sizeof(firmwareOptions.uid));
        firmwareOptions.hasUID = true;
    }
    else
    {
        firmwareOptions.hasUID = false;
    }
    strlcpy(firmwareOptions.home_wifi_ssid, doc["wifi-ssid"] | "", sizeof(firmwareOptions.home_wifi_ssid));
    strlcpy(firmwareOptions.home_wifi_password, doc["wifi-password"] | "", sizeof(firmwareOptions.home_wifi_password));
    strlcpy(firmwareOptions.product_name, doc["product-name"] | "", sizeof(firmwareOptions.product_name));
    flash_discriminator = doc["flash-discriminator"] | 0U;

    builtinOptions.clear();
    saveOptions(builtinOptions, doc["customised"] | false);
}

bool options_init()
{
    uint32_t baseAddr = 0;
#if defined(PLATFORM_ESP32)
    SPIFFS.begin(true);
    const esp_partition_t *runningPart = esp_ota_get_running_partition();
    if (runningPart)
    {
        baseAddr = runningPart->address;
    }
#else
    SPIFFS.begin();
    // ESP8266 sketch baseAddr is always 0
#endif

    EspFlashStream strmFlash;
    strmFlash.setBaseAddress(baseAddr + ESP.getSketchSize());

    // options.json
    options_LoadFromFlashOrFile(strmFlash);

    // hardware.json
    // bool hasHardware = hardware_init(strmFlash);

    return true; //hasHardware;
}
