#if defined(AAT_BACKPACK)
#include "common.h"
#include "module_aat.h"
#include "logging.h"
#include "devWifi.h"

#include <math.h>
#include <Arduino.h>

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

    if (_cnt == VBAT_UPDATE_COUNT)
    {
        adc = _sum / _cnt;
        // For negative offsets, anything between abs(OFFSET) and 0 is considered 0
        if (config.GetVBatOffset() < 0 && (int32_t)adc <= -config.GetVBatOffset())
            _value = 0;
        else
            _value = ((int32_t)adc - config.GetVBatOffset()) * 100 / config.GetVbatScale();

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
    , _display(SCREEN_WIDTH, SCREEN_HEIGHT)
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
    if (!_gpsLast.updated)
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

#if defined(PIN_OLED_SDA)
void AatModule::displayInit()
{
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    _display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    _display.setTextWrap(false);
    _display.setTextSize(2);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(0, 0);
    if (connectionState == binding)
        _display.print("Bind\nmode...\n\n");
    else if (connectionState == wifiUpdate)
        _display.print("Wifi\nmode...\n\n");
    else
        _display.print("AAT\nBackpack\n\n");
    _display.setTextSize(1);
    _display.print(VERSION);
    _display.display();
}

void AatModule::displayIdle(uint32_t now)
{
    // A screen with just the GPS position, sat count, and interval bar
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
    _display.display();
}

void AatModule::displayAzimuth(int32_t projectedAzim)
{
    int32_t y = SCREEN_HEIGHT - FONT_H + 1 - 6;

    // start with horizon line with arrow and unprojected azimuth
    // |-----/\---|-|
    _display.drawFastVLine(0, y, 5, SSD1306_WHITE);
    _display.drawFastHLine(0, y+2, SCREEN_WIDTH-1, SSD1306_WHITE);
    _display.drawFastVLine(SCREEN_WIDTH-1, y, 5, SSD1306_WHITE);
    int32_t azimPos = map((_targetAzim + 180) % 360, 0, 360, 0, SCREEN_WIDTH);
    _display.drawFastVLine(azimPos, y+1, 3, SSD1306_WHITE);
    // Projected Azimuth arrow
    azimPos = map((projectedAzim + 180) % 360, 0, 360, 0, SCREEN_WIDTH);
    const uint8_t oledim_arrowup[] = { 0x10, 0x38, 0x7c }; // 'arrowup', 7x3px
    _display.drawBitmap(azimPos-7/2, y+1, oledim_arrowup, 7, 3, SSD1306_WHITE, SSD1306_BLACK);

    // S    W    N    E    S under that
    y += 6;
    _display.drawChar(0, y, 'S', SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH-FONT_W+1, y, 'S', SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH/4-FONT_W/2, y, 'W', SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(SCREEN_WIDTH/2-FONT_W/2+1, y, 'N', SSD1306_WHITE, SSD1306_BLACK, 1);
    _display.drawChar(3*SCREEN_WIDTH/4-FONT_W/2, y, 'E', SSD1306_WHITE, SSD1306_BLACK, 1);

    // Projected azim value as 3 digit number on a white background (max 2px each side)
    y -= 2;
    uint32_t azimStart = constrain(azimPos-((3*FONT_W)/2), -2, SCREEN_WIDTH-(3*FONT_W)-2);
    _display.fillRoundRect(azimStart, y, 3*FONT_W + 3, FONT_H + 1, 2, SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_BLACK);
    _display.setCursor(azimStart+2, y+1);
    _display.printf("%03u", projectedAzim);
}

void AatModule::displayTargetDistance(int32_t azimPos, int32_t elevPos)
{
    if (_targetDistance == 0)
        return;

    // Target distance over the X: 9m, 99m, 999m, 9.9k, 99.9k, 999k
    char dist[16];
    if (_targetDistance > 99999) // > 99.9km = "xxxk" can you believe that?! so far.
    {
        snprintf(dist, sizeof(dist), "%3uk", _targetDistance / 1000);
    }
    else if (_targetDistance > 999) // >999m = "xx.xk"
    {
        snprintf(dist, sizeof(dist), "%u.%uk", _targetDistance / 1000, (_targetDistance % 1000) / 100);
    }
    else // "xxxm"
    {
        snprintf(dist, sizeof(dist), "%um", _targetDistance);
    }

    // If elevation is low, put distance over the pip, if elev high put the distance below it
    // and allow the units to go off the screen to the right if needed
    int32_t strWidth = strlen(dist) * FONT_W;
    int32_t distX = constrain(azimPos - (strWidth/2) + 1, 0, SCREEN_WIDTH + FONT_W - strWidth);
    int32_t distY = (_targetElev < 50) ? elevPos - FONT_H - 5 : elevPos + 6;
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(distX, distY);
    _display.write(dist);
}

void AatModule::displayTargetCircle(int32_t projectedAzim)
{
    const int32_t elevH = 33;
    const int32_t elevOff = 16;
    // Dotted line to separate top of screen from tracking area
    for (int32_t x=0; x<SCREEN_WIDTH; x+=2)
        _display.drawPixel(x, elevOff, SSD1306_WHITE);

    int32_t elevPos = map(_targetElev, 0, 90, elevH, 0) + elevOff;
    //elevPos = 0 + elevOff;
    // X for _projectedAzim
    int32_t azimPos = map((projectedAzim + 180) % 360, 0, 360, 0, SCREEN_WIDTH);
    _display.drawPixel(azimPos-1, elevPos-1, SSD1306_WHITE);
    _display.drawPixel(azimPos+1, elevPos-1, SSD1306_WHITE);
    _display.drawPixel(azimPos, elevPos, SSD1306_WHITE);
    _display.drawPixel(azimPos-1, elevPos+1, SSD1306_WHITE);
    _display.drawPixel(azimPos+1, elevPos+1, SSD1306_WHITE);

    // circle/rectangle cage for _servoPos
    int32_t servoX = map(_servoPos[IDX_AZIM],
        config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM),
        0, SCREEN_WIDTH);
    int32_t servoY = map(_servoPos[IDX_ELEV],
        config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV),
        elevH, 0) + elevOff;
    //servoY = 0 + elevOff
    _display.drawFastHLine(servoX-1, servoY-3, 3, SSD1306_WHITE);
    _display.drawFastHLine(servoX-1, servoY+3, 3, SSD1306_WHITE);
    _display.drawFastVLine(servoX-3, servoY-1, 3, SSD1306_WHITE);
    _display.drawFastVLine(servoX+3, servoY-1, 3, SSD1306_WHITE);

    displayTargetDistance(azimPos, elevPos);
}

void AatModule::displayVBat()
{
    if (_vbat.value() == 0)
        return;

    //_display.setTextColor(SSD1306_WHITE);
    //_display.setTextSize(1);
    //_display.setCursor(0, SCREEN_HEIGHT-FONT_H+1); // SCREEN_WIDTH-(5*FONT_W)+1
    char buf[16];
    snprintf(buf, sizeof(buf), "%2d.%01dV", _vbat.value() / 100, (_vbat.value() % 100) / 10);

    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(SCREEN_WIDTH - strlen(buf)*FONT_W, FONT_H);
    _display.write(buf);
}

void AatModule::displayAltitude()
{
    _display.setTextColor(SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.write("Alt ");
    _display.setTextSize(2);
    _display.printf("%3dm", constrain(_gpsLast.altitude - _home.alt, -99, 999));
}

void AatModule::displayActive(uint32_t now, int32_t projectedAzim)
{
    _display.clearDisplay();

    displayGpsIntervalBar(now);
    displayAzimuth(projectedAzim);
    displayTargetCircle(projectedAzim);
    displayAltitude();
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
        uint8_t pxWidth = SCREEN_WIDTH/2 * (100U - gpsIntervalPct) / 100U;
        _display.fillRect(SCREEN_WIDTH-pxWidth, 0, pxWidth, 2, SSD1306_WHITE);
    }
}
#endif /* defined(PIN_OLED_SDA) */

void AatModule::servoUpdate(uint32_t now)
{
    uint32_t interval = now - _lastServoUpdateMs;
    if (interval < 20U)
        return;
    _lastServoUpdateMs = now;

    int32_t projectedAzim = calcProjectedAzim(now);
    int32_t transformedAzim = projectedAzim;
    int32_t transformedElev = _targetElev;

    // For 1:2 gearing on the azim servo to allow 360 rotation
    // For Elev servos that only go 0-90 and the azim does 360
    transformedAzim = (transformedAzim + 180) % 360; // convert so 0 maps to 1500us
    int32_t newServoPos[IDX_COUNT];
    newServoPos[IDX_AZIM] = map(transformedAzim, 0, 360, config.GetAatServoLow(IDX_AZIM), config.GetAatServoHigh(IDX_AZIM));
    newServoPos[IDX_ELEV] = map(transformedElev, 0, 90, config.GetAatServoLow(IDX_ELEV), config.GetAatServoHigh(IDX_ELEV));

    for (uint32_t idx=IDX_AZIM; idx<IDX_COUNT; ++idx)
    {
        // Use smoothness to denote the maximum us per 20ms update
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
            displayIdle(now);
#endif
        delay(DELAY_IDLE);
    }

    CrsfModuleBase::Loop(now);
}
#endif /* AAT_BACKPACK */