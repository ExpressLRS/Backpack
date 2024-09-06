#ifndef BACKPACK_IMU_H
#define BACKPACK_IMU_H

#include "Fusion.h"

class IMU {
public:
    bool initialize();
    static bool readIMUData(FusionVector &accel, FusionVector &gyro);
    int getSampleRate() const { return sampleRate; }

    static void BeginCalibration();
    static bool UpdateCalibration(FusionVector &g);

    static void SetCalibration(float (*calibration)[3]);

    static const float *GetCalibration();

private:
    int sampleRate = 0;
};

#endif //BACKPACK_IMU_H
