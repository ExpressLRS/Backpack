#pragma once

typedef enum
{
    starting,
    binding,
    running,
    wifiUpdate,
    FAILURE_STATES
} connectionState_e;

extern connectionState_e connectionState;
extern unsigned long bindingStart;
