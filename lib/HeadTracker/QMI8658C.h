#include "FusionMath.h"

class QMI8658C {
public:
    bool initialize();
    bool getDataFromRegisters(FusionVector &accel, FusionVector &gyro);

private:
    static void writeRegister(uint8_t reg, uint8_t val);
    static uint8_t readRegister(uint8_t reg);

    static int writeCommand(uint8_t cmd);

    static uint8_t readBuffer(uint8_t reg, uint8_t *buffer, int length);
};