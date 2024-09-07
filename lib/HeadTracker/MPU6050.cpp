#include "Wire.h"
#include "MPU6050.h"

#define ADDR 0x68
#define WHO_AM_I 0x75
#define INT_STATUS 0x3A
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
    Wire.setTimeOut(1000);
    Wire.setTimeOut(1000);
    Wire.beginTransmission(ADDR);
    Wire.write(WHO_AM_I);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, 1);
    if (((Wire.read() >> 1) & 0x3F) != 0x34) {
        return false;
    }
    for (int i=0 ; i<sizeof(INIT_DATA) ; i+=2) {
        Wire.beginTransmission(ADDR);
        Wire.write(INIT_DATA[i]); // send address
        Wire.write(INIT_DATA[i+1]);
        uint8_t status = Wire.endTransmission();
        if (status != 0) {
            return false;
        }
    }
    return true;
}

bool MPU6050::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[14];

    Wire.beginTransmission(ADDR);
    Wire.write(ACCEL_X_H);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, 14);
    uint32_t t1 = millis();
    for (uint32_t count = 0; Wire.available() && (millis() - t1 < 1000) && count < 14; count++) {
        values[count] = Wire.read();
    }

    accel.axis.x =  (int16_t)((values[0] << 8) | values[1]) * ARES;
    accel.axis.y =  (int16_t)((values[2] << 8) | values[3]) * ARES;
    accel.axis.z =  (int16_t)((values[4] << 8) | values[5]) * ARES;
    gyro.axis.x =  (int16_t)((values[8] << 8) | values[9]) * GRES;
    gyro.axis.y =  (int16_t)((values[10] << 8) | values[11]) * GRES;
    gyro.axis.z =  (int16_t)((values[12] << 8) | values[13]) * GRES;
    return true;
}
