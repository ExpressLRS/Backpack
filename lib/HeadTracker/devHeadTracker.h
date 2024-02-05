#pragma once

#include "device.h"

extern device_t HeadTracker_device;

typedef enum {
    STATE_ERROR,
    STATE_RUNNING,
    STATE_COMPASS_CALIBRATING,
    STATE_IMU_CALIBRATING
} HeadTrackerState;

typedef enum {
    PHASE_BEGIN,
    PHASE_VRX_FLAT,
    PHASE_BOARD_FLAT
} OrientationPhase;

void startCompassCalibration();
void startIMUCalibration();
void setupBoardOrientation(OrientationPhase phase);
HeadTrackerState getHeadTrackerState();

void resetCenter();
void getEuler(float *yaw, float *pitch, float *roll);
