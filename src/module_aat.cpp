#if defined(AAT_BACKPACK)
#include "common.h"
#include "module_aat.h"
#include "logging.h"

#include <math.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#define RAD2DEG(rad) ((rad) * 180.0 / M_PI)
#define DELAY_IDLE          (20U)   // sleep used when not tracking
#define DELAY_FIRST_UPDATE  (5000U) // absolute delay before first servo update
#define FONT_W              (6)     // Actually 5x7 + 1 pixel space
#define FONT_H              (8)

static void calcDistAndAzimuth(int32_t srcLat, int32_t srcLon, int32_t dstLat, int32_t dstLon,
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

VbatSampler::VbatSampler() :
    _lastUpdate(0), _value(0), _sum(0), _cnt(0)
{
}

void VbatSampler::update(uint32_t now)
{
    if (_lastUpdate != 0 && now - _lastUpdate < VBAT_UPDATE_INTERVAL)
        return;
    _lastUpdate = now;

    uint32_t adc = analogRead(A0);
    _sum += adc;
    ++_cnt;

    if (_cnt >= VBAT_UPDATE_COUNT)
    {
        adc = _sum / _cnt;
        // For negative offsets, anything between abs(OFFSET) and 0 is considered 0
        if ((config.GetVbatOffset() < 0 && (int32_t)adc <= -config.GetVbatOffset())
            || config.GetVbatScale() == 0)
            _value = 0;
        else
            _value = ((int32_t)adc - config.GetVbatOffset()) * 100 / config.GetVbatScale();

        _sum = 0;
        _cnt = 0;
    }
}

AatModule::AatModule(Stream &port) :
    CrsfModuleBase(port), _gpsLast{0}, _home{0},
    _gpsAvgUpdateIntervalMs(0), _lastServoUpdateMs(0), _targetDistance(0),
    _targetAzim(0), _targetElev(0), _azimMsPerDegree(0),
    _servoPos{0}, _lastAzimFlipMs(0)
#if defined(PIN_SERVO_AZIM)
    , _servo_Azim()
#endif
#if defined(PIN_SERVO_ELEV)
    , _servo_Elev()
#endif
#if defined(PIN_OLED_SDA)
    , _display(SCREEN_WIDTH, SCREEN_HEIGHT), _lastDisplayActiveMs(0)
#endif
{
    // Init is called manually
}

void AatModule::Init()
{
#if !defined(DEBUG_LOG)
    // Need to call _port's end but it is a stream reference not the HardwareSerial
    Serial.end();
#endif
    _servoPos[IDX_AZIM] = (config.GetAatServoLow(IDX_AZIM) + config.GetAatServoHigh(IDX_AZIM)) / 2;
#if defined(PIN_SERVO_AZIM)
    _servo_Azim.attach(PIN_SERVO_AZIM, 500, 2500, _servoPos[IDX_AZIM]);
#endif
    _servoPos[IDX_ELEV] = (config.GetAatServoLow(IDX_ELEV) + config.GetAatServoHigh(IDX_ELEV)) / 2;
#if defined(PIN_SERVO_ELEV)
    _servo_Elev.attach(PIN_SERVO_ELEV, 500, 2500, _servoPos[IDX_ELEV]);
#endif
#if defined(PIN_OLED_SDA)
    displayInit();
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

    //DBGLN("GPS: (%d,%d) %dm %usats", _gpsLast.lat, _gpsLast.lon,
    //    _gpsLast.altitude, _gpsLast.satcnt);

    _gpsLast.updated = true;
}

void AatModule::updateGpsInterval(uint32_t interval)
{
    // Low pass filter. Note there is no fast init of the average, so it will take some time to grow
    // this prevents overprojection caused by the first update after setting home
    _gpsAvgUpdateIntervalMs += ((int32_t)interval - (int32_t)_gpsAvgUpdateIntervalMs) / 4;

    // Limit the maximum interval to provent projecting for too long
    const uint32_t GPS_UPDATE_INTERVAL_MAX = (10U * 1000U);
    if (_gpsAvgUpdateIntervalMs > GPS_UPDATE_INTERVAL_MAX)
        _gpsAvgUpdateIntervalMs = GPS_UPDATE_INTERVAL_MAX;
}

uint8_t AatModule::calcGpsIntervalPct(uint32_t now)
{
    if (_gpsAvgUpdateIntervalMs)
    {
        return constrain((now - _gpsLast.lastUpdateMs) * 100U / (uint32_t)_gpsAvgUpdateIntervalMs, 0U, 100U);
    }

    return 0;
}

void AatModule::processGps(uint32_t now)
{
    if (!_gpsLast.updated || _isOverrideMode)
        return;
    _gpsLast.updated = false;

    // Actually want to track time between _processing_ each GPS update
    uint32_t interval = now - _gpsLast.lastUpdateMs;
    _gpsLast.lastUpdateMs = now;

    // Check if need to set home position
    bool didSetHome = false;
    if (!isHomeSet())
    {
        if (_gpsLast.satcnt >= config.GetAatSatelliteHomeMin())
        {
            didSetHome = true;
            _home.lat = _gpsLast.lat;
            _home.lon = _gpsLast.lon;
            _home.alt = _gpsLast.altitude;
            DBGLN("GPS Home set to (%d,%d)", _home.lat, _home.lon);
        }
        else
            return;
    }

    uint32_t azimuth;
    uint32_t distance;
    calcDistAndAzimuth(_home.lat, _home.lon, _gpsLast.lat, _gpsLast.lon, &distance, &azimuth);
    uint8_t elevation = constrain(calcElevation(distance, _gpsLast.altitude - _home.alt), 0, 90);
    DBGLN("Azimuth: %udeg Elevation: %udeg Distance: %um", azimuth, elevation, distance);

    // Calculate angular velocity to allow dead reckoning projection
    if (!didSetHome)
    {
        updateGpsInterval(interval);
        // azimDelta is the azimuth change since the last packet, -180 to +180
        int32_t azimDelta = (azimuth - _targetAzim + 540) % 360 - 180;
        _azimMsPerDegree = (azimDelta == 0) ? 0 : (int32_t)interval / azimDelta;
        DBGLN("%d delta in %ums, %dms/d %uavg", azimDelta, interval, _azimMsPerDegree, _gpsAvgUpdateIntervalMs);
    }

    _targetDistance = distance;
    _targetElev = elevation;
    _targetAzim = azimuth;
}

int32_t AatModule::calcProjectedAzim(uint32_t now)
{
    // Attempt to do a linear projection of the last
    // If enabled, we know the GPS update rate, the azimuth has changed, and more than a few meters away
    if (config.GetAatProject() && _gpsAvgUpdateIntervalMs && _azimMsPerDegree && _targetDistance > 3)
    {
        uint32_t elapsed = constrain(now - _gpsLast.lastUpdateMs, 0U, _gpsAvgUpdateIntervalMs);

        // Prevent excessive rotational velocity (100 degrees per second / 10ms per degree)
        int32_t azimMsPDLimited;
        if (_azimMsPerDegree > -10 && _azimMsPerDegree < 10)
            if (_azimMsPerDegree > 0)
                azimMsPDLimited = 10;
            else
                azimMsPDLimited = -10;
        else
            azimMsPDLimited = _azimMsPerDegree;

        int32_t target = (((int32_t)elapsed / azimMsPDLimited) + _targetAzim + 360) % 360;
        //DBGLN("%u t=%d p=%d", elapsed, _targetAzim, target);
        return target;
    }

    return _targetAzim;
}

/**
 * @brief: Calculate the bearing from centerdir to azim
 * @return: -180 to +179 bearing
*/
const int32_t AatModule::azimToBearing(int32_t azim) const
{
    int32_t center = 45 * config.GetAatCenterDir();
    return ((azim - center + 540) % 360) - 180;  // -180 to +179
}

#if defined(PIN_OLED_SDA)
void AatModule::displayInit()
{
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    _display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    _display.setTextWrap(false);
    displayState();
}

void AatModule::displayState()
{
    // Boot screen with vbat, version, and bind/wifi state
    _display.clearDisplay();
    _display.setTextSize(2);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    if (connectionState == binding)
    {
        _display.write("Bind\nmode...\n\n");
    }
    else if (connectionState == wifiUpdate)
    {
        _display.write("Wifi\nmode...\n");
        _display.setTextSize(1);
        IPAddress localAddr;
        if (WiFi.getMode() == WIFI_STA)
            localAddr = WiFi.localIP();
        else
            localAddr = WiFi.softAPIP();
        _display.print(localAddr.toString());
        _display.write("\n\n");
    }
    else
        _display.write("AAT\nBackpack\n\n");

    _display.setTextSize(1);
    _display.print(VERSION);

    displayVBat();
    _display.display();
}

void AatModule::displayGpsIdle(uint32_t now)
{
    // A screen with just the GPS position, sat count, vbat, and interval bar
    _display.clearDisplay();
    _display.setCursor(0, 0);

    _display.setTextSize(2);
    _display.printf("Sats: %u\n",  _gpsLast.satcnt);

    _display.setTextSize(1);
    _display.printf("\nLat: %d.%07d\nLon: %d.%07d",
        _gpsLast.lat / 10000000, abs(_gpsLast.lat) % 10000000,
        _gpsLast.lon / 10000000, abs(_gpsLast.lon) % 10000000
    );

    displayGpsIntervalBar(now);
    displayVBat();
    _display.display();
}

void AatModule::displayAzimuthExtent(int32_t y)
{
    switch ((ServoMode)config.GetAatServoMode())
    {
        // Just a line on the left / right of the azim line
        default: /* fallthrough */
        case ServoMode::TwoToOne: /* fallthrough */
        case ServoMode::Flip180:
            _display.drawFastVLine(0, y, 5, SSD1306_WHITE);
            _display.drawFastVLine(SCREEN_WIDTH-1, y, 5, SSD1306_WHITE);
            break;

        // 180 mode put the lines at the extent of the servo rotation
        case ServoMode::Clip180:
            //_display.drawFastVLine(SCREEN_WIDTH/4, y, 5, SSD1306_WHITE);
            //_display.drawFastVLine(3*SCREEN_WIDTH/4, y, 5, SSD1306_WHITE);
            // Dotted line all the way up the elevation field
            for (int32_t dotY=0; dotY<(33+5); dotY+=4)
            {
                _display.drawPixel(SCREEN_WIDTH/4, dotY+16, SSD1306_WHITE);
                _display.drawPixel(3*SCREEN_WIDTH/4, dotY+16, SSD1306_WHITE);
            }
            break;
    }
}

void AatModule::displayAzimuth(int32_t projectedAzim)
{
    int32_t y = SCREEN_HEIGHT - FONT_H + 1 - 6;

    // horizon line + arrow + unprojected azim line
    // |-----/\---|-|
    _display.drawFastHLine(0, y+2, SCREEN_WIDTH-1, SSD1306_WHITE);
    displayAzimuthExtent(y);

    // Unprojected azimuth line
    int32_t azimPos = map(azimToBearing(_targetAzim), -180, 180, 0, SCREEN_WIDTH);
    _display.drawFastVLine(azimPos, y+1, 3, SSD1306_WHITE);
    // Projected Azimuth arrow
    azimPos = map(azimToBearing(projectedAzim), -180, 180, 0, SCREEN_WIDTH);
    const uint8_t oledim_arrowup[] = { 0x10, 0x38, 0x7c }; // 'arrowup', 7x3px
    _display.drawBitmap(azimPos-7/2, y+1, oledim_arrowup, 7, 3, SSD1306_WHITE, SSD1306_BLACK);

    // S    W    N    E    S under that
    y += 6;
    const char AZIM_LABELS[] = "SWNES" "WNESW" "NESWN" "ESWNE"; // 5 characters for each direction, 3rd character = center
    const char *labels = &AZIM_LABELS[config.GetAatCenterDir()*5/2];
    _display.drawChar(0, y, labels[0], SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH/4-FONT_W/2, y, labels[1], SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH/2-FONT_W/2+1, y, labels[2], SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(3*SCREEN_WIDTH/4-FONT_W/2, y, labels[3], SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH-FONT_W+1, y, labels[4], SSD1306_WHITE, SSD1306_BLACK, 1);

    // Projected azim value as 3 digit number on a white background (max 2px each side)
    y -= 2;
    uint32_t azimStart = constrain(azimPos-((3*FONT_W)/2), -2, SCREEN_WIDTH-(3*FONT_W)-2);
    _display.fillRoundRect(azimStart, y, 3*FONT_W + 3, FONT_H + 1, 2, SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_BLACK);
    _display.setCursor(azimStart+2, y+1);
    _display.printf("%03u", projectedAzim);
}

void AatModule::displayAltitude(int32_t azimPos, int32_t elevPos)
{
    int alt = constrain(_gpsLast.altitude - _home.alt, -99, 999);
    if (alt == 0)
        return;

    char buf[16];
    snprintf(buf, sizeof(buf), "%dm", alt);

    // If elevation is low, put alt over the pip, if elev high put the alt below it
    // and allow the units to go off the screen to the right if needed
    int32_t strWidth = strlen(buf) * FONT_W;
    int32_t distX = constrain(azimPos - (strWidth/2) + 1, 0, SCREEN_WIDTH + FONT_W - strWidth);
    int32_t distY = (_targetElev < 50) ? elevPos - FONT_H - 5 : elevPos + 6;
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(distX, distY);
    _display.write(buf);
}

void AatModule::displayTargetCircle(int32_t projectedAzim)
{
    const int32_t elevH = 33;
    // yellow-blue OLED have 16 pixels of yellow, start below that
    const int32_t elevOff = 16;
    // Dotted line to separate top of screen from tracking area
    for (int32_t x=0; x<SCREEN_WIDTH; x+=3)
        _display.drawPixel(x, elevOff, SSD1306_WHITE);

    int32_t elevPos = map(_targetElev, 0, 90, elevH, 0) + elevOff;
    //elevPos = 0 + elevOff;
    // X for projectedAzim
    int32_t azimPos = map(azimToBearing(projectedAzim), -180, 180, 0, SCREEN_WIDTH);
    _display.drawPixel(azimPos-1, elevPos-1, SSD1306_WHITE);
    _display.drawPixel(azimPos+1, elevPos-1, SSD1306_WHITE);
    _display.drawPixel(azimPos, elevPos, SSD1306_WHITE);
    _display.drawPixel(azimPos-1, elevPos+1, SSD1306_WHITE);
    _display.drawPixel(azimPos+1, elevPos+1, SSD1306_WHITE);

    // circle/rectangle cage for _servoPos
    int32_t servoXMin = ((ServoMode)config.GetAatServoMode() == ServoMode::Clip180) ? (SCREEN_WIDTH/4) : 0;
    int32_t servoXMax = ((ServoMode)config.GetAatServoMode() == ServoMode::Clip180) ? (3*SCREEN_WIDTH/4) : SCREEN_WIDTH;
    int32_t servoX = map(_servoPos[IDX_AZIM],
        config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM),
        servoXMin, servoXMax);
    int32_t servoY = map(_servoPos[IDX_ELEV],
        config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV),
        elevH, 0) + elevOff;
    //servoY = 0 + elevOff
    _display.drawFastHLine(servoX-1, servoY-3, 3, SSD1306_WHITE);
    _display.drawFastHLine(servoX-1, servoY+3, 3, SSD1306_WHITE);
    _display.drawFastVLine(servoX-3, servoY-1, 3, SSD1306_WHITE);
    _display.drawFastVLine(servoX+3, servoY-1, 3, SSD1306_WHITE);

    displayAltitude(azimPos, elevPos);
}

void AatModule::displayTargetDistance()
{
    // Target distance over the X: 9m, 99m, 999m, 9.9k, 99.9k, 999k
    char dist[16];
    const char *units;
    if (_targetDistance > 99999) // >99.99m = "xxx.x" (5 chars)
    {
        snprintf(dist, sizeof(dist), "%u.%u", _targetDistance / 1000, (_targetDistance % 1000) / 100);
        units = "km";
    }
    else if (_targetDistance > 999) // >999m = "[x]x.xx" (4-5 chars)
    {
        snprintf(dist, sizeof(dist), "%u.%02u", _targetDistance / 1000, (_targetDistance % 1000) / 10);
        units = "km";
    }
    else // "[x]xx" (2-3 chars)
    {
        snprintf(dist, sizeof(dist), "%2u", _targetDistance);
        units = "m";
    }
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.write("Dst");
    _display.setTextSize(2);
    _display.setCursor(3 * FONT_W + 3, 0);
    _display.printf(dist);

    _display.setTextSize(1);
    _display.setCursor(_display.getCursorX() + 1, FONT_H);
    _display.write(units);
}

void AatModule::displayVBat()
{
    if (_vbat.value() == 0)
        return;

    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(SCREEN_WIDTH - 5*FONT_W, FONT_H);
    _display.printf("%2d.%1dV", _vbat.value() / 10, _vbat.value() % 10);
}

void AatModule::displayActive(uint32_t now, int32_t projectedAzim)
{
    // Throttle the dislay update rate to <60Hz as the tracker runs 100Hz
    const uint32_t DISPLAY_UPDATE_MS = 16;
    if (now - _lastDisplayActiveMs < DISPLAY_UPDATE_MS)
        return;
    _lastDisplayActiveMs = now;

    _display.clearDisplay();

    displayGpsIntervalBar(now);
    displayAzimuth(projectedAzim);
    displayTargetCircle(projectedAzim);
    displayTargetDistance();
    displayVBat();

    _display.display();
}

void AatModule::displayGpsIntervalBar(uint32_t now)
{
    if (_gpsAvgUpdateIntervalMs == 0)
        return;

    if (now - _gpsLast.lastUpdateMs > (3*_gpsAvgUpdateIntervalMs))
    {
        // Inverse "OLD" in the center of where the update bar would be
        _display.fillRoundRect(80, 0, SCREEN_WIDTH-1-80, FONT_H-1, 2, SSD1306_WHITE);
        _display.setTextSize(1);
        _display.setTextColor(SSD1306_BLACK);
        _display.setCursor(104 - (3*FONT_W)/2, 0);
        _display.write("OLD");
    }
    else
    {
        // A depleting bar going from screen center to right
        uint8_t gpsIntervalPct = calcGpsIntervalPct(now);
        uint8_t pxWidth = (SCREEN_WIDTH-1-80) * (100U - gpsIntervalPct) / 100U;
        _display.fillRect(SCREEN_WIDTH-pxWidth, 0, pxWidth, 2, SSD1306_WHITE);
    }
}
#endif /* defined(PIN_OLED_SDA) */

void AatModule::servoApplyMode(int32_t azim, int32_t elev, int32_t newServoPos[])
{
    int32_t bearing = azimToBearing(azim);

    // 2-to-1 reduction can do 360 so the input and output is the same
    if ((ServoMode)config.GetAatServoMode() == ServoMode::TwoToOne)
    {
        newServoPos[IDX_AZIM] = map(bearing, -180, 179, config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM));
        newServoPos[IDX_ELEV] = map(elev, 0, 90, config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV));
        return;
    }

    // Clip180 limits azim to 90 degrees left/right from center
    if ((ServoMode)config.GetAatServoMode() == ServoMode::Clip180)
    {
        bearing = constrain(bearing, -90, 90);
        newServoPos[IDX_AZIM] = map(bearing, -90, 90, config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM));
        newServoPos[IDX_ELEV] = map(elev, 0, 90, config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV));
        return;
    }

    if ((ServoMode)config.GetAatServoMode() == ServoMode::Flip180)
    {
        if (bearing < -90)
        {
            bearing = -180 - bearing;
            elev = 180 - elev;
        }
        else if (bearing > 90)
        {
            bearing = 180 - bearing;
            elev = 180 - elev;
        }
        newServoPos[IDX_AZIM] = map(bearing, -90, 90, config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM));
        newServoPos[IDX_ELEV] = map(elev, 0, 180, config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV));
        return;
    }
}

void AatModule::servoUpdate(uint32_t now)
{
    uint32_t interval = now - _lastServoUpdateMs;
    if (interval < 10U)
        return;
    _lastServoUpdateMs = now;

    // If the servo endpoints aren't valid, all this math will divide by zero all over
    if (!config.GetAatServoEndpointsValid())
        return;

    int32_t projectedAzim = calcProjectedAzim(now);
    int32_t newServoPos[IDX_COUNT];
    servoApplyMode(projectedAzim, _targetElev, newServoPos);

    for (uint32_t idx=IDX_AZIM; idx<IDX_COUNT; ++idx)
    {
        // Use smoothness to denote the maximum us per 10ms update
        const int32_t SMOOTHNESS_US_PER_STEP = (config.GetAatServoSmooth() < 3) ? 6 : 4;
        int32_t range = (config.GetAatServoHigh(idx) - config.GetAatServoLow(idx));
        int32_t diff = newServoPos[idx] - _servoPos[idx];
        int32_t maxDiff = (10 - config.GetAatServoSmooth()) * SMOOTHNESS_US_PER_STEP;
        // If the distance the servo needs to go is more than 80% away
        // jump immediately. otherwise smooth it
        if (idx == IDX_AZIM && (abs(diff) * 100 / range) > 80)
        {
            // Prevent the servo from flipping back and forth around the 180 point
            // by only allowing 1 flip ever Xms. Just keep pushing toward the limit
            const uint32_t AZIM_FLIP_MIN_DELAY = 2000U;
            if (now - _lastAzimFlipMs > AZIM_FLIP_MIN_DELAY)
            {
                _servoPos[idx] = newServoPos[idx];
                _lastAzimFlipMs = now;
            }
            else if (diff < 0)
                _servoPos[idx] = constrain(_servoPos[idx] + maxDiff, _servoPos[idx], config.GetAatServoHigh(idx));
            else
                _servoPos[idx] = constrain(_servoPos[idx] - maxDiff, config.GetAatServoLow(idx), _servoPos[idx]);
        }
        else
            _servoPos[idx] += constrain(diff, -maxDiff, maxDiff);
    }
    //DBGLN("t=%u pro=%d us=%d smoo=%d", _targetAzim, projectedAzim, newServoPos[IDX_AZIM], _servoPos[IDX_AZIM]);

#if defined(PIN_SERVO_AZIM)
    _servo_Azim.writeMicroseconds(_servoPos[IDX_AZIM]);
#endif
#if defined(PIN_SERVO_ELEV)
    _servo_Elev.writeMicroseconds(_servoPos[IDX_ELEV]);
#endif

#if defined(PIN_OLED_SDA)
    displayActive(now, projectedAzim);
#endif
}

uint32_t AatModule::getVbat()
{
    return _vbat.value();
}

void AatModule::overrideTargetCommon(int32_t azimuth, int32_t elevation)
{
    _targetAzim = azimuth;
    _targetElev = elevation;

    // Clear out other fields to not do projection or process any updates
    _isOverrideMode = true;
    _gpsAvgUpdateIntervalMs = 0;
    _targetDistance = 0;
    _azimMsPerDegree = 0;
}

void AatModule::overrideTargetBearing(int32_t bearing)
{
    // bearing is -180 to +180, azimuth 0-360
    int32_t azim = (360 + bearing + (45 * config.GetAatCenterDir())) % 360;
    // The furthest right a servo can go is +179 degrees, +180 goes to the -180 position
    if (bearing == 180)
        azim = (azim == 0) ? 359 : azim - 1;
    overrideTargetCommon(azim, _targetElev);
}

void AatModule::overrideTargetElev(int32_t elev)
{
    overrideTargetCommon(_targetAzim, constrain(elev, 0, 90));
}

void AatModule::onCrsfPacketIn(const crsf_header_t *pkt)
{
    if (pkt->sync_byte == CRSF_SYNC_BYTE)
    {
        if (pkt->type == CRSF_FRAMETYPE_GPS)
            SendGpsTelemetry((crsf_packet_gps_t *)pkt);
    }
}

void AatModule::Loop(uint32_t now)
{
    processGps(now);
    _vbat.update(now);

    if (isHomeSet() && now > DELAY_FIRST_UPDATE)
    {
        servoUpdate(now);
    }
    else
    {
#if defined(PIN_OLED_SDA)
        if (isGpsActive())
            displayGpsIdle(now);
        else
            displayState();
#endif
        delay(DELAY_IDLE);
    }

    CrsfModuleBase::Loop(now);
}
#endif /* AAT_BACKPACK */