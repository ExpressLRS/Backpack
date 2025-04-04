#include "FusionMath.h"
#include "IMUBase.h"

class MPU6050 : public IMUBase {
public:
    MPU6050() : IMUBase(0x68) {}

    bool initialize();

protected:
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);
};