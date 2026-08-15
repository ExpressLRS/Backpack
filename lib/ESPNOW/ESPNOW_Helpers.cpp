#include "ESPNOW_Helpers.h"
#include <options.h>
#include <common.h>
#include "devLED.h"
#include "logging.h"
#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
#endif

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
    // Send Bind packets with the broadcast address, everything else to the peer
    const uint8_t *dest = (packet->function == MSP_ELRS_BIND) ? bindingAddress : firmwareOptions.uid;

    // Don't swallow the error.  If ESP-NOW was never initialised (as happens when the
    // backpack boots straight into WiFi mode) every send fails here, and without this
    // log the whole path looks like it is working while nothing goes out on air.
    int err = esp_now_send((uint8_t *)dest, (uint8_t *)&nowDataOutput, packetSize);
    if (err != 0)
    {
        DBGLN("esp_now_send failed (%d) for function 0x%x", err, packet->function);
        return;
    }
    blinkLED();
}