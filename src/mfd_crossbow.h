#pragma once

#include "module_base.h"
#include "msptypes.h"

#define VRX_UART_BAUD               115200

class MFDCrossbow : public ModuleBase
{
public:
    MFDCrossbow(HardwareSerial *port) : m_port(port), gpsLastSent(0), gpsLastUpdated(0) {};

    void send_heartbeat(uint8_t system_id, uint8_t component_id, uint8_t system_type, uint8_t autopilot_type, uint8_t system_mode, uint32_t custom_mode, uint8_t system_state);
    void send_gps_raw_int(int8_t system_id, int8_t component_id, int32_t upTime, int8_t fixType, float lat, float lon, float alt, float gps_alt, int16_t heading, float groundspeed, float gps_hdop, int16_t gps_sats);
    void send_global_position_int(int8_t system_id, int8_t component_id, int32_t upTime, float lat, float lon, float alt, float gps_alt, uint16_t heading);
    void SendGpsTelemetry(crsf_packet_gps_t *packet);
    void Loop(uint32_t now);

private:
    HardwareSerial *m_port;
    uint32_t gpsLastSent;
    uint32_t gpsLastUpdated;
};
