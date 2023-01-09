#include "module_base.h"

#include "common.h"
#include "device.h"
#include "msptypes.h"
#include "logging.h"

void RebootIntoWifi();
bool BindingExpired(uint32_t now);

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

void
MSPModuleBase::Loop(uint32_t now)
{
    static bool isBinding = false;
    // if "binding" && timeout
    if (BindingExpired(now))
    {
        connectionState = running;
        isBinding = false;
        DBGLN("Bind timeout");
        sendResponse(MSP_ELRS_BACKPACK_SET_MODE, "F", 1); // FAILED
    }
    if (isBinding && connectionState == running)
    {
        DBGLN("Bind completed");
        isBinding = false;
        sendResponse(MSP_ELRS_BACKPACK_SET_MODE, "O", 1); // OK
    }

    while (m_port->available())
    {
        uint8_t data = m_port->read();
        if (msp.processReceivedByte(data))
        {
            // process the packet
            mspPacket_t *packet = msp.getReceivedPacket();
            if (packet->function == MSP_ELRS_BACKPACK_SET_MODE)
            {
                if (packet->payloadSize == 1)
                {
                    if (packet->payload[0] == 'B')
                    {
                        DBGLN("Enter binding mode...");
                        bindingStart =  now;
                        connectionState = binding;
                        isBinding = true;
                    }
                    else if (packet->payload[0] == 'W')
                    {
                        DBGLN("Enter WIFI mode...");
                        connectionState = wifiUpdate;
                        devicesTriggerEvent();
                    }
                    // send "in-progress" response
                    sendResponse(MSP_ELRS_BACKPACK_SET_MODE, "P", 1);
                }
            }
        }
    }
}


void
MSPModuleBase::sendResponse(uint16_t function, uint8_t *response, uint32_t responseSize)
{
    mspPacket_t packet;
    packet.reset();
    packet.makeResponse();
    packet.function = function;
    for (uint32_t i = 0 ; i<responseSize ; i++)
    {
        packet.addByte(response[i]);
    }
    msp.sendPacket(&packet, m_port);
}
