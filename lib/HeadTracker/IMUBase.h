#ifndef BACKPACK_IMUBASE_H
#define BACKPACK_IMUBASE_H

#include "FusionMath.h"

class IMUBase {
public:
    explicit IMUBase(int _address) : address(_address) {}

    virtual bool initialize() = 0;
    virtual bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro) = 0;

protected:
    int address;

    void writeRegister(uint8_t reg, uint8_t val);
    uint8_t readRegister(uint8_t reg);

    uint8_t readBuffer(uint8_t reg, uint8_t *buffer, int length);
};

#endif //BACKPACK_IMUBASE_H
