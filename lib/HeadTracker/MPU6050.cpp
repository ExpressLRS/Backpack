#include "Arduino.h"
#include "Wire.h"
#include "MPU6050.h"

#define WHO_AM_I 0x75
#define ACCEL_X_H 0x3B

#define ARES (16.0 / 32768)
#define GRES (2000.0 / 32768)

bool MPU6050::initialize() {
    static uint8_t INIT_DATA[] = {
        0x6B, 0x01,
        0x1B, 0x18,
        0x1C, 0x18,
        0x1A, 0x01,
        0x19, 0x09,
        0x37, 0x02,
        0x38, 0x01,
        0x23, 0x78
    };

    if (((readRegister(WHO_AM_I) >> 1) & 0x3F) != 0x34) {
        return false;
    }
    for (uint32_t i=0 ; i<sizeof(INIT_DATA) ; i+=2) {
        writeRegister(INIT_DATA[i], INIT_DATA[i+1]);
    }

    setInterruptHandler(PIN_INT);
    gyroRange = 2000.0;
    gRes = 2000.0 / 32768;

    return true;
}

bool MPU6050::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[14];

    readBuffer(ACCEL_X_H, values, 14);
    accel.axis.x =  (int16_t)((values[0] << 8) | values[1]) * ARES;
    accel.axis.y =  (int16_t)((values[2] << 8) | values[3]) * ARES;
    accel.axis.z =  (int16_t)((values[4] << 8) | values[5]) * ARES;
    gyro.axis.x =  (int16_t)((values[8] << 8) | values[9]) * GRES;
    gyro.axis.y =  (int16_t)((values[10] << 8) | values[11]) * GRES;
    gyro.axis.z =  (int16_t)((values[12] << 8) | values[13]) * GRES;
    return true;
}
