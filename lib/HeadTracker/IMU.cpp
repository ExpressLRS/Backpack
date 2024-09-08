#if defined(HAS_HEADTRACKING)
#include "IMU.h"

#include "logging.h"

#include "ICM42670P.h"
#include "MPU6050.h"

static enum {
    IMU_ICM42670P,
    IMU_MPU6050
} deviceType;

static void *device;
static float aRes;
static float gRes;

// Calibration data
constexpr int Loops = 20;
static float imuCalibration[3];
static double kP, kI;
static int16_t eSample;
static float ITerm[3] = {0 ,0, 0};
static int c, L;

static volatile bool irq_received;

static IRAM_ATTR void irq_handler() {
    irq_received = true;
}

bool IMU::initialize() {
    Wire.setClock(400000);
    DBGLN("Try MPU6050");
    auto *mpu6050 = new MPU6050(MPU6050_DEFAULT_ADDRESS, &Wire);
    if (mpu6050->testConnection()) {
        DBGLN("Found MPU6050");
        pinMode(PIN_INT, INPUT_PULLUP);
        attachInterrupt(PIN_INT, irq_handler, RISING);
        mpu6050->setSleepEnabled(false);
        mpu6050->setClockSource(1);
        mpu6050->setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
        mpu6050->setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
        mpu6050->setDLPFMode(MPU6050_DLPF_BW_188);
        mpu6050->setRate(9); // 1000/(1+79) = 100Hz
        mpu6050->setInterruptMode(false);
        mpu6050->setInterruptDrive(false);
        mpu6050->setInterruptLatch(false);
        mpu6050->setInterruptLatchClear(true);
        mpu6050->setFSyncInterruptLevel(false);
        mpu6050->setFSyncInterruptEnabled(false);
        mpu6050->setI2CBypassEnabled(true);
        mpu6050->setClockOutputEnabled(false);
        mpu6050->setIntDataReadyEnabled(true);
        sampleRate = 100;

        aRes = 16.0/32768;
        gRes = 2000.0/32768;

        device = mpu6050;
        deviceType = IMU_MPU6050;
        return true;
    }
    delete mpu6050;

    DBGLN("Try ICM42670P");
    auto *imu = new ICM42670P(Wire, false);
    int ret = imu->begin();
    if (ret == 0) {
        if ((ret = imu->enableDataInterrupt(PIN_INT, irq_handler))) {
            DBGLN("Interrupt enable failed: %d");
            return false;
        }

        sampleRate = 100;

        // Accel ODR = 100 Hz and Full Scale Range = 16G
        imu->startAccel(100, 16);
        aRes = 16.0/32768;
        // Gyro ODR = 100 Hz and Full Scale Range = 2000 dps
        imu->startGyro(100, 2000);
        gRes = 2000.0/32768;

        device = imu;
        deviceType = IMU_ICM42670P;
        DBGLN("Found ICM42670P");
        return true;
    }
    delete imu;

    DBGLN("IMU initialization failed: No IMU");
    return false;
}

bool IMU::readIMUData(FusionVector &accel, FusionVector &gyro) {
    bool hasData = irq_received;
    if (!hasData) return false;
    irq_received = false;

    switch (deviceType) {
        case IMU_ICM42670P: {
            inv_imu_sensor_event_t imu_event;
            ((ICM42670P *)device)->getDataFromRegisters(&imu_event);
            accel.axis.x =  imu_event.accel[0] * aRes;
            accel.axis.y =  imu_event.accel[1] * aRes;
            accel.axis.z =  imu_event.accel[2] * aRes;
            gyro.axis.x =  imu_event.gyro[0] * gRes;
            gyro.axis.y =  imu_event.gyro[1] * gRes;
            gyro.axis.z =  imu_event.gyro[2] * gRes;
            break;
        }

        case IMU_MPU6050: {
            int16_t values[6];
            ((MPU6050 *)device)->getMotion6(&values[0], &values[1], &values[2], &values[3], &values[4], &values[5]);
            accel.axis.x =  values[0] * aRes;
            accel.axis.y =  values[1] * aRes;
            accel.axis.z =  values[2] * aRes;
            gyro.axis.x =  values[3] * gRes;
            gyro.axis.y =  values[4] * gRes;
            gyro.axis.z =  values[5] * gRes;
            break;
        }
    }
    gyro.axis.x += imuCalibration[0];
    gyro.axis.y += imuCalibration[1];
    gyro.axis.z += imuCalibration[2];
    return true;
}

void IMU::BeginCalibration()
{
    kP = 0.5;
    kI = 150;
    float x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
    kP *= x;
    kI *= x;

    c = 0;
    L = 0;
    eSample = 0;
    ITerm[0] = 0;
    ITerm[1] = 0;
    ITerm[2] = 0;
}

bool IMU::UpdateCalibration(FusionVector &g)
{
    float eSum = 0;
    float Data, Error, PTerm;

    ITerm[0] = imuCalibration[0];
    ITerm[1] = imuCalibration[1];
    ITerm[2] = imuCalibration[2];

    Error = -g.axis.x;
    eSum += abs(g.axis.x);
    PTerm = kP * Error;
    ITerm[0] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[0];	//Compute PID Output
    imuCalibration[0] = Data;

    Error = -g.axis.y;
    eSum += abs(g.axis.y);
    PTerm = kP * Error;
    ITerm[1] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[1];	//Compute PID Output
    imuCalibration[1] = Data;

    Error = -g.axis.z;
    eSum += abs(g.axis.z);
    PTerm = kP * Error;
    ITerm[2] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[2];	//Compute PID Output
    imuCalibration[2] = Data;

    c++;
    if((c == 99) && eSum > 1000 * gRes) {						// Error is still to great to continue
        c = 0;
        DBGLN("Calibration retry %d", (int)eSum);
    }
    if(eSum < 5 * gRes) eSample++;	// Successfully found offsets prepare to  advance
    if(c == 100 || ((eSum < 100 * gRes) && (c > 10) && (eSample >= 10))) { 		// Advance to next Loop
        imuCalibration[0] = ITerm[0];
        imuCalibration[1] = ITerm[1];
        imuCalibration[2] = ITerm[2];
        kP *= .75;
        kI *= .75;
        c = 0;
        eSample = 0;
        L++;
        DBGLN("Finished loop iteration %d", L);
        Serial.printf("%6.2f %6.2f %6.2f\r\n", imuCalibration[0], imuCalibration[1], imuCalibration[2]);
    }
    return L == Loops;
}

void IMU::SetCalibration(float (*calibration)[3])
{
    memcpy(imuCalibration, calibration, sizeof(imuCalibration));
    for (float & i : imuCalibration) {
        if (i > 1000 || i < -1000 || isnan(i)) {
            i = 0;
        }
    }
}

const float *IMU::GetCalibration() {
    return imuCalibration;
}
#endif