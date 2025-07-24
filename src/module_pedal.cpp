#if defined(PEDAL_BACKPACK)

#include "module_pedal.h"
#include "common.h"
#include "msptypes.h"

void sendMSPViaEspnow(mspPacket_t *packet);
bool BindingExpired(uint32_t now);

PedalModule *PedalModule::instance = nullptr;

PedalModule::PedalModule()
 :  _pedalEventFired(false), _lastTxValue(false), _lastTxMs(0), _lastPedalChangeMs(0)
{
    PedalModule::instance = this;
    _pedal.OnShortPress = std::bind(&PedalModule::button_OnShortPress, this);
    _pedal.OnLongPress = std::bind(&PedalModule::button_OnLongPress, this);
}

void PedalModule::Init()
{
    _pedalSemaphore = xSemaphoreCreateBinary();
    attachInterrupt(PIN_PEDAL_BUTTON, &PedalModule::button_interrupt, CHANGE);
}

void PedalModule::Loop(uint32_t now)
{
    if (BindingExpired(now))
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
    if (_lastTxMs == 0 && now < STARTUP_MS)
        return;

    checkSendPedalPos(now);
}

void PedalModule::checkSendPedalPos(uint32_t now)
{
    bool pedalPressed = _pedalEventFired || _pedal.isPressed();
    bool pedalChanged = pedalPressed != _lastTxValue;
    _pedalEventFired = false;

    if (pedalChanged)
    {
        _lastPedalChangeMs = now;
    }

    // Send the pedal position frequently after last change, until it is unchanged for one UNCHANGED_MS interval
    uint32_t txIntervalMs = (now - _lastPedalChangeMs < PEDAL_INTERVAL_UNCHANGED_MS) ? PEDAL_INTERVAL_CHANGED_MS : PEDAL_INTERVAL_UNCHANGED_MS;
    if (pedalChanged || now - _lastTxMs > txIntervalMs)
    {
        _lastTxMs = now;
        _lastTxValue = pedalPressed;
        DBGLN("%u pedal %u", now, pedalPressed);

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
    }
}

void IRAM_ATTR PedalModule::button_interrupt()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(PedalModule::instance->_pedalSemaphore, &xHigherPriorityTaskWoken);
    // No need to if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR(), just let the loop break
}

void PedalModule::button_OnShortPress()
{
    _pedalEventFired = true;
    DBGLN("Short %u", _pedal.getCount());
}

void PedalModule::button_OnLongPress()
{
    _pedalEventFired = true;
    DBGLN("Long %u", _pedal.getLongCount());

    // Long press of the pedal for 5s takes the pedal into binding mode
    if (_pedal.getLongCount() >= 10 && connectionState == running)
    {
        bindingStart = millis();
        connectionState = binding;
    }
    // Long press of pedal for 10s goes from binding to wifi
    else if (_pedal.getLongCount() >= 20 && connectionState == binding)
    {
        connectionState = wifiUpdate;
    }
}

#endif /* PEDAL_BACKPACK */