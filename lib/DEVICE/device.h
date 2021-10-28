#pragma once

#include <stdio.h>
#include <stdint.h>

// duration constants which can be returned from start(), event() or timeout()
#define DURATION_IGNORE -2      // when returned from event() does not update the current timeout
#define DURATION_NEVER -1       // timeout() will not be called, only event()
#define DURATION_IMMEDIATELY 0  // timeout() will be called each loop

typedef struct {
    // Called at the beginning of setup() so the device can configure IO pins etc
    void (*initialize)();
    // start() is called at the end of setup() and returns the number of ms when to call timeout()
    int (*start)();
    // An event was fired, take action and return new duration for timeout call
    int (*event)();
    // The duration has passed so take appropriate action and return a new duration, this function should never return DURATION_IGNORE
    int (*timeout)();
} device_t;

void devicesInit(device_t **devices, uint8_t count);
void devicesStart();
void devicesUpdate(unsigned long now);
void devicesTriggerEvent();