#if defined(AAT_BACKPACK)
#include "common.h"
#include "module_aat.h"
#include "logging.h"
#include <math.h>
#include <Arduino.h>

#define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#define RAD2DEG(rad) ((rad) * 180.0 / M_PI)

AatModule::AatModule() :
#if defined(PIN_SERVO_AZIM)
    _servo_Azim(),
#endif
#if defined(PIN_SERVO_ELEV)
    _servo_Elev(),
#endif
#if defined(PIN_OLED_SDA)
    display(SCREEN_WIDTH, SCREEN_HEIGHT),
#endif
#if defined (AAT_SERIAL_INPUT)
    _crc(CRSF_CRC_POLY)
#endif
{
  // Init is called manually
}

void AatModule::Init()
{
#if defined(PIN_SERVO_AZIM)
    _servo_Azim.attach(PIN_SERVO_AZIM, 500, 2500);
#endif
#if defined(PIN_SERVO_ELEV)
    _servo_Elev.attach(PIN_SERVO_ELEV, 500, 2500);
#endif
#if defined(PIN_OLED_SDA)
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    if (connectionState == binding)
        display.print("Bind\nmode...");
    else
        display.print("AAT\nBackpack");
    display.display();
#endif
    ModuleBase::Init();
}

void AatModule::SendGpsTelemetry(crsf_packet_gps_t *packet)
{
    _gpsLast.lat = be32toh(packet->p.lat);
    _gpsLast.lon = be32toh(packet->p.lon);
    _gpsLast.speed = be16toh(packet->p.speed);
    _gpsLast.heading = be16toh(packet->p.heading);
    _gpsLast.altitude = (int32_t)be16toh(packet->p.altitude) - 1000;
    _gpsLast.satcnt = packet->p.satcnt;

    DBGLN("GPS: (%d,%d) %dm %u sats", _gpsLast.lat, _gpsLast.lon,
        _gpsLast.altitude, _gpsLast.satcnt);

    _gpsUpdated = true;
}

void calcDistAndAzimuth(int32_t srcLat, int32_t srcLon, int32_t dstLat, int32_t dstLon,
    uint32_t *out_dist, uint32_t *out_azimuth)
{
    // https://www.movable-type.co.uk/scripts/latlong.html
    // https://www.igismap.com/formula-to-find-bearing-or-heading-angle-between-two-points-latitude-longitude/

    // Have to use doubles for at least some of these, due to short distances getting rounded
    // particularly cos(deltaLon) for <2000 m rounds to 1.0000000000
    double deltaLon = DEG2RAD((float)(dstLon - srcLon) / 1e7);
    double thetaA = DEG2RAD((float)srcLat / 1e7);
    double thetaB = DEG2RAD((float)dstLat / 1e7);
    double cosThetaA = cos(thetaA);
    double cosThetaB = cos(thetaB);
    double sinThetaA = sin(thetaA);
    double sinThetaB = sin(thetaB);
    double cosDeltaLon = cos(deltaLon);
    double sinDeltaLon = sin(deltaLon);

    if (out_dist)
    {
        const double R = 6371e3;
        double dist = acos(sinThetaA * sinThetaB + cosThetaA * cosThetaB * cosDeltaLon) * R;
        *out_dist = (uint32_t)dist;
    }

    if (out_azimuth)
    {
        double X = cosThetaB * sinDeltaLon;
        double Y = cosThetaA * sinThetaB - sinThetaA * cosThetaB * cosDeltaLon;

        // Convert to degrees, normalized to 0-360
        uint32_t hdg = RAD2DEG(atan2(X, Y));
        *out_azimuth = (hdg + 360) % 360;
    }
}

static int32_t calcElevation(uint32_t distance, int32_t altitude)
{
    return RAD2DEG(atan2(altitude, distance));
}

void AatModule::processGps()
{
    // Actually want to track time between _processing_ each GPS update
    uint32_t now = millis();
    uint32_t interval = _gpsLast.lastUpdateMs - now;
    _gpsLast.lastUpdateMs = now;

    // Check if need to set home position
    bool didSetHome = false;
    if (!isHomeSet())
    {
        if (_gpsLast.satcnt >= HOME_MIN_SATS)
        {
            didSetHome = true;
            _homeLat = _gpsLast.lat;
            _homeLon = _gpsLast.lon;
            _homeAlt = _gpsLast.altitude;
            DBGLN("GPS Home set to (%d,%d)", _homeLat, _homeLon);
        }
        else
            return;
    }

    uint32_t azimuth;
    uint32_t distance;
    calcDistAndAzimuth(_homeLat, _homeLon, _gpsLast.lat, _gpsLast.lon, &distance, &azimuth);
    uint8_t elevation = constrain(calcElevation(distance, _gpsLast.altitude - _homeAlt), 0, 90);
    DBGLN("Azimuth: %udeg Elevation: %udeg Distance: %um", azimuth, elevation, distance);

    // Calculate angular velocity to allow dead reckoning projection
    if (!didSetHome)
    {
        int32_t azimDelta = azimuth - _targetAzim;
        int32_t azimMsPerDelta = interval / azimDelta;
        DBGLN("%d delta in %ums, %dms/d", azimDelta, interval, azimMsPerDelta);
    }

    _targetDistance = distance;
    _targetElev = elevation;
    _targetAzim = azimuth;
}

void AatModule::servoUpdate(uint32_t now)
{
    if (!isHomeSet())
        return;
    if (now - _lastServoUpdateMs < 20U)
        return;
    _lastServoUpdateMs = now;

    // Smooth the updates and scale to degrees * 100
    _currentAzim = ((_currentAzim * 9) + (_targetAzim * 100)) / 10;
    _currentElev = ((_currentElev * 9) + (_targetElev * 100)) / 10;

    int32_t projectedAzim = _currentAzim;
    int32_t projectedElev = _currentElev;

    // 90-270 azim inverts the elevation servo
    // if (projectedAzim > 18000)
    // {
    //     projectedElev = 18000 - projectedElev;
    //     projectedAzim = projectedAzim - 18000;
    // }
    // For straight geared servos with 180 degree elev flip
    // int32_t usAzim = map(projectedAzim, 0, 18000, 500, 2500);
    // int32_t usElev = map(projectedElev, 0, 18000, 500, 2500);

    // For 1:2 gearing on the azim servo to allow 360 rotation
    // For Elev servos that only go 0-90 and the azim does 360
    projectedAzim = (projectedAzim + 18000) % 36000; // convert so 0 maps to 1500us
    int32_t usAzim = map(projectedAzim, 0, 36000, 700, 2400);
    int32_t usElev = map(projectedElev, 0, 9000, 1100, 1900);

#if defined(PIN_SERVO_AZIM)
    _servo_Azim.writeMicroseconds(usAzim);
#endif
#if defined(PIN_SERVO_ELEV)
    _servo_Elev.writeMicroseconds(usElev);
#endif

#if defined(PIN_OLED_SDA)
    display.clearDisplay();
    display.setCursor(0, 0);
    //display.printf("GPS (%d,%d)\n", _gpsLast.lat, _gpsLast.lon);
    //display.printf("%dm %u sats\n", _gpsLast.altitude, _gpsLast.satcnt);
    display.printf("Az:%u %dm\nEl:%u %dm\n", _targetAzim, _targetDistance,
        _targetElev, _gpsLast.altitude - _homeAlt);
    display.printf("SAz:%dus\nSEl:%dus\n", usAzim, usElev);
    display.display();
#endif

}

#if defined(AAT_SERIAL_INPUT)
void AatModule::processPacketIn(uint8_t len)
{
    const crsf_header_t *hdr = (crsf_header_t *)_rxBuf;
    if (hdr->sync_byte == CRSF_SYNC_BYTE) // || hdr->sync_byte == CRSF_ADDRESS_FLIGHT_CONTROLLER)
    {
        switch (hdr->type)
        {
        case CRSF_FRAMETYPE_GPS:
            SendGpsTelemetry((crsf_packet_gps_t *)hdr);
            break;
        }
    }
}

// Shift the bytes in the RxBuf down by cnt bytes
void AatModule::shiftRxBuffer(uint8_t cnt)
{
    // If removing the whole thing, just set pos to 0
    if (cnt >= _rxBufPos)
    {
        _rxBufPos = 0;
        return;
    }

    // Otherwise do the slow shift down
    uint8_t *src = &_rxBuf[cnt];
    uint8_t *dst = &_rxBuf[0];
    _rxBufPos -= cnt;
    uint8_t left = _rxBufPos;
    while (left--)
        *dst++ = *src++;
}

void AatModule::handleByteReceived()
{
    bool reprocess;
    do
    {
        reprocess = false;
        if (_rxBufPos > 1)
        {
            uint8_t len = _rxBuf[1];
            // Sanity check the declared length isn't outside Type + X{1,CRSF_MAX_PAYLOAD_LEN} + CRC
            // assumes there never will be a CRSF message that just has a type and no data (X)
            if (len < 3 || len > (CRSF_MAX_PAYLOAD_LEN + 2))
            {
                shiftRxBuffer(1);
                reprocess = true;
            }

            else if (_rxBufPos >= (len + 2))
            {
                uint8_t inCrc = _rxBuf[2 + len - 1];
                uint8_t crc = _crc.calc(&_rxBuf[2], len - 1);
                if (crc == inCrc)
                {
                    processPacketIn(len);
                    shiftRxBuffer(len + 2);
                    reprocess = true;
                }
                else
                {
                    shiftRxBuffer(1);
                    reprocess = true;
                }
            }  // if complete packet
        } // if pos > 1
    } while (reprocess);
}
#endif

void AatModule::checkSerialInput()
{
#if defined(AAT_SERIAL_INPUT)
    while (Serial.available())
    {
        uint8_t b = Serial.read();
        _rxBuf[_rxBufPos++] = b;

        handleByteReceived();

        if (_rxBufPos == (sizeof(_rxBuf)/sizeof(_rxBuf[0])))
        {
            // Packet buffer filled and no valid packet found, dump the whole thing
            _rxBufPos = 0;
        }
    }
#endif
}

void AatModule::Loop(uint32_t now)
{
    checkSerialInput();

    if (_gpsUpdated)
    {
        _gpsUpdated = false;
        processGps();
    }

    servoUpdate(now);
    ModuleBase::Loop(now);
}
#endif /* AAT_BACKPACK */