#include <Arduino.h>
#include "common.h"

#if defined(PIN_SCL)
#include <Wire.h>

#include "devHeadTracker.h"

#include "logging.h"

#include "ICM42670P.h"
#include "QMC5883LCompass.h"

#include "Fusion.h"

static ICM42670P IMU(Wire,0);
static QMC5883LCompass compass;
static FusionAhrs ahrs;

static float aRes;
static float gRes;
static float mRes;
static volatile uint8_t irq_received = 0;

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
        while(1);
    }
    if ((ret = IMU.enableDataInterrupt(9, irq_handler))) {
        DBGLN("Interrupt enable failed: %d");
        while(1);
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
}

static int start()
{
    //TODO: load compass calibration settings from config
    compass.setCalibrationOffsets(0,0,0);
    compass.setCalibrationScales(0,0,0);
    //TODO: load gyro/accel calibration from settings
    //TODO: load orientation from settings
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

static float orientation[3] = {0.0, 0.0, 0.0};
static FusionEuler euler;
static float rollHome = 0, pitchHome = 0, yawHome = 0;

static int timeout()
{
    static boolean running = true;
    static int counter = 0;

    if(irq_received) {
        irq_received = 0;

        inv_imu_sensor_event_t imu_event;
        IMU.getDataFromRegisters(&imu_event);

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
    return DURATION_IMMEDIATELY;
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

void setupBoardOrientation(OrientationPhase phase)
{
    static FusionEuler vrx_flat;

    switch(phase)
    {
        case PHASE_BEGIN:
            orientation[0] = 0;
            orientation[1] = 0;
            orientation[2] = 0;
            rollHome = 0;
            pitchHome = 0;
            yawHome = 0;
            FusionAhrsReset(&ahrs);
            break;
        case PHASE_VRX_FLAT:
            vrx_flat = euler;
            break;
        case PHASE_BOARD_FLAT:
            orientation[0] = (vrx_flat.angle.roll - euler.angle.roll)*DEG_TO_RAD;
            orientation[1] = (vrx_flat.angle.pitch - euler.angle.pitch)*DEG_TO_RAD;
            orientation[2] = (vrx_flat.angle.yaw - euler.angle.yaw)*DEG_TO_RAD;
            FusionAhrsReset(&ahrs);
            // TODO: save orientation in settings
            break;
    }
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