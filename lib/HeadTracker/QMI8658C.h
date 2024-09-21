#include "IMUBase.h"

class QMI8658C : public IMUBase {
public:
    QMI8658C() : IMUBase(0x6B) {}

    bool initialize();

protected:
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);

private:
    int writeCommand(uint8_t cmd);
};