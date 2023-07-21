#include <Arduino.h>
#include "hdzero.h"
#include "msptypes.h"
#include "logging.h"
#include "time.h"

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
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_GET_CHANNEL_INDEX;

    // Send request, then wait for a response back from the VRX
    bool receivedResponse = msp.awaitPacket(&packet, m_port, VRX_RESPONSE_TIMEOUT);

    if (receivedResponse)
    {
        mspPacket_t *packetResponse = msp.getReceivedPacket();
        msp.markPacketReceived();
        return packetResponse->readByte();
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
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_GET_RECORDING_STATE;

    // Send request, then wait for a response back from the VRX
    bool receivedResponse = msp.awaitPacket(&packet, m_port, VRX_RESPONSE_TIMEOUT);

    if (receivedResponse)
    {
        mspPacket_t *packetResponse = msp.getReceivedPacket();
        msp.markPacketReceived();
        return packetResponse->readByte() ? VRX_DVR_RECORDING_ACTIVE : VRX_DVR_RECORDING_INACTIVE;
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
HDZero::SendHeadTrackingEnableCmd(bool enable)
{
    MSP msp;
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_HEAD_TRACKING;
    packet.addByte(enable);

    msp.sendPacket(&packet, m_port);
}

void
HDZero::SetOSD(mspPacket_t *packet)
{
    MSP msp;
    msp.sendPacket(packet, m_port);
}

void
HDZero::SetRTC()
{
    MSP msp;
    mspPacket_t packet;
    tm timeData;
    if(!getLocalTime(&timeData)) {
        DBGLN("Could not obtain time data.");
        return;
    }
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_RTC;
    packet.addByte(timeData.tm_year);
    packet.addByte(timeData.tm_mon);
    packet.addByte(timeData.tm_mday);
    packet.addByte(timeData.tm_hour);
    packet.addByte(timeData.tm_min);
    packet.addByte(timeData.tm_sec);

    msp.sendPacket(&packet, m_port);
}
