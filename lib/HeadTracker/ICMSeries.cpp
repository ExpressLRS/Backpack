#include "Arduino.h"
#include "Wire.h"
#include "ICMSeries.h"

#define WHO_AM_I 0x75
#define REVISION_ID 0x01
#define ACCEL_X1 0x0B
#define INT_STATUS_DRDY 0x39

#define ARES (16.0 / 32768)
#define GRES (2000.0 / 32768)

bool ICMSeries::initialize() {
    // Make sure it's one of the supported ICM IMUs
    // ICM42607P, ICM42607C, ICM42670P, ICM42670S, ICM42670T
    int whoami = readRegister(WHO_AM_I);
    if (whoami != 0x60 && whoami != 0x61 && whoami != 0x67 && whoami != 0x69 && whoami != 0x64) {
        return false;
    }

    /* Reset device */
    writeRegister(0x7C, 0);         // BLK_SEL_R
    writeRegister(0x79, 0);         // BLK_SEL_W
    writeRegister(0x02, 0x10);      // SIGNAL_PATH_RESET (soft reset device)
    delay(1);
    writeRegister(0x01, 0x04);      // DEVICE_CONFIG
    writeRegister(0x36, 0x41);      // INF_CONFIG1, i2c mode, PLL clock

    readRegister(0x3A);             // Clear the reset done interrupt

    writeRegister(0x06, 0x03);      // INT_CONFIG, interrupt push-pull, active high
    writeRegister(0x2B, 0x08);      // INT_SOURCE0, drdy

    writeRegister(0x21, 9);         // 16G, 100Hz
    writeRegister(0x20, 9);         // 2000dps, 100Hz
    writeRegister(0x1F, 0x0F);      // Low noise mode for Accel/Gyro

    gyroRange = 2000.0;
    gRes = GRES;

    return true;
}

bool ICMSeries::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[12];

    // return if NOT ready
    if (!(readRegister(INT_STATUS_DRDY) & 1)) {
        return false;
    }

    // read the accel and gyro data
    readBuffer(ACCEL_X1, values, 12);

    // map into Gs and dps
    accel.axis.x =  (int16_t)((values[0] << 8) | values[1]) * ARES;
    accel.axis.y =  (int16_t)((values[2] << 8) | values[3]) * ARES;
    accel.axis.z =  (int16_t)((values[4] << 8) | values[5]) * ARES;
    gyro.axis.x =  (int16_t)((values[6] << 8) | values[7]) * GRES;
    gyro.axis.y =  (int16_t)((values[8] << 8) | values[9]) * GRES;
    gyro.axis.z =  (int16_t)((values[10] << 8) | values[11]) * GRES;
    return true;
}
