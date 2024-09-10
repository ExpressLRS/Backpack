#include <Arduino.h>

#if defined(HAS_HEADTRACKING)
#include <Wire.h>
#include "config.h"
#include "logging.h"

#include "devHeadTracker.h"

#include "QMC5883LCompass.h"
#include "IMUBase.h"
#include "ICMSeries.h"
#include "MPU6050.h"
#include "QMI8658C.h"
#include "Fusion.h"

static HeadTrackerState ht_state = STATE_ERROR;
static IMUBase *imu;
static QMC5883LCompass compass;
static bool hasCompass = false;
static FusionAhrs ahrs;

static float orientation[3] = {0.0, 0.0, 0.0};
static FusionEuler euler;
static float rollHome = 0, pitchHome = 0, yawHome = 0;
static int calibrationData[3][2];
static uint32_t cal_started;


static void initialize()
{
    Wire.setClock(400000);
    Wire.begin(PIN_SDA, PIN_SCL);

    // Compass init first
    compass.init();
    hasCompass = compass.readChipId() == 0xFF;
	if (hasCompass) compass.setMode(0x01,0x08,0x10,0X00); // continuous, 100Hz, 8G, 512 over sample

    // Initializing the IMU
    imu = new ICMSeries();
    if (!imu->initialize())
    {
        delete imu;
        imu = new MPU6050();
        if (!imu->initialize()) {
            delete imu;
            imu = new QMI8658C();
            if (!imu->initialize()) {
                delete imu;
                ht_state = STATE_ERROR;
                return;
            }
        }
    }

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
    if (hasCompass)
    {
        int (*cal)[3][2] = config.GetCompassCalibration();
        compass.setCalibration((*cal)[0][0],(*cal)[0][1],(*cal)[1][0],(*cal)[1][1],(*cal)[2][0],(*cal)[2][1]);
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

            FusionVector m FUSION_VECTOR_ZERO;
            if (hasCompass)
            {
                compass.read();

                m.axis.x = compass.getX();
                m.axis.y = compass.getY();
                m.axis.z = compass.getZ();
                rotate(m.array, orientation);
            }

            // Calculate delta time (in seconds) to account for gyroscope sample clock error
            const clock_t timestamp = micros();
            static clock_t previousTimestamp;
            const float deltaTime = (float) (timestamp - previousTimestamp) / (float) 1000000;
            previousTimestamp = timestamp;

            FusionAhrsUpdate(&ahrs, g, a, m, deltaTime);

            euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
            euler.angle.roll -= rollHome;
            euler.angle.pitch -= pitchHome;
            euler.angle.yaw -= yawHome;
            break;
        }

        case STATE_COMPASS_CALIBRATING: {
            if ((millis() - cal_started) < 10000) {
                compass.read();

                int x = compass.getX();
                int y = compass.getY();
                int z = compass.getZ();

                if (x < calibrationData[0][0]) {
                    calibrationData[0][0] = x;
                }
                if (x > calibrationData[0][1]) {
                    calibrationData[0][1] = x;
                }

                if (y < calibrationData[1][0]) {
                    calibrationData[1][0] = y;
                }
                if (y > calibrationData[1][1]) {
                    calibrationData[1][1] = y;
                }

                if (z < calibrationData[2][0]) {
                    calibrationData[2][0] = z;
                }
                if (z > calibrationData[2][1]) {
                    calibrationData[2][1] = z;
                }
            } else {
                compass.setCalibration(
                        calibrationData[0][0],
                        calibrationData[0][1],
                        calibrationData[1][0],
                        calibrationData[1][1],
                        calibrationData[2][0],
                        calibrationData[2][1]
                );
                config.SetCompassCalibration(calibrationData);
                config.Commit();
                ht_state = STATE_RUNNING;
            }
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

void startCompassCalibration()
{
    if (hasCompass) {
        compass.clearCalibration();
        calibrationData[0][0] = calibrationData[0][1] = compass.getX();
        calibrationData[1][0] = calibrationData[1][1] = compass.getY();
        calibrationData[2][0] = calibrationData[2][1] = compass.getZ();

        cal_started = millis();
        ht_state = STATE_COMPASS_CALIBRATING;
    }
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
    *yaw = -euler.angle.yaw;
    *pitch = euler.angle.pitch;
    *roll = -euler.angle.roll;
}

device_t HeadTracker_device = {
    .initialize = initialize,
    .start = start,
    .event = nullptr,
    .timeout = timeout
};

#endif