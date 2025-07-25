#if defined(PEDAL_BACKPACK)

#include "module_pedal.h"
#include "common.h"
#include "msptypes.h"

void sendMSPViaEspnow(mspPacket_t *packet);
bool BindingExpired(uint32_t now);

PedalModule::PedalModule()
 :  _lastTxValue(false), _lastNow(0), _lastTxMs(0), _lastPedalChangeMs(0)
{
    _pedal.OnLongPress = std::bind(&PedalModule::button_OnLongPress, this);
}

PedalModule::~PedalModule()
{
    detachInterrupt(PIN_PEDAL_BUTTON);
}

void PedalModule::Init()
{
    _pedalSemaphore = xSemaphoreCreateBinary();
    attachInterruptArg(PIN_PEDAL_BUTTON, &PedalModule::button_interrupt, this, CHANGE);
}

void PedalModule::Loop(uint32_t now)
{
    // Need to store what the main loop thinks is "now" in case the button
    // event is going to use it to set the bindstart time, otherwise millis()
    // could be in the future (compared to now) and timeout immediately
    _lastNow = now;
    if (BindingExpired(_lastNow))
    {
        connectionState = running;
    }

    // Pause here to lower power usage (C3 drops from 100mA to 90mA)
    // If button is currently transitioning or pressed just consume the semaphore, else delay until an interrupt
    xSemaphoreTake(_pedalSemaphore, _pedal.isIdle() ? pdMS_TO_TICKS(20) : 0);
    _pedal.update();

    // Don't TX if binding/wifi
    if (connectionState != running)
        return;
    // Don't TX on startup
    if (_lastTxMs == 0 && _lastNow < STARTUP_MS)
        return;

    checkSendPedalPos();
}

void PedalModule::checkSendPedalPos()
{
    bool pedalPressed = _pedal.isPressed();
    bool pedalChanged = pedalPressed != _lastTxValue;

    if (pedalChanged)
    {
        _lastPedalChangeMs = _lastNow;
    }

    // Send the pedal position frequently after last change, until it is unchanged for one UNCHANGED_MS interval
    uint32_t txIntervalMs = (_lastNow - _lastPedalChangeMs < PEDAL_INTERVAL_UNCHANGED_MS) ? PEDAL_INTERVAL_CHANGED_MS : PEDAL_INTERVAL_UNCHANGED_MS;
    if (pedalChanged || _lastNow - _lastTxMs > txIntervalMs)
    {
        _lastTxMs = _lastNow;
        _lastTxValue = pedalPressed;
        DBGLN("%u pedal %u", _lastNow, pedalPressed);

        // Pedal is 1000us if not pressed, 2000us if pressed
        uint16_t pedalState = pedalPressed ? CRSF_CHANNEL_VALUE_2000 : CRSF_CHANNEL_VALUE_1000;
        mspPacket_t packet;
        packet.reset();
        packet.makeCommand();
        packet.function = MSP_ELRS_BACKPACK_SET_PTR;
        packet.addByte(pedalState & 0xFF); // CH0
        packet.addByte(pedalState >> 8);
        packet.addByte(0); // CH1
        packet.addByte(0);
        packet.addByte(0); // CH2
        packet.addByte(0);
        sendMSPViaEspnow(&packet);
        yield();
    }
}

void IRAM_ATTR PedalModule::button_interrupt(void *instance)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(((PedalModule *)instance)->_pedalSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
}

void PedalModule::button_OnLongPress()
{
    // Long press of the pedal for 5s takes the pedal into binding mode
    if (_pedal.getLongCount() >= 10 && connectionState == running)
    {
        bindingStart = _lastNow;
        connectionState = binding;
    }
    // Long press of pedal for 10s goes from binding to wifi
    else if (_pedal.getLongCount() >= 20 && connectionState == binding)
    {
        connectionState = wifiUpdate;
    }
}

#endif /* PEDAL_BACKPACK */