#include "hdzero.h"
#include "logging.h"
#include <Arduino.h>

void
HDZero::Init()
{
    DBGLN("HDZero backpack init complete");
}

void
HDZero::SendIndexCmd(uint8_t index)
{  
    uint8_t retries = 5;
    while (GetChannelIndex() != index && retries > 0)
    {
        SetChannelIndex(index);
        retries--;
    }
}

void
HDZero::SetChannelIndex(uint8_t index)
{  
    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_SET_CHANNEL_INDEX;
    packet.addByte(index);  // payload

    SendMSPViaSerial(&packet);
}

uint8_t
HDZero::GetChannelIndex()
{
    MSP msp;
    uint8_t index = CHANNEL_INDEX_UNKNOWN;

    mspPacket_t packet;
    packet.reset();
    packet.makeCommand();
    packet.function = MSP_ELRS_BACKPACK_GET_CHANNEL_INDEX;

    SendMSPViaSerial(&packet);

    // Wait for a response back from the VRX

    uint32_t timeout = 500; // wait up to <timeout> milliseconds for a response, then bail out
    uint32_t requestTime = millis();

    while(millis() - requestTime < timeout)
    {
        while (Serial.available())
        {
            uint8_t data = Serial.read();
            if (msp.processReceivedByte(data))
            {
                DBGLN("Received an MSP packet from VRX");
                mspPacket_t *receivedPacket = msp.getReceivedPacket();
                if (receivedPacket->function == MSP_ELRS_BACKPACK_GET_CHANNEL_INDEX)
                {
                    DBGLN("Received channel index");
                    index = receivedPacket->readByte();
                }
                msp.markPacketReceived();
                return index;
            }
        }
    }
    DBGLN("Exceeded timeout while waiting for channel index response");
    return CHANNEL_INDEX_UNKNOWN;
}

void
HDZero::SendMSPViaSerial(mspPacket_t *packet)
{
    MSP msp;
    
    uint8_t packetSize = msp.getTotalPacketSize(packet);
    uint8_t buf[packetSize];

    uint8_t result = msp.convertToByteArray(packet, buf);

    if (!result)
    {
        DBGLN("Packet could not be converted to array, bail out");
        return;
    }
    
    for (uint8_t i = 0; i < packetSize; ++i)
    {
        Serial.write(buf[i]);
    }

    // Leaving this in as its useful to debug
    // for (uint8_t i = 0; i < pos; ++i)
    // {
    //     Serial.print("0x"); Serial.print(buf[i], HEX); Serial.print(", ");
    // }
    // Serial.println("");
}