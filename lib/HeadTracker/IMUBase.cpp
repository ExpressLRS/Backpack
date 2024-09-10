#include "IMUBase.h"

#include "Wire.h"

void IMUBase::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t IMUBase::readRegister(uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, 1);
    return Wire.read();
}

uint8_t IMUBase::readBuffer(uint8_t reg, uint8_t *buffer, int length) {
    uint32_t t1 = millis();
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, length);
    int count = 0;
    while (Wire.available() && (millis() - t1 < 1000) && count < length) {
        buffer[count++] = Wire.read();
    }
    return count;
}
