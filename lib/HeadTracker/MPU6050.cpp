#include "Arduino.h"
#include "MPU6050.h"

#define WHO_AM_I 0x75
#define ACCEL_X_H 0x3B

#define ARES (16.0 / 32768)
#define GRES (2000.0 / 32768)

bool MPU6050::initialize() {
    // First check if this an MPU6050
    if (((readRegister(WHO_AM_I) >> 1) & 0x3F) != 0x34) {
        return false;
    }

    writeRegister(0x6B, 0x01);  // PWR_MGMT_1, use X axis gyro as clock reference
    writeRegister(0x1B, 0x18);  // GYRO_CONFIG, 2000dps
    writeRegister(0x1C, 0x18);  // ACCEL_CONFIG, +-16g
    writeRegister(0x1A, 0x01);  // CONFIG, 184/188Hz DLPF
    writeRegister(0x19, 0x09);  // SMPRT_DIV, 1kHz / (1 + 9) = 100Hz
    writeRegister(0x37, 0x02);  // INT_PIN_CFG, I2C_BYPASS_EN
    writeRegister(0x38, 0x01);  // INT_ENABLE, DATA_RDY_EN
    writeRegister(0x23, 0x00);  // FIFO_EN, disabled

    gyroRange = 2000.0;
    gRes = GRES;

    return true;
}

bool MPU6050::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[14];

    // Read Accel, temp & gyro data (ignore the temp)
    readBuffer(ACCEL_X_H, values, 14);
    accel.axis.x =  (int16_t)((values[0] << 8) | values[1]) * ARES;
    accel.axis.y =  (int16_t)((values[2] << 8) | values[3]) * ARES;
    accel.axis.z =  (int16_t)((values[4] << 8) | values[5]) * ARES;
    gyro.axis.x =  (int16_t)((values[8] << 8) | values[9]) * GRES;
    gyro.axis.y =  (int16_t)((values[10] << 8) | values[11]) * GRES;
    gyro.axis.z =  (int16_t)((values[12] << 8) | values[13]) * GRES;
    return true;
}
