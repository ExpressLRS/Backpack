#include "FusionMath.h"
#include "IMUBase.h"

class MPU6050 : IMUBase {
public:
    MPU6050() : IMUBase(0x68) {}

    bool initialize();
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);
};