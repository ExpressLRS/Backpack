#include "mfd_crossbow.h"
#include "common/mavlink.h"

MFDCrossbow::MFDCrossbow(HardwareSerial *port) :
    m_port(port),
    gpsLastSent(0),
    gpsLastUpdated(0),
    heading(0),
    lat(0.0),
    lon(0.0),
    alt(0.0),
    groundspeed(0.0),
    gps_sats(0),
    gps_alt(0),
    gps_hdop(100),
    fixType(3)
{
}

void
MFDCrossbow::SendHeartbeat()
{
    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  
    // Pack the message
    mavlink_msg_heartbeat_pack(MAVLINK_SYSTEM_ID, MAVLINK_COMPONENT_ID, &msg, MAVLINK_SYSTEM_TYPE, MAVLINK_AUTOPILOT_TYPE, MAVLINK_SYSTEM_MODE, MAVLINK_CUSTOM_MODE, MAVLINK_SYSTEM_STATE);
  
    // Copy the message to the send buffer
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  
    // Send the message
    m_port->write(buf, len);
}

void
MFDCrossbow::SendGpsRawInt()
{
    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    const uint16_t eph = UINT16_MAX;
    const uint16_t epv = UINT16_MAX;
    const uint16_t cog = UINT16_MAX;
    const uint32_t alt_ellipsoid = 0;
    const uint32_t h_acc = 0;
    const uint32_t v_acc = 0;
    const uint32_t vel_acc = 0;
    const uint32_t hdg_acc = 0;

    // Pack the message
    mavlink_msg_gps_raw_int_pack(MAVLINK_SYSTEM_ID, MAVLINK_COMPONENT_ID, &msg, MAVLINK_UPTIME, fixType, lat, lon, alt, eph, epv, groundspeed, cog, gps_sats, alt_ellipsoid, h_acc, v_acc, vel_acc, hdg_acc, heading);
    
    // Copy the message to the send buffer
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    
    // Send the message
    m_port->write(buf, len);
}

void
MFDCrossbow::SendGlobalPositionInt()
{
    const int16_t velx = 0;
    const int16_t vely = 0;
    const int16_t velz = 0;

    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // Pack the message
    mavlink_msg_global_position_int_pack(MAVLINK_SYSTEM_ID, MAVLINK_COMPONENT_ID, &msg, MAVLINK_UPTIME, lat, lon, gps_alt, alt, velx, vely, velz, heading);

    // Copy the message to the send buffer
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

    // Send the message
    m_port->write(buf, len);
}

void
MFDCrossbow::SendGpsTelemetry(crsf_packet_gps_t *packet)
{
    int32_t rawLat = be32toh(packet->p.lat); // Convert to host byte order
    int32_t rawLon = be32toh(packet->p.lon); // Convert to host byte order
    lat = static_cast<double>(rawLat);
    lon = static_cast<double>(rawLon);

    // Convert from CRSF scales to mavlink scales
    groundspeed = be16toh(packet->p.speed) / 36.0 * 100.0;
    heading = be16toh(packet->p.heading);
    alt = (be16toh(packet->p.altitude) - 1000) * 1000.0;
    gps_alt = alt;
    gps_sats = packet->p.satcnt;

    // Send heartbeat and GPS mavlink messages to the tracker
    SendHeartbeat();
    SendGpsRawInt();
    SendGlobalPositionInt();

    // Log the last time we received new GPS coords
    gpsLastUpdated = millis();
}

void
MFDCrossbow::Loop(uint32_t now)
{
    ModuleBase::Loop(now);

    // If the GPS data is <= 10 seconds old, keep spamming it out at 10hz
    bool gpsIsValid = (now < gpsLastUpdated + 10000) && gps_sats > 0;

    if (now > gpsLastSent + 100 && gpsIsValid)
    {
        SendHeartbeat();
        SendGpsRawInt();
        SendGlobalPositionInt();

        gpsLastSent = now;
    }
}
