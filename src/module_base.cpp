#include "module_base.h"

void
ModuleBase::Init()
{
    delay(VRX_BOOT_DELAY);
}

void
ModuleBase::SendIndexCmd(uint8_t index)
{
}

void
ModuleBase::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
}

void
ModuleBase::Loop(uint32_t now)
{
}