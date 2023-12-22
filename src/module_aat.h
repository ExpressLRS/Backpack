#pragma once

#include "module_base.h"
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

class AatModule : public ModuleBase
{
public:
    AatModule();

    void Init();
    void Loop(uint32_t now);

    void SendGpsTelemetry(crsf_packet_gps_t *packet);
    bool isHomeSet() const { return _homeLat !=0 && _homeLon != 0; }
private:
    // Minimum number of satellites to lock in the home position
    static constexpr uint8_t HOME_MIN_SATS = 5;

    void processGps();
    void servoUpdate(uint32_t now);

#if defined(PIN_SERVO_AZIM)
    Servo _servo_Azim;
#endif
#if defined(PIN_SERVO_ELEV)
    Servo _servo_Elev;
#endif
#if defined(PIN_OLED_SDA)
    Adafruit_SSD1306 display;
#endif

    bool _gpsUpdated;
    struct {
        int32_t lat;
        int32_t lon;
        uint32_t speed;   // km/h * 10
        uint32_t heading; // degrees * 10
        int32_t altitude; // meters
        uint8_t satcnt;   // number of satellites
    } _gpsLast;
    int32_t _homeLat;
    int32_t _homeLon;
    int32_t _homeAlt;
    // Servo Position
    uint32_t _lastServoUpdateMs;
    uint32_t _targetDistance; // meters
    uint16_t _targetAzim; // degrees
    uint8_t _targetElev; // degrees
    int32_t _currentElev; // degrees * 100
    int32_t _currentAzim; // degrees * 100
};
