#include "Arduino.h"
#include "logging.h"
#include "Wire.h"
#include "QMI8658C.h"

#define WHO_AM_I 0x00
#define REVISION_ID 0x01
#define ACCEL_X_L 0x35

#define ARES (16.0 / 32768)
#define GRES (1024.0 / 32768)

int QMI8658C::writeCommand(uint8_t cmd)
{
    int      val;
    uint32_t startMillis;

    writeRegister(0x0A, cmd);
    startMillis = millis();
    do {
        val = readRegister(0x2D);
        if (millis() - startMillis > 1000) {
            DBGLN("Wait for ctrl9 command done time out : %x val: %x", cmd, val);
            return -1;
        }
    } while (!(val & 0x80));

    writeRegister(0x0A, 0x00);

    startMillis = millis();
    do {
        val = readRegister(0x2D);
        if (millis() - startMillis > 1000) {
            DBGLN("Clear ctrl9 command done flag time out : %x val: %x", cmd, val);
            return -1;
        }
    } while ((val & 0x80));

    return 0;
}

bool QMI8658C::initialize() {
    writeRegister(0x60, 0xB0);          // Reset
    while(!(readRegister(0x4D) & 0x80));

    writeRegister(0x02, 0b01100000);    // enable auto-increment, BIG endian

    if (readRegister(WHO_AM_I) != 0x05) {
        return false;
    }

    if (readRegister(REVISION_ID) != 0x7C) {
        DBGLN("REVISION_ID failed");
        return false;
    }

    writeRegister(0x03, 0b00110011);    // 16G, 896.8 ODR
    writeRegister(0x06, 0b00000111);    // both LPF enabled @ 13.37% ODR
    writeRegister(0x03, 0b10110011);    // 16G, 896.8 ODR + self test
    writeRegister(0x04, 0b01100011);    // 1024dps, 896.8 ODR
    writeRegister(0x06, 0b01110111);    // both LPF enabled @ 13.37% ODR
    writeRegister(0x04, 0b11100011);    // 1024dps, 896.8 ODR + self test
    writeRegister(0x08, 0b10000011);    // SyncSample, G+A enabled

    // disable AHB clock gating
    writeRegister(0x0B, 0x01);
    writeCommand(0x12);                 // times out, but meh!

    writeRegister(0x02, 0b01110000);    // enable auto-increment, BE, INT2 enable

    if ((readRegister(0x08) & 3) != 3) {
        DBGLN("Check mode failed");
        return false;
    }

    gyroRange = 1024.0;
    gRes = GRES;

    return true;
}

bool QMI8658C::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[12];

    // Wait for data ready
    uint32_t t1 = millis();
    do {
        readBuffer(0x2D, values, 3);
    } while ((values[0] & 3) != 3 && (millis() - t1 < 1000));
    if ((values[0] & 3) != 3) {
        return false;
    }

    // read teh accel and gyro data
    readBuffer(ACCEL_X_L, values, 12);

    // map into Gs and dps
    accel.axis.x =  (int16_t)((values[1] << 8) | values[0]) * ARES;
    accel.axis.y =  (int16_t)((values[3] << 8) | values[2]) * ARES;
    accel.axis.z =  (int16_t)((values[5] << 8) | values[4]) * ARES;
    gyro.axis.x =  (int16_t)((values[7] << 8) | values[6]) * GRES;
    gyro.axis.y =  (int16_t)((values[9] << 8) | values[8]) * GRES;
    gyro.axis.z =  (int16_t)((values[11] << 8) | values[10]) * GRES;
    return true;
}
