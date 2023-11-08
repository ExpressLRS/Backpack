#pragma once

#include "module_base.h"
#include "crsf_protocol.h"
#include <Servo.h>

class AatModule : public ModuleBase
{
public:
    void Init();
    void Loop(uint32_t now);

    void SendGpsTelemetry(crsf_packet_gps_t *packet);
private:
    // Minimum number of satellites to lock in the home position
    static constexpr uint8_t HOME_MIN_SATS = 5;
    void processGps();
    void servoUpdate(uint32_t now);

    bool _gpsUpdated;
    crsf_sensor_gps_t _gpsLast;
    int32_t _homeLat;
    int32_t _homeLon;
    // Servo Position
    Servo _servo_Azim;
    Servo _servo_Elev;
    uint32_t _lastServoUpdateMs;
    uint32_t _targetDistance; // meters
    uint8_t _targetElev; // degrees
    uint8_t _targetAzim; // degrees
    int32_t _currentElev; // degrees * 100
    int32_t _currentAzim; // degrees * 100
};
