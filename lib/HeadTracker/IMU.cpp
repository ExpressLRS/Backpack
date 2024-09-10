#if defined(HAS_HEADTRACKING)
#include "Arduino.h"
#include "Wire.h"
#include "logging.h"

#include "IMU.h"

#include "ICMSeries.h"
#include "MPU6050.h"
#include "QMI8658C.h"

static enum {
    IMU_ICM42670P,
    IMU_MPU6050,
    IMU_QMI8658C
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
    auto *mpu6050 = new MPU6050();
    if (mpu6050->initialize()) {
        DBGLN("Found MPU6050");
        pinMode(PIN_INT, INPUT_PULLUP);
        attachInterrupt(PIN_INT, irq_handler, RISING);
        sampleRate = 100;

        aRes = 16.0/32768;
        gRes = 2000.0/32768;

        device = mpu6050;
        deviceType = IMU_MPU6050;
        return true;
    }
    delete mpu6050;

    DBGLN("Try QMI8658C");
    auto *qmi8658c = new QMI8658C();
    if (qmi8658c->initialize()) {
        DBGLN("Found QMI8658C");
        pinMode(PIN_INT, INPUT_PULLUP);
        attachInterrupt(PIN_INT, irq_handler, RISING);
        sampleRate = 100;

        aRes = 16.0/32768;
        gRes = 2000.0/32768;

        device = qmi8658c;
        deviceType = IMU_QMI8658C;
        return true;
    }
    delete qmi8658c;

    DBGLN("Try ICM42670P");
    auto *imu = new ICMSeries();
    if (imu->initialize()) {
        DBGLN("Found ICM42670P");
        pinMode(PIN_INT, INPUT_PULLUP);
        attachInterrupt(PIN_INT, irq_handler, RISING);
        sampleRate = 100;

        aRes = 16.0/32768;
        gRes = 2000.0/32768;

        device = imu;
        deviceType = IMU_ICM42670P;
        return true;
    }
    delete imu;

    DBGLN("IMU initialization failed: No IMU");
    return false;
}

bool IMU::readIMUData(FusionVector &accel, FusionVector &gyro) {
    bool hasData = irq_received;
    if (!hasData) return false;

    switch (deviceType) {
        case IMU_ICM42670P: {
            if (!((ICMSeries *) device)->getDataFromRegisters(accel, gyro))
                return false;
            break;
        }

        case IMU_MPU6050: {
            if (!((MPU6050 *) device)->getDataFromRegisters(accel, gyro))
                return false;
            break;
        }

        case IMU_QMI8658C: {
            if (!((QMI8658C *) device)->getDataFromRegisters(accel, gyro))
                return false;
            break;
        }
    }
    gyro.axis.x += imuCalibration[0];
    gyro.axis.y += imuCalibration[1];
    gyro.axis.z += imuCalibration[2];
    irq_received = false;
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