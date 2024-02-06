#include <Arduino.h>
#include "common.h"

#if defined(PIN_SCL)
#include <Wire.h>

#include "devHeadTracker.h"

#include "config.h"
#include "logging.h"

#include "ICM42670P.h"
#include "QMC5883LCompass.h"

#include "Fusion.h"

static HeadTrackerState ht_state = STATE_ERROR;
static ICM42670P IMU(Wire,0);
static QMC5883LCompass compass;
static FusionAhrs ahrs;

static float aRes;
static float gRes;
static volatile uint8_t irq_received = 0;

static float orientation[3] = {0.0, 0.0, 0.0};
static FusionEuler euler;
static float rollHome = 0, pitchHome = 0, yawHome = 0;
static int calibrationData[3][2];
static uint32_t cal_started;


static IRAM_ATTR void irq_handler(void) {
  irq_received = 1;
}

static void initialize()
{
    Wire.begin(PIN_SDA, PIN_SCL);

    // Compass init first
    compass.init();
	compass.setMode(0x01,0x08,0x10,0X00); // continuous, 100Hz, 8G, 512 over sample

    // Initializing the ICM42607C
    int ret = IMU.begin();
    if (ret != 0) {
        DBGLN("ICM42607C initialization failed: %d", ret);
        return;
    }
    if ((ret = IMU.enableDataInterrupt(9, irq_handler))) {
        DBGLN("Interrupt enable failed: %d");
        return;
    }
    // Accel ODR = 100 Hz and Full Scale Range = 16G
    IMU.startAccel(100, 16);
    aRes = 16.0/32768;
    // Gyro ODR = 100 Hz and Full Scale Range = 2000 dps
    IMU.startGyro(100, 2000);
    gRes = 2000.0/32768;

    FusionAhrsInitialise(&ahrs);
    // Set AHRS algorithm settings
    const FusionAhrsSettings settings = {
            .convention = FusionConventionNwu,
            .gain = 0.5f,
            .gyroscopeRange = 2000.0f, /* replace this with actual gyroscope range in degrees/s */
            .accelerationRejection = 10.0f,
            .magneticRejection = 10.0f,
            .recoveryTriggerPeriod = 5 * 100, /* 5 seconds */
    };
    FusionAhrsSetSettings(&ahrs, &settings);
    ht_state = STATE_RUNNING;
}

static int start()
{
    if (ht_state == STATE_ERROR)
    {
        return DURATION_NEVER;
    }
    int (*cal)[3][2] = config.GetCompassCalibration();
    compass.setCalibration((*cal)[0][0],(*cal)[0][1],(*cal)[1][0],(*cal)[1][1],(*cal)[2][0],(*cal)[2][1]);
    //TODO: load gyro/accel calibration from settings
    memcpy(orientation, *config.GetBoardOrientation(), sizeof(orientation));
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
    if(!irq_received)
    {
        return DURATION_IMMEDIATELY;
    }
    irq_received = 0;

    inv_imu_sensor_event_t imu_event;
    IMU.getDataFromRegisters(&imu_event);

    switch(ht_state)
    {
    case STATE_RUNNING:
        {
            FusionVector a;
            a.axis.x =  imu_event.accel[0] * aRes;
            a.axis.y =  imu_event.accel[1] * aRes;
            a.axis.z =  imu_event.accel[2] * aRes;
            rotate(a.array, orientation);

            FusionVector g;
            g.axis.x =  imu_event.gyro[0] * gRes;
            g.axis.y =  imu_event.gyro[1] * gRes;
            g.axis.z =  imu_event.gyro[2] * gRes;
            rotate(g.array, orientation);

            compass.read();

            FusionVector m;
            m.axis.x = compass.getX();
            m.axis.y = compass.getY();
            m.axis.z = compass.getZ();
            rotate(m.array, orientation);

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
        }
        break;

    case STATE_COMPASS_CALIBRATING:
        {
            if ((millis() - cal_started) < 10000)
            {
                compass.read();

                int x = compass.getX();
                int y = compass.getY();
                int z = compass.getZ();

                if(x < calibrationData[0][0]) {
                    calibrationData[0][0] = x;
                }
                if(x > calibrationData[0][1]) {
                    calibrationData[0][1] = x;
                }

                if(y < calibrationData[1][0]) {
                    calibrationData[1][0] = y;
                }
                if(y > calibrationData[1][1]) {
                    calibrationData[1][1] = y;
                }

                if(z < calibrationData[2][0]) {
                    calibrationData[2][0] = z;
                }
                if(z > calibrationData[2][1]) {
                    calibrationData[2][1] = z;
                }
            }
            else
            {
                compass.setCalibration(
                    calibrationData[0][0],
                    calibrationData[0][1],
                    calibrationData[1][0],
                    calibrationData[1][1],
                    calibrationData[2][0],
                    calibrationData[2][1]
                );
                config.SetCompassCalibration(calibrationData);
                ht_state = STATE_RUNNING;
            }
        }
        break;

    case STATE_IMU_CALIBRATING:
        {

        }
        break;
    }

    return DURATION_IMMEDIATELY;
}

void startCompassCalibration()
{
    compass.clearCalibration();
  	calibrationData[0][0] = calibrationData[0][1] = compass.getX();
  	calibrationData[1][0] = calibrationData[1][1] = compass.getY();
  	calibrationData[2][0] = calibrationData[2][1] = compass.getZ();

	cal_started = millis();
    ht_state = STATE_COMPASS_CALIBRATING;
}

void startIMUCalibration()
{

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
    config.SetBoardOrientation(orientation);
    config.Commit();
}

void setBoardOrientation(int xAngle, int yAngle, int zAngle)
{
    orientation[0] = yAngle * DEG_TO_RAD;
    orientation[1] = xAngle * DEG_TO_RAD;
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
    *yaw = euler.angle.yaw;
    *pitch = -euler.angle.pitch;
    *roll = -euler.angle.roll;
}

device_t HeadTracker_device = {
    .initialize = initialize,
    .start = start,
    .event = NULL,
    .timeout = timeout
};

#endif