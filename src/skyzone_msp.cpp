#include "skyzone_msp.h"
#include "logging.h"
#include <Arduino.h>
#include <max7456.h>
#include <SPI.h>

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
SkyzoneMSP::SetOSD(mspPacket_t *packet)
{
    Max7456 osd;

    byte tab[] = {0xC8, 0xC9};

    SPI.begin();

    osd.init(4);
    osd.setDisplayOffsets(10, 10);
    osd.setBlinkParams(_8fields, _BT_BT);

    osd.activateOSD();
    osd.printMax7456Char(0x01, 0, 1);
    osd.print("Hello world :)", 1, 3);
    osd.print("Current Arduino time :", 1, 4);

    osd.printMax7456Char(0xD1, 9, 6, true);
    osd.print("00'00\"", 10, 6);
    osd.printMax7456Chars(tab, 2, 12, 7);
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
