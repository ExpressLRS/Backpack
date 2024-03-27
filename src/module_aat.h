#pragma once

#if defined(AAT_BACKPACK)
#include "config.h"
#include "module_crsf.h"
#include "crsf_protocol.h"
#include "devWIFI.h"

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

class VbatSampler
{
public:
    VbatSampler();

    void update(uint32_t now);
    // Reported in V * 10
    uint32_t value() const { return _value; }
private:
    const uint32_t VBAT_UPDATE_INTERVAL = 100U;
    const uint32_t VBAT_UPDATE_COUNT = 5U;

    uint32_t _lastUpdate;
    uint32_t _value;
    uint32_t _sum;  // in progress sum/count
    uint32_t _cnt;
};

class AatModule : public CrsfModuleBase
{
public:
    AatModule() = delete;
    AatModule(Stream &port);

    void Init();
    void Loop(uint32_t now);

    void SendGpsTelemetry(crsf_packet_gps_t *packet);
    bool isHomeSet() const { return _home.lat != 0 || _home.lon != 0 || _isOverrideMode; }
    bool isGpsActive() const { return _gpsLast.lastUpdateMs != 0; };

    void overrideTargetBearing(int32_t bearing);
    void overrideTargetElev(int32_t elev);
    uint32_t getVbat();
protected:
    void overrideTargetCommon(int32_t azimuth, int32_t elevation);
    virtual void onCrsfPacketIn(const crsf_header_t *pkt);
private:
    enum ServoIndex { IDX_AZIM, IDX_ELEV, IDX_COUNT };
    enum ServoMode { TwoToOne, Clip180, Flip180 };

    void displayInit();
    void updateGpsInterval(uint32_t interval);
    uint8_t calcGpsIntervalPct(uint32_t now);
    int32_t calcProjectedAzim(uint32_t now);
    void servoApplyMode(int32_t azim, int32_t elev, int32_t newServoPos[]);
    void processGps(uint32_t now);
    void servoUpdate(uint32_t now);
    const int32_t azimToBearing(int32_t azim) const;

#if defined(PIN_OLED_SDA)
    void displayState();
    void displayGpsIdle(uint32_t now);
    void displayActive(uint32_t now, int32_t projectedAzim);
    void displayGpsIntervalBar(uint32_t now);
    void displayAzimuthExtent(int32_t y);
    void displayAzimuth(int32_t projectedAzim);
    void displayAltitude(int32_t azimPos, int32_t elevPos);
    void displayTargetCircle(int32_t projectedAzim);
    void displayTargetDistance();
    void displayVBat();
#endif

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
    uint32_t _gpsAvgUpdateIntervalMs;
    // Servo Position
    uint32_t _lastServoUpdateMs;
    uint32_t _targetDistance; // meters
    uint16_t _targetAzim; // degrees
    uint8_t _targetElev; // degrees
    bool    _isOverrideMode;
    int32_t _azimMsPerDegree; // milliseconds per degree
    int32_t _servoPos[IDX_COUNT]; // smoothed azim servo output us
    uint32_t _lastAzimFlipMs;
    VbatSampler _vbat;

#if defined(PIN_SERVO_AZIM)
    Servo _servo_Azim;
#endif
#if defined(PIN_SERVO_ELEV)
    Servo _servo_Elev;
#endif
#if defined(PIN_OLED_SDA)
    Adafruit_SSD1306 _display;
    uint32_t _lastDisplayActiveMs;
#endif
};

extern AatModule vrxModule;

#endif /* AAT_BACKPACK */