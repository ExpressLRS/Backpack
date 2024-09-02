#ifndef BACKPACK_IMU_H
#define BACKPACK_IMU_H

#include "Fusion.h"

class IMU {
public:
    bool initialize();
    bool readIMUData(FusionVector &accel, FusionVector &gyro);
    int getSampleRate() const { return sampleRate; }

    void BeginCalibration();
    bool UpdateCalibration(FusionVector &g);

    void SetCalibration(float (*calibration)[3]);

    const float *GetCalibration();

private:
    int sampleRate = 0;
};

#endif //BACKPACK_IMU_H
