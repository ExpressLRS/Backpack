#ifndef BACKPACK_ICM_H
#define BACKPACK_ICM_H

#include "FusionMath.h"
#include "IMUBase.h"

class ICMSeries : IMUBase {
public:
    ICMSeries() : IMUBase(0x68) {}
    bool initialize();
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);

private:
    void writeMem1Register(uint8_t reg, uint8_t val);
};

#endif //BACKPACK_ICM_H
