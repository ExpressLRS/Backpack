//
// Created by Paul Kendall on 25/08/2024.
//

#ifndef BACKPACK_IMU_H
#define BACKPACK_IMU_H

#include "Fusion.h"

class IMU {
public:
    bool initialize();
    bool readIMUData(FusionVector &accel, FusionVector &gyro);
    int getSampleRate() { return sampleRate; }

private:
    int sampleRate = 0;
};

#endif //BACKPACK_IMU_H
