#pragma once

#include "device.h"

extern device_t HeadTracker_device;

typedef enum {
    STATE_ERROR,
    STATE_RUNNING,
    STATE_COMPASS_CALIBRATING,
    STATE_IMU_CALIBRATING
} HeadTrackerState;

void startCompassCalibration();
void startIMUCalibration();
void resetBoardOrientation();
void saveBoardOrientation();
void setBoardOrientation(int xAngle, int yAngle, int zAngle);
HeadTrackerState getHeadTrackerState();

void resetCenter();
void getEuler(float *yaw, float *pitch, float *roll);
