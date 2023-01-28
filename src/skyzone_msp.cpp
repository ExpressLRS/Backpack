#include "skyzone_msp.h"
#include "logging.h"
#include <Arduino.h>

SkyzoneMSP::SkyzoneMSP(Stream *port)
{
    m_port = port;
}

void
SkyzoneMSP::Init()
{
    ModuleBase::Init();
    m_delay = 0;
}

void
SkyzoneMSP::SendIndexCmd(uint8_t index)
{
    uint8_t retries = 3;
    while (GetChannelIndex() != index && retries > 0)
    {
        SetChannelIndex(index);
        retries--;
    }
}

uint8_t
SkyzoneMSP::GetChannelIndex()
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
        msp.markPacketReceived();
        return packet->readByte();
    }

    DBGLN("Skyzone module: Exceeded timeout while waiting for channel index response");
    return CHANNEL_INDEX_UNKNOWN;
}

void
SkyzoneMSP::SetChannelIndex(uint8_t index)
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
SkyzoneMSP::GetRecordingState()
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
        msp.markPacketReceived();
        return packet->readByte() ? VRX_DVR_RECORDING_ACTIVE : VRX_DVR_RECORDING_INACTIVE;
    }

    DBGLN("Skyzone module: Exceeded timeout while waiting for recording state response");
    return VRX_DVR_RECORDING_UNKNOWN;
}

void
SkyzoneMSP::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
    DBGLN("SetRecordingState = %d delay = %d", recordingState, delay);

    m_recordingState = recordingState;
    m_delay = delay * 1000; // delay is in seconds, convert to milliseconds
    m_delayStartMillis = millis();

    if (m_delay == 0)
    {
        SendRecordingState();
    }
}

void
SkyzoneMSP::SendRecordingState()
{
    DBGLN("SendRecordingState = %d delay = %d", m_recordingState, m_delay);

    MSP msp;
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_RECORDING_STATE;
    packet.addByte(m_recordingState);
    packet.addByte(m_delay & 0xFF); // delay byte 1
    packet.addByte(m_delay >> 8); // delay byte 2

    msp.sendPacket(&packet, m_port);
}

void
SkyzoneMSP::Loop(uint32_t now)
{
    // Handle delay timer for SendRecordingState()
    if (m_delay != 0)
    {
        if (now - m_delayStartMillis >= m_delay)
        {
            SendRecordingState();
            m_delay = 0;
        }
    }
}
