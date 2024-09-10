#ifndef BACKPACK_ICM_H
#define BACKPACK_ICM_H

#include "FusionMath.h"

class ICMSeries {
public:
    bool initialize();
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);

private:
    static void writeRegister(uint8_t reg, uint8_t val);
    static uint8_t readRegister(uint8_t reg);
    static uint8_t readBuffer(uint8_t reg, uint8_t *buffer, int length);

    static void writeMem1Register(uint8_t reg, uint8_t val);
};

#endif //BACKPACK_ICM_H
