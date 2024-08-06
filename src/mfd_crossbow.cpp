#include "mfd_crossbow.h"
#include "common/mavlink.h"

//Basic UAV Parameters
uint8_t system_id = 1;        // MAVLink system ID. Leave at 0 unless you need a specific ID.
uint8_t component_id = 1;     // Should be left at 0. Set to 190 to simulate mission planner sending a command
uint8_t system_type = 1;      // UAV type. 0 = generic, 1 = fixed wing, 2 = quadcopter, 3 = helicopter
uint8_t autopilot_type = 3;   // Autopilot type. Usually set to 0 for generic autopilot with all capabilities
uint8_t system_mode = 64;     // Flight mode. 4 = auto mode, 8 = guided mode, 16 = stabilize mode, 64 = manual mode
uint32_t custom_mode = 0;     // Usually set to 0          
uint8_t system_state = 4;     // 0 = unknown, 3 = standby, 4 = active
uint32_t upTime = 0;          // System uptime, usually set to 0 for cases where it doesn't matter

// Flight parameters
float roll = 0;         // Roll angle in degrees
float pitch = 0;        // Pitch angle in degrees
float yaw = 0;          // Yaw angle in degrees
int16_t heading = 0;    // Geographical heading angle in degrees
float lat = 0.0;        // GPS latitude in degrees (example: 47.123456)
float lon = 0.0;        // GPS longitude in degrees
float alt = 0.0;        // Relative flight altitude in m
float lat_temp = 0.0;   //Temp values for GPS coords
float lon_temp = 0.0; 
float groundspeed = 0.0; // Groundspeed in m/s
float airspeed = 0.0;    // Airspeed in m/s
float climbrate = 0.0;   // Climb rate in m/s, currently not working
float throttle = 0.0;    // Throttle percentage

// GPS parameters
int16_t gps_sats = 0;     // Number of visible GPS satellites
int32_t gps_alt = 0.0;    // GPS altitude (Altitude above MSL)
float gps_hdop = 100.0;   // GPS HDOP
uint8_t fixType = 0;      // GPS fix type. 0-1: no fix, 2: 2D fix, 3: 3D fix

// Battery parameters
float battery_remaining = 0.0;  // Remaining battery percentage
float voltage_battery = 0.0;    // Battery voltage in V
float current_battery = 0.0;    // Battery current in A


void
MFDCrossbow::send_heartbeat(uint8_t system_id, uint8_t component_id, uint8_t system_type, uint8_t autopilot_type, uint8_t system_mode, uint32_t custom_mode, uint8_t system_state)
{
    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  
    // Pack the message
    mavlink_msg_heartbeat_pack(system_id,component_id, &msg, system_type, autopilot_type, system_mode, custom_mode, system_state);
  
    // Copy the message to the send buffer
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  
    // Send the message
    m_port->write(buf, len);
}

void
MFDCrossbow::send_gps_raw_int(int8_t system_id, int8_t component_id, int32_t upTime, int8_t fixType, float lat, float lon, float alt, float gps_alt, int16_t heading, float groundspeed, float gps_hdop, int16_t gps_sats)
{
    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    uint16_t eph = UINT16_MAX;
    uint16_t epv = UINT16_MAX;
    uint16_t vel = groundspeed;
    uint16_t cog = UINT16_MAX;
    uint8_t satellites_visible = gps_sats;
    uint32_t alt_ellipsoid = 0;
    uint32_t h_acc = 0;
    uint32_t v_acc = 0;
    uint32_t vel_acc = 0;
    uint32_t hdg_acc = 0;
    uint16_t yaw = heading;

    // Pack the message
    mavlink_msg_gps_raw_int_pack(system_id, component_id, &msg, upTime, fixType, lat, lon, alt, eph, epv, vel, cog, satellites_visible, alt_ellipsoid, h_acc, v_acc, vel_acc, hdg_acc, yaw);

    // Copy the message to the send buffer
    uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
    
    m_port->write(buf, len);
}

void
MFDCrossbow::send_global_position_int(int8_t system_id, int8_t component_id, int32_t upTime, float lat, float lon, float alt, float gps_alt, uint16_t heading)
{
    int16_t velx = 0; //x speed
    int16_t vely = 0; //y speed
    int16_t velz = 0; //z speed

    // Initialize the required buffers
    mavlink_message_t msg;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // Pack the message
    mavlink_msg_global_position_int_pack(system_id, component_id, &msg, upTime, lat, lon, gps_alt, alt, velx, vely, velz, heading);

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

    groundspeed = be16toh(packet->p.speed) / 36.0 * 100.0;
    heading = be16toh(packet->p.heading);
    alt = (be16toh(packet->p.altitude) - 1000) * 1000.0;
    gps_sats = packet->p.satcnt;
    
    // m_port->println("GPS lat: ");
    // m_port->println(lat);
    // m_port->println("GPS lon: ");
    // m_port->println(lon);
    // m_port->println("GPS speed: ");
    // m_port->println(groundspeed); 
    // m_port->println("GPS heading: ");
    // m_port->println(heading);
    // m_port->println("GPS altitude: ");
    // m_port->println(alt);
    // m_port->println("GPS satellites: ");
    // m_port->println(gps_sats);

    send_heartbeat(system_id, component_id, system_type, autopilot_type, system_mode, custom_mode, system_state);
    send_gps_raw_int(system_id, component_id, upTime, fixType, lat, lon, alt, gps_alt, heading, groundspeed, gps_hdop, gps_sats);
    send_global_position_int(system_id, component_id, upTime, lat, lon, alt, gps_alt, heading);

    // We have received gps values, so we are ok to spam these out at 10hz
    gpsLastUpdated = millis();
}

void
MFDCrossbow::Loop(uint32_t now)
{
    ModuleBase::Loop(now);

    // If the GPS has been updated in the last 10 seconds, keep spamming it out at 10hz
    bool gpsIsValid = (now < gpsLastUpdated + 10000) && gps_sats > 0;

    if (now > gpsLastSent + 100 && gpsIsValid)
    {
        send_heartbeat(system_id, component_id, system_type, autopilot_type, system_mode, custom_mode, system_state);
        send_gps_raw_int(system_id, component_id, upTime, fixType, lat, lon, alt, gps_alt, heading, groundspeed, gps_hdop, gps_sats);
        send_global_position_int(system_id, component_id, upTime, lat, lon, alt, gps_alt, heading);

        gpsLastSent = now;
    }
}