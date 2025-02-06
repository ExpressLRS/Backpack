#include "Arduino.h"
#include "logging.h"

#include "IMUBase.h"

#include "Wire.h"

// Calibration data
constexpr int Loops = 20;
static float imuCalibration[3];
static double kP, kI;
static int16_t eSample;
static float ITerm[3] = {0 ,0, 0};
static int c, L;

static volatile bool irq_received;

static IRAM_ATTR void irq_handler() {
    irq_received = true;
}

void IMUBase::setInterruptHandler(int pin) {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(pin, irq_handler, RISING);
}

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


bool IMUBase::readIMUData(FusionVector &accel, FusionVector &gyro) {
    bool hasData = irq_received;
    if (!hasData) return false;
    if (!getDataFromRegisters(accel, gyro)) {
        return false;
    }
    gyro.axis.x += imuCalibration[0];
    gyro.axis.y += imuCalibration[1];
    gyro.axis.z += imuCalibration[2];
    irq_received = false;
    return true;
}

void IMUBase::beginCalibration()
{
    kP = 0.5;
    kI = 150;
    float x = (100 - map(Loops, 1, 5, 20, 0)) * .01;
    kP *= x;
    kI *= x;

    c = 0;
    L = 0;
    eSample = 0;
    ITerm[0] = 0;
    ITerm[1] = 0;
    ITerm[2] = 0;
}

bool IMUBase::updateCalibration(FusionVector &g)
{
    float eSum = 0;
    float Data, Error, PTerm;

    ITerm[0] = imuCalibration[0];
    ITerm[1] = imuCalibration[1];
    ITerm[2] = imuCalibration[2];

    Error = -g.axis.x;
    eSum += abs(g.axis.x);
    PTerm = kP * Error;
    ITerm[0] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[0];	//Compute PID Output
    imuCalibration[0] = Data;

    Error = -g.axis.y;
    eSum += abs(g.axis.y);
    PTerm = kP * Error;
    ITerm[1] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[1];	//Compute PID Output
    imuCalibration[1] = Data;

    Error = -g.axis.z;
    eSum += abs(g.axis.z);
    PTerm = kP * Error;
    ITerm[2] += (Error * 0.001) * kI;				// Integral term 1000 Calculations a second = 0.001
    Data = PTerm + ITerm[2];	//Compute PID Output
    imuCalibration[2] = Data;

    c++;
    if((c == 99) && eSum > 1000 * gRes) {						// Error is still to great to continue
        c = 0;
        DBGLN("Calibration retry %d", (int)eSum);
    }
    if(eSum < 5 * gRes) eSample++;	// Successfully found offsets prepare to  advance
    if(c == 100 || ((eSum < 100 * gRes) && (c > 10) && (eSample >= 10))) { 		// Advance to next Loop
        imuCalibration[0] = ITerm[0];
        imuCalibration[1] = ITerm[1];
        imuCalibration[2] = ITerm[2];
        kP *= .75;
        kI *= .75;
        c = 0;
        eSample = 0;
        L++;
#ifdef DEBUG_LOG
        DBGLN("Finished loop iteration %d", L);
        Serial.printf("%6.2f %6.2f %6.2f\r\n", imuCalibration[0], imuCalibration[1], imuCalibration[2]);
#endif
    }
    return L == Loops;
}

void IMUBase::setCalibration(float (*calibration)[3])
{
    memcpy(imuCalibration, calibration, sizeof(imuCalibration));
    for (float & i : imuCalibration) {
        if (i > 1000 || i < -1000 || isnan(i)) {
            i = 0;
        }
    }
}

const float *IMUBase::getCalibration() {
    return imuCalibration;
}
