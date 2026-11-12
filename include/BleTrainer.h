#pragma once

#include "device.h"

// Bluetooth trainer bridge: the backpack acts as a BLE central and connects to
// a device that advertises the FrSky trainer service (0xFFF0/0xFFF6), such as a
// HeadTracker, parses the FrSky trainer frame and emits
// MSP_ELRS_BACKPACK_SET_PTR over Serial towards the ELRS module.
//
// Compiled only on BLE-capable targets (ESP32-C3/S3/ESP32) with HAS_BLE_TRAINER.

#if defined(HAS_BLE_TRAINER)

extern device_t BleTrainer_device;

// True while a trainer is connected over BLE.
bool bleTrainerConnected();

// Forget the paired peer and go back to pairing mode (button long press).
void bleTrainerForgetPeer();

// Retained-RAM diagnostic: number of forwarded trainer packets, read from /config
// (the Nomad backpack has no USB console).
uint32_t bleTrainerSetPtrCount();

#endif
