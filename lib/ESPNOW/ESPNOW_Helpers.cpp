#include "ESPNOW_Helpers.h"
#include "espnow.h"
#include <options.h>
#include <common.h>
#include "devLED.h"

extern MSP msp;
extern connectionState_e connectionState; // from Vrx_main.cpp

void ESPNOW::sendMSPViaEspnow(mspPacket_t *packet)
{
    // Do not send while in binding mode.  The currently used firmwareOptions.uid may be garbage.
    if (connectionState == binding)
        return;

    uint8_t packetSize = msp.getTotalPacketSize(packet);
    uint8_t nowDataOutput[packetSize];

    uint8_t result = msp.convertToByteArray(packet, nowDataOutput);

    if (!result)
    {
        // packet could not be converted to array, bail out
        return;
    }
    if (packet->function == MSP_ELRS_BIND)
    {
      esp_now_send((uint8_t*)bindingAddress, (uint8_t *) &nowDataOutput, packetSize); // Send Bind packet with the broadcast address
    }
    else
    {
      esp_now_send(firmwareOptions.uid, (uint8_t *) &nowDataOutput, packetSize);
    }
    blinkLED();
}