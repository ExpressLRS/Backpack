#include "FusionMath.h"

class MPU6050 {
public:
    bool initialize();
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);
};