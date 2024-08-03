#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "config.h"

#if defined(PIN_BUTTON)
#include "logging.h"
#include "button.h"

static Button<PIN_BUTTON, false> button;

extern unsigned long rebootTime;
void RebootIntoWifi(wifi_service_t service);

static void shortPress()
{
    if (connectionState == wifiUpdate)
    {
        rebootTime = millis();
    }
    else
    {
        RebootIntoWifi(WIFI_SERVICE_UPDATE);
    }
}

static void initialize()
{
    button.OnShortPress = shortPress;
}

static int start()
{
    return DURATION_IMMEDIATELY;
}

static int timeout()
{
    return button.update();
}

device_t Button_device = {
    .initialize = initialize,
    .start = start,
    .event = NULL,
    .timeout = timeout
};

#endif