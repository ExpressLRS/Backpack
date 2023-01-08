#include <Arduino.h>
#include "hdzero.h"
#include "logging.h"
#include "device.h"

void RebootIntoWifi();
bool BindingExpired(uint32_t now);

void
HDZero::Init()
{
    ModuleBase::Init();
}

void
HDZero::SendIndexCmd(uint8_t index)
{
    uint8_t retries = 3;
    while (GetChannelIndex() != index && retries > 0)
    {
        SetChannelIndex(index);
        retries--;
    }
}

uint8_t
HDZero::GetChannelIndex()
{
    MSP msp;
    mspPacket_t* packet = new mspPacket_t;
    packet->reset();
    packet->makeCommand();
    packet->function = MSP_ELRS_BACKPACK_GET_CHANNEL_INDEX;

    // Send request, then wait for a response back from the VRX
    bool receivedResponse = msp.awaitPacket(packet, m_port, VRX_RESPONSE_TIMEOUT);

    if (receivedResponse)
    {
        packet = msp.getReceivedPacket();
        return packet->readByte();
    }

    DBGLN("HDZero module: Exceeded timeout while waiting for channel index response");
    return CHANNEL_INDEX_UNKNOWN;
}

void
HDZero::SetChannelIndex(uint8_t index)
{
    MSP msp;
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_CHANNEL_INDEX;
    packet.addByte(index);  // payload

    msp.sendPacket(&packet, m_port);
}

uint8_t
HDZero::GetRecordingState()
{
    MSP msp;
    mspPacket_t* packet = new mspPacket_t;
    packet->reset();
    packet->makeCommand();
    packet->function = MSP_ELRS_BACKPACK_GET_RECORDING_STATE;

    // Send request, then wait for a response back from the VRX
    bool receivedResponse = msp.awaitPacket(packet, m_port, VRX_RESPONSE_TIMEOUT);

    if (receivedResponse)
    {
        packet = msp.getReceivedPacket();
        return packet->readByte() ? VRX_DVR_RECORDING_ACTIVE : VRX_DVR_RECORDING_INACTIVE;
    }

    DBGLN("HDZero module: Exceeded timeout while waiting for recording state response");
    return VRX_DVR_RECORDING_UNKNOWN;
}

void
HDZero::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
    DBGLN("SetRecordingState = %d delay = %d", recordingState, delay);

    MSP msp;
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_RECORDING_STATE;
    packet.addByte(recordingState);
    packet.addByte(delay & 0xFF); // delay byte 1
    packet.addByte(delay >> 8); // delay byte 2

    msp.sendPacket(&packet, m_port);
}

void
HDZero::Loop(uint32_t now)
{
    static bool isBinding = false;
    // if "binding" && timeout
    if (BindingExpired(now))
    {
        connectionState = running;
        isBinding = false;
        DBGLN("Bind timeout");
        sendResponse('F');  // FAILED
    }
    if (isBinding && connectionState == running)
    {
        DBGLN("Bind completed");
        isBinding = false;
        sendResponse('O');  // OK
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
                    sendResponse('P');
                }
            }
        }
    }
}

void
HDZero::sendResponse(uint8_t response)
{
    mspPacket_t packet;
    packet.reset();
    packet.makeResponse();
    packet.function = MSP_ELRS_BACKPACK_SET_MODE;
    packet.addByte(response);  // payload
    msp.sendPacket(&packet, m_port);
}
