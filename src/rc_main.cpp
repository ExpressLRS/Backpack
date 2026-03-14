#include "rc_main.h"

void RcMain::SetPeerAddress(const uint8_t addr[6])
{
    memcpy(_peerAddr, addr, 6);
}

bool RcMain::IsTelemEnabled()
{
    return (config.GetTelemMode() == BACKPACK_TELEM_MODE_ESPNOW);
}

RcMain& RcMain::Instance()
{
    static RcMain instance;
    return instance;
}

void RcMain::Init()
{

}

void RcMain::SendCrsf(const uint8_t* data, uint8_t len)
{
    if (config.GetTelemMode() != BACKPACK_TELEM_MODE_ESPNOW)
        return;

    if (!data || len == 0)
        return;

#if defined(PLATFORM_ESP8266)
    esp_now_send(_peerAddr, (uint8_t*)data, len);
#elif defined(PLATFORM_ESP32)
    esp_now_send(_peerAddr, (uint8_t*)data, len);
#endif
}
