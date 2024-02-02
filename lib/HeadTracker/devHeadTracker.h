#pragma once

#include "device.h"

extern device_t HeadTracker_device;

typedef enum {
    PHASE_BEGIN,
    PHASE_VRX_FLAT,
    PHASE_BOARD_FLAT
} OrientationPhase;

void resetCenter();
void setupBoardOrientation(OrientationPhase phase);
void getEuler(float *yaw, float *pitch, float *roll);
