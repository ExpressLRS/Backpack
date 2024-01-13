#if defined(AAT_BACKPACK)

#include "devwifi_proxies.h"
#include "module_aat.h"

void WebAatAppendConfig(ArduinoJson::JsonDocument &json)
{
    auto aat = json["config"].createNestedObject("aat");
    aat["satmin"] = config.GetAatSatelliteHomeMin();
    aat["servosmoo"] = config.GetAatServoSmooth();
    aat["servomode"] = config.GetAatServoMode();
    aat["project"] = config.GetAatProject();
    aat["azim_center"] = config.GetAatCenterDir();
    aat["azim_min"] = config.GetAatServoLow(0);
    aat["azim_max"] = config.GetAatServoHigh(0);
    aat["elev_min"] = config.GetAatServoLow(1);
    aat["elev_max"] = config.GetAatServoHigh(1);

    // VBat
    auto vbat = json["config"].createNestedObject("vbat");
    vbat["offset"] = config.GetVbatOffset();
    vbat["scale"] = config.GetVbatScale();
    vbat["vbat"] = vrxModule.getVbat();
}

void WebAatConfig(AsyncWebServerRequest *request)
{
    // Servos
    if (request->hasArg("servosmoo"))
        config.SetAatServoSmooth(request->arg("servosmoo").toInt());
    if (request->hasArg("servomode"))
        config.SetAatServoMode(request->arg("servomode").toInt());
    if (request->hasArg("azim_center"))
        config.SetAatCenterDir(request->arg("azim_center").toInt());
    if (request->hasArg("azim_min"))
        config.SetAatServoLow(0, request->arg("azim_min").toInt());
    if (request->hasArg("azim_max"))
        config.SetAatServoHigh(0, request->arg("azim_max").toInt());
    if (request->hasArg("elev_min"))
        config.SetAatServoLow(1, request->arg("elev_min").toInt());
    if (request->hasArg("elev_max"))
        config.SetAatServoHigh(1, request->arg("elev_max").toInt());
    // VBat
    if (request->hasArg("vbat_offset"))
        config.SetVbatOffset(request->arg("vbat_offset").toInt());
    if (request->hasArg("vbat_scale"))
        config.SetVbatScale(request->arg("vbat_scale").toInt());
    // Servo Target Override (must be set after azim_center because bearing relies on center)
    if (request->hasArg("bear"))
        vrxModule.overrideTargetBearing(request->arg("bear").toInt());
    if (request->hasArg("elev"))
        vrxModule.overrideTargetElev(request->arg("elev").toInt());

    const char *response;
    if (request->arg("commit").toInt() == 1)
    {
        response = "Saved";
        config.Commit();
    }
    else
        response = "Modified";

    request->send(200, "text/plain", response);
}

void WebAatInit(AsyncWebServer &server)
{
    server.on("/aatconfig", WebAatConfig);
}

#endif /* defined(AAT_BACKPACK) */

