#pragma once

#if defined(PEDAL_BACKPACK)
#include "module_base.h"
#include "logging.h"
#include "button.h"

// For wake
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class PedalModule : public ModuleBase
{
public:
    PedalModule();
    ~PedalModule();

    void Init();
    void Loop(uint32_t now);
private:
    static void button_interrupt(void *instance);

    // Don't send updates on for a short time after bootup
    static constexpr uint32_t STARTUP_MS = 2000U;
    // Transmit interval differs based on how recently the pedal changed position
    static constexpr uint32_t PEDAL_INTERVAL_UNCHANGED_MS = 750U;
    static constexpr uint32_t PEDAL_INTERVAL_CHANGED_MS = 50U;

    void button_OnLongPress();
    void checkSendPedalPos();

    Button<PIN_PEDAL_BUTTON, false> _pedal;
    SemaphoreHandle_t _pedalSemaphore;
    bool _lastTxValue;
    uint32_t _lastNow;
    uint32_t _lastTxMs;
    uint32_t _lastPedalChangeMs;
};

#endif /* PEDAL_BACKPACK */