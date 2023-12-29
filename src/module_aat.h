#pragma once

#if defined(AAT_BACKPACK)
#include "config.h"
#include "module_crsf.h"
#include "crsf_protocol.h"

#if defined(PIN_SERVO_AZIM) || defined(PIN_SERVO_ELEV)
#include <Servo.h>
#endif

#if defined(PIN_OLED_SDA)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#endif

class AatModule : public CrsfModuleBase
{
public:
    AatModule() = delete;
    AatModule(Stream &port);

    void Init();
    void Loop(uint32_t now);

    void SendGpsTelemetry(crsf_packet_gps_t *packet);
    bool isHomeSet() const { return _home.lat != 0 || _home.lon != 0; }
    bool isGpsActive() const { return _gpsLast.lastUpdateMs != 0; };
protected:
    virtual void onCrsfPacketIn(const crsf_header_t *pkt);
private:
    enum tagServoIndex { IDX_AZIM, IDX_ELEV, IDX_COUNT };
    void displayInit();
    void updateGpsInterval(uint32_t interval);
    uint8_t calcGpsIntervalPct(uint32_t now);
    int32_t calcProjectedAzim(uint32_t now);
    void processGps(uint32_t now);
    void servoUpdate(uint32_t now);
    void displayIdle(uint32_t now);
    void displayActive(uint32_t now, int32_t projectedAzim);
    void displayGpsIntervalBar(uint32_t now);

    struct {
        int32_t lat;
        int32_t lon;
        uint32_t speed;   // km/h * 10
        uint32_t heading; // degrees * 10
        int32_t altitude; // meters
        uint32_t lastUpdateMs; // timestamp of last update
        uint8_t satcnt;   // number of satellites
        bool updated;
    } _gpsLast;
    struct {
        int32_t lat;
        int32_t lon;
        int32_t alt;
    } _home;
    uint32_t _gpsAvgUpdateInterval; // ms * 100
    // Servo Position
    uint32_t _lastServoUpdateMs;
    uint32_t _targetDistance; // meters
    uint16_t _targetAzim; // degrees
    uint8_t _targetElev; // degrees
    int32_t _azimMsPerDegree; // milliseconds per degree
    int32_t _servoPos[IDX_COUNT]; // smoothed azim servo output us * 100

#if defined(PIN_SERVO_AZIM)
    Servo _servo_Azim;
#endif
#if defined(PIN_SERVO_ELEV)
    Servo _servo_Elev;
#endif
#if defined(PIN_OLED_SDA)
    Adafruit_SSD1306 _display;
#endif
};
#endif /* AAT_BACKPACK */