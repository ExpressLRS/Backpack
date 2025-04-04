#include <Arduino.h>

#include <Wire.h>

#include "crsf_protocol.h"

#include "devHeadTracker.h"

#if defined(HAS_HEADTRACKING)
#include "config.h"
#include "logging.h"

#include "IMUBase.h"
#include "ICMSeries.h"
#include "MPU6050.h"
#include "QMI8658C.h"
#include "Fusion.h"

static HeadTrackerState ht_state = STATE_ERROR;
static IMUBase *imu;
static bool hasCompass = false;
static FusionAhrs ahrs;

static float orientation[3] = {0.0, 0.0, 0.0};
static FusionEuler euler;
static float rollHome = 0, pitchHome = 0, yawHome = 0;
static int calibrationData[3][2];
static uint32_t cal_started;

static void initialize()
{
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000);
    Wire.setTimeout(1000);

    // Initializing the IMU
    imu = new MPU6050();
    if (imu->initialize()) {
        DBGLN("Found MPU6050 IMU")
    }
    else {
        delete imu;
        imu = new QMI8658C();
        if (imu->initialize()) {
            DBGLN("Found QMI8658C IMU")
        }
        else {
            delete imu;
            imu = new ICMSeries();
            if (imu->initialize()) {
                DBGLN("Found ICM Series IMU")
            }
            else {
                delete imu;
                ht_state = STATE_ERROR;
                return;
            }
        }
    }
    imu->setInterruptHandler(PIN_INT);

    FusionAhrsInitialise(&ahrs);
    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
            .convention = FusionConventionNwu,
            .gain = 0.5f,
            .gyroscopeRange = imu->getGyroRange(), /* replace this with actual gyroscope range in degrees/s */
            .accelerationRejection = 10.0f,
            .magneticRejection = 10.0f,
            .recoveryTriggerPeriod = 5U * imu->getSampleRate(), /* 5 seconds */
    };
    FusionAhrsSetSettings(&ahrs, &settings);
    DBGLN("starting head tracker");
    ht_state = STATE_RUNNING;
}

static int start()
{
    if (ht_state == STATE_ERROR)
    {
        return DURATION_NEVER;
    }
    imu->setCalibration(config.GetIMUCalibration());
    memcpy(orientation, *config.GetBoardOrientation(), sizeof(orientation));

#ifdef DEBUG_LOG
    Serial.printf("Load %7.2f %7.2f %7.2f\r\n", orientation[0], orientation[1], orientation[2]);
#endif
    if (isnan(orientation[0]) || isnan(orientation[2]) || isnan(orientation[2])) {
        orientation[0] = 0;
        orientation[1] = 0;
        orientation[2] = 0;
    }
    FusionAhrsReset(&ahrs);
    return DURATION_IMMEDIATELY;
}

// Normalizes any number to an arbitrary range
// by assuming the range wraps around when going below min or above max
static float normalize(float value, float start, float end) {
    float width = end - start;         //
    float offsetValue = value - start; // value relative to 0

    return (offsetValue - (floor(offsetValue / width) * width)) + start;
    // + start to reset back to start of original range
}

// Rotate a point (pn) in space in Order X -> Y -> Z
static void rotate(float pn[3], const float rot[3]) {
    float out[3];

    // X-axis Rotation
    if (rot[0] != 0) {
        out[0] = pn[0] * 1 + pn[1] * 0 + pn[2] * 0;
        out[1] = pn[0] * 0 + pn[1] * cos(rot[0]) - pn[2] * sin(rot[0]);
        out[2] = pn[0] * 0 + pn[1] * sin(rot[0]) + pn[2] * cos(rot[0]);
        memcpy(pn, out, sizeof(out[0]) * 3);
    }

    // Y-axis Rotation
    if (rot[1] != 0) {
        out[0] = pn[0] * cos(rot[1]) - pn[1] * 0 + pn[2] * sin(rot[1]);
        out[1] = pn[0] * 0 + pn[1] * 1 + pn[2] * 0;
        out[2] = -pn[0] * sin(rot[1]) + pn[1] * 0 + pn[2] * cos(rot[1]);
        memcpy(pn, out, sizeof(out[0]) * 3);
    }

    // Z-axis Rotation
    if (rot[2] != 0) {
        out[0] = pn[0] * cos(rot[2]) - pn[1] * sin(rot[2]) + pn[2] * 0;
        out[1] = pn[0] * sin(rot[2]) + pn[1] * cos(rot[2]) + pn[2] * 0;
        out[2] = pn[0] * 0 + pn[1] * 0 + pn[2] * 1;
        memcpy(pn, out, sizeof(out[0]) * 3);
    }
}

static int timeout()
{
    FusionVector a;
    FusionVector g;

    if (!imu->readIMUData(a, g))
        return DURATION_IMMEDIATELY;

    switch (ht_state) {
        case STATE_RUNNING: {
            rotate(a.array, orientation);
            rotate(g.array, orientation);

            // Calculate delta time (in seconds) to account for gyroscope sample clock error
            const clock_t timestamp = micros();
            static clock_t previousTimestamp;
            const float deltaTime = (float) (timestamp - previousTimestamp) / (float) 1000000;
            previousTimestamp = timestamp;

            FusionAhrsUpdate(&ahrs, g, a, FUSION_VECTOR_ZERO, deltaTime);

            euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
            euler.angle.roll -= rollHome;
            euler.angle.pitch -= pitchHome;
            euler.angle.yaw -= yawHome;
            break;
        }

        case STATE_IMU_CALIBRATING: {
            if (imu->updateCalibration(g)) {
                config.SetIMUCalibration(imu->getCalibration());
                config.Commit();
                ht_state = STATE_RUNNING;
            }
            break;
        }
    }
    return DURATION_IMMEDIATELY;
}

void startIMUCalibration()
{
    imu->beginCalibration();
    cal_started = millis();
    ht_state = STATE_IMU_CALIBRATING;
}

HeadTrackerState getHeadTrackerState()
{
    return ht_state;
}

void resetBoardOrientation()
{
    orientation[0] = 0;
    orientation[1] = 0;
    orientation[2] = 0;
    rollHome = 0;
    pitchHome = 0;
    yawHome = 0;
    FusionAhrsReset(&ahrs);
}

void saveBoardOrientation()
{
#ifdef DEBUG_LOG
    Serial.printf("Save %7.2f %7.2f %7.2f\r\n", orientation[0], orientation[1], orientation[2]);
#endif
    config.SetBoardOrientation(orientation);
    config.Commit();
}

void setBoardOrientation(int xAngle, int yAngle, int zAngle)
{
    orientation[0] = xAngle * DEG_TO_RAD;
    orientation[1] = yAngle * DEG_TO_RAD;
    orientation[2] = zAngle * DEG_TO_RAD;
    ahrs.initialising = true;
    ahrs.rampedGain = 10.0f;
}

void resetCenter()
{
    rollHome += euler.angle.roll;
    pitchHome += euler.angle.pitch;
    yawHome += euler.angle.yaw;
    rollHome = normalize(rollHome, -180.0, 180.0);
    pitchHome = normalize(pitchHome, -180.0, 180.0);
    yawHome = normalize(yawHome, -180.0, 180.0);
}

void getEuler(float *yaw, float *pitch, float *roll)
{
    *yaw = normalize(euler.angle.yaw, -180.0, 180.0);
    *pitch = normalize(euler.angle.pitch, -180.0, 180.0);
    *roll = normalize(euler.angle.roll, -180.0, 180.0);
}

device_t HeadTracker_device = {
    .initialize = initialize,
    .start = start,
    .event = nullptr,
    .timeout = timeout
};

#endif
#if defined(SUPPORT_HEADTRACKING)
extern int16_t ptrChannelData[3];

HeadTrackerState getHeadTrackerState()
{
    return STATE_RUNNING;
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    const float run = in_max - in_min;
    const float rise = out_max - out_min;
    const float delta = x - in_min;
    return (delta * rise) / run + out_min;
}

void resetCenter()
{
}

void getEuler(float *yaw, float *pitch, float *roll)
{
    *yaw = -fmap(ptrChannelData[0], CRSF_CHANNEL_VALUE_1000, CRSF_CHANNEL_VALUE_2000, -180.0, 180.0);
    *pitch = fmap(ptrChannelData[2], CRSF_CHANNEL_VALUE_1000, CRSF_CHANNEL_VALUE_2000, -180.0, 180.0);
    *roll = fmap(ptrChannelData[1], CRSF_CHANNEL_VALUE_1000, CRSF_CHANNEL_VALUE_2000, -180.0, 180.0);
}
#endif