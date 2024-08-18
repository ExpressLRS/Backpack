#pragma once

#include "module_base.h"
#include "msptypes.h"

#define VRX_UART_BAUD           115200

#define MAVLINK_SYSTEM_ID       1
#define MAVLINK_COMPONENT_ID    1
#define MAVLINK_SYSTEM_TYPE     1
#define MAVLINK_AUTOPILOT_TYPE  3
#define MAVLINK_SYSTEM_MODE     64
#define MAVLINK_CUSTOM_MODE     0
#define MAVLINK_SYSTEM_STATE    4
#define MAVLINK_UPTIME          0

class MFDCrossbow : public ModuleBase
{
public:
    MFDCrossbow(HardwareSerial *port);
    void SendGpsTelemetry(crsf_packet_gps_t *packet);
    void Loop(uint32_t now);

private:
    void SendHeartbeat();
    void SendGpsRawInt();
    void SendGlobalPositionInt();

    HardwareSerial *m_port;
    uint32_t gpsLastSent;
    uint32_t gpsLastUpdated;

    int16_t heading;    // Geographical heading angle in degrees
    float lat;          // GPS latitude in degrees (example: 47.123456)
    float lon;          // GPS longitude in degrees
    float alt;          // Relative flight altitude in m
    float groundspeed;  // Groundspeed in m/s
    int16_t gps_sats;   // Number of visible GPS satellites
    int32_t gps_alt;    // GPS altitude (Altitude above MSL)
    float gps_hdop;     // GPS HDOP
    uint8_t fixType;    // GPS fix type. 0-1: no fix, 2: 2D fix, 3: 3D fix
};
