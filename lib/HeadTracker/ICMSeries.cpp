#include "Arduino.h"
#include "logging.h"
#include "Wire.h"
#include "ICMSeries.h"

#define ADDR 0x68
#define WHO_AM_I 0x75
#define REVISION_ID 0x01
#define ACCEL_X1 0x0B
#define INT_STATUS_DRDY 0x39

#define ARES (16.0 / 32768)
#define GRES (2000.0 / 32768)

void ICMSeries::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void ICMSeries::writeMem1Register(uint8_t reg, uint8_t val) {
    uint32_t t1 = millis();
    writeRegister(0x1F, 0x10);
    while (!(readRegister(0x00) & 0x08) && (millis() - t1 < 1000));
    writeRegister(0x7A, reg);
    writeRegister(0x7B, val);
    writeRegister(0x1F, 0x00);
}

uint8_t ICMSeries::readRegister(uint8_t reg) {
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, 1);
    return Wire.read();
}

uint8_t ICMSeries::readBuffer(uint8_t reg, uint8_t *buffer, int length) {
    uint32_t t1 = millis();
    Wire.beginTransmission(ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(ADDR, length);
    int count = 0;
    while (Wire.available() && (millis() - t1 < 1000) && count < length) {
        buffer[count++] = Wire.read();
    }
    return count;
}

bool ICMSeries::initialize() {
    Wire.setTimeout(1000);
    Wire.setClock(1000000);

    int whoami = readRegister(WHO_AM_I);
    DBGLN("WHO_AM_I %x", whoami);
    if (whoami != 0x60 && whoami != 0x61 && whoami != 0x67 && whoami != 0x69 && whoami != 0x64) {
        return false;
    }

    /* Reset device */
    /* Ensure BLK_SEL_R and BLK_SEL_W are set to 0 */
    writeRegister(0x7C, 0);
    writeRegister(0x79, 0);
    writeRegister(0x02, 0x10);
    delay(1);
    writeRegister(0x01, 0x04);
    writeRegister(0x36, 0x41);

    /* Clear the reset done interrupt */
    readRegister(0x3A);

    writeMem1Register(0x03, 0x00);

    writeRegister(0x06, 0x03);
    writeRegister(0x35, 0x50);
    writeMem1Register(0x00, 0x09);
    writeRegister(0x29, 0x01);

    writeMem1Register(0x02, 0x01);
    writeMem1Register(0x01, 0x24);

    writeRegister(0x2B, 0x1C);
    writeRegister(0x2C, 0x0F);
    writeMem1Register(0x2F, 0xF8);

    writeRegister(0x21, 9); // 16G, 100Hz
    writeRegister(0x20, 9); // 2000dps, 100Hz
    writeRegister(0x1F, 0x0F); // Low noise mode for Accel/Gyro

    return true;
}

bool ICMSeries::getDataFromRegisters(FusionVector &accel, FusionVector &gyro) {
    uint8_t values[12];

    // return if NOT ready
    if (!(readRegister(INT_STATUS_DRDY) & 1)) {
        return false;
    }

    // read teh accel and gyro data
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
