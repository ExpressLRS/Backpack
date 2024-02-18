#include "module_base.h"

#include "common.h"
#include "options.h"
#include "device.h"
#include "msptypes.h"
#include "logging.h"

#if defined(TARGET_VRX_BACKPACK) && defined(PIN_SCL)
#include "devHeadTracker.h"
#endif

void RebootIntoWifi();
bool BindingExpired(uint32_t now);
extern uint8_t backpackVersion[];
extern bool headTrackingEnabled;
void sendMSPViaEspnow(mspPacket_t *packet);

void
ModuleBase::Init()
{
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
ModuleBase::SetOSD(mspPacket_t *packet)
{
}

void
ModuleBase::SendHeadTrackingEnableCmd(bool enable)
{
}

void
ModuleBase::SetRTC()
{
}

void
ModuleBase::SendLinkTelemetry(uint8_t *rawCrsfPacket)
{
}

void
ModuleBase::SendBatteryTelemetry(uint8_t *rawCrsfPacket)
{
}

void
ModuleBase::Loop(uint32_t now)
{
#if defined(TARGET_VRX_BACKPACK) && defined(PIN_SCL)
    static uint32_t lastSend = 0;
    if (headTrackingEnabled && now - lastSend > 20)
    {
        float fyaw, fpitch, froll;
        getEuler(&fyaw, &fpitch, &froll);

        // convert from degrees to servo positions
        int yaw = fyaw + 90 * 1000 / 180;
        int pitch = fpitch + 90 * 1000 / 180;
        int roll = froll + 90 * 1000 / 180;

        mspPacket_t packet;
        packet.reset();
        packet.makeCommand();
        packet.function = MSP_ELRS_BACKPACK_SET_PTR;
        packet.addByte(yaw & 0xFF);
        packet.addByte(yaw >> 8);
        packet.addByte(pitch & 0xFF);
        packet.addByte(pitch >> 8);
        packet.addByte(roll & 0xFF);
        packet.addByte(roll >> 8);
        sendMSPViaEspnow(&packet);
        lastSend = now;
    }
#endif
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
        sendResponse(MSP_ELRS_BACKPACK_SET_MODE, (const uint8_t *)"F", 1); // FAILED
    }
    if (isBinding && connectionState == running)
    {
        DBGLN("Bind completed");
        isBinding = false;
        sendResponse(MSP_ELRS_BACKPACK_SET_MODE, (const uint8_t *)"O", 1); // OK
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
                    sendResponse(MSP_ELRS_BACKPACK_SET_MODE, (const uint8_t *)"P", 1);
                }
            }
            else if (packet->function == MSP_ELRS_BACKPACK_GET_VERSION)
            {
                sendResponse(MSP_ELRS_BACKPACK_GET_VERSION, backpackVersion, strlen((const char *)backpackVersion));
            }
            else if (packet->function == MSP_ELRS_BACKPACK_GET_STATUS)
            {
                static const uint8_t unbound[6] = {0,0,0,0,0,0};
                uint8_t response[7] = {0};
                response[0] |= connectionState == wifiUpdate ? 1 : 0;
                response[0] |= connectionState == binding ? 2 : 0;
                response[0] |= memcmp(firmwareOptions.uid, unbound, 6) != 0 ? 4 : 0;
                memcpy(&response[1], firmwareOptions.uid, 6);
                sendResponse(MSP_ELRS_BACKPACK_GET_STATUS, response, sizeof(response));
            }
            else if (packet->function == MSP_ELRS_BACKPACK_SET_PTR && headTrackingEnabled)
            {
                sendMSPViaEspnow(packet);
            }
            msp.markPacketReceived();
        }
    }
}


void
MSPModuleBase::sendResponse(uint16_t function, const uint8_t *response, uint32_t responseSize)
{
    mspPacket_t packet;
    packet.reset();
    packet.makeResponse();
    packet.function = function;
    for (uint32_t i = 0 ; i < responseSize ; i++)
    {
        packet.addByte(response[i]);
    }
    msp.sendPacket(&packet, m_port);
}
