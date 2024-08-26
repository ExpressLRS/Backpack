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

static volatile bool irq_received;

static IRAM_ATTR void irq_handler() {
    irq_received = true;
}

bool IMU::initialize() {
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
        return true;
    }
    delete imu;

    auto *mpu6050 = new MPU6050(MPU6050_DEFAULT_ADDRESS, &Wire);
    mpu6050->initialize();
    if (mpu6050->testConnection()) {
        DBGLN("Found MPU6050");
        pinMode(PIN_INT, INPUT_PULLUP);
        attachInterrupt(PIN_INT, irq_handler, RISING);
        mpu6050->setSleepEnabled(false);
        mpu6050->setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
        mpu6050->setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
        mpu6050->setDLPFMode(MPU6050_DLPF_BW_188);
        mpu6050->setRate(9); // 1000/(1+79) = 100Hz
        mpu6050->setInterruptMode(false);
        mpu6050->setInterruptDrive(false);
        mpu6050->setInterruptLatch(false);
        mpu6050->setInterruptLatchClear(false);
        mpu6050->setFSyncInterruptLevel(false);
        mpu6050->setFSyncInterruptEnabled(false);
        mpu6050->setI2CBypassEnabled(true);
        mpu6050->setClockOutputEnabled(false);
        mpu6050->setIntDataReadyEnabled(true);
        mpu6050->setFIFOEnabled(true);
        mpu6050->resetFIFO();
        mpu6050->setXGyroFIFOEnabled(true);
        mpu6050->setYGyroFIFOEnabled(true);
        mpu6050->setZGyroFIFOEnabled(true);
        mpu6050->setAccelFIFOEnabled(true);

        sampleRate = 100;

        aRes = 16.0/32768;
        gRes = 2000.0/32768;

        device = mpu6050;
        deviceType = IMU_MPU6050;
        return true;
    }
    delete mpu6050;

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
            uint8_t values[12];
            ((MPU6050 *)device)->GetCurrentFIFOPacket(values, 12);
            ((MPU6050 *)device)->getIntDataReadyStatus(); // Clears the interrupt status
            accel.axis.x =  (int16_t)((values[0] << 8) | values[1]) * aRes;
            accel.axis.y =  (int16_t)((values[2] << 8) | values[3]) * aRes;
            accel.axis.z =  (int16_t)((values[4] << 8) | values[5]) * aRes;
            gyro.axis.x =  (int16_t)((values[6] << 8) | values[7]) * gRes;
            gyro.axis.y =  (int16_t)((values[8] << 8) | values[9]) * gRes;
            gyro.axis.z =  (int16_t)((values[10] << 8) | values[11]) * gRes;
            break;
        }
    }
    return true;
}
