#pragma once

#if defined(AAT_BACKPACK)

#include <cstdint>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

void WebAatAppendConfig(ArduinoJson::JsonDocument &json);
void WebAatInit(AsyncWebServer &server);

#endif /* defined(AAT_BACKPACK) */
