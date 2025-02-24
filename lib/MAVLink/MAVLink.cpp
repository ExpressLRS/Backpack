#if defined(MAVLINK_ENABLED)
#include <Arduino.h>
#include "MAVLink.h"
#include <config.h>
#include <crsf_protocol.h>
#include <msp.h>
#include <msptypes.h>
#include <CRSF.h>
#include <ESPNOW_Helpers.h>

void
MAVLink::ProcessMAVLinkFromTX(uint8_t c)
{
    mavlink_status_t status;
    mavlink_message_t msg;

    if (mavlink_frame_char(MAVLINK_COMM_0, c, &msg, &status) != MAVLINK_FRAMING_INCOMPLETE)
    {
        if (mavlink_to_gcs_buf_count >= MAVLINK_BUF_SIZE)
        {
            // Cant fit any more msgs in the queue,
            // drop the oldest msg and start overwriting
            mavlink_stats.overflows_downlink++;
            mavlink_to_gcs_buf_count = 0;
        }
        
        // Track gaps in the sequence number, add to a dropped counter
        uint8_t seq = msg.seq;
        if (expectedSeqSet && seq != expectedSeq)
        {
            // account for rollovers
            if (seq < expectedSeq)
            {
                mavlink_stats.drops_downlink += (UINT8_MAX - expectedSeq) + seq;
            }
            else
            {
                mavlink_stats.drops_downlink += seq - expectedSeq;
            }
        }
        expectedSeq = seq + 1;
        expectedSeqSet = true;

        // Queue the msgs, to forward to peers
        mavlink_to_gcs_buf[mavlink_to_gcs_buf_count] = msg;
        mavlink_to_gcs_buf_count++;
        mavlink_stats.packets_downlink++;
        
        // Look for GPS packets - convert them to CRSF
        if (msg.msgid == MAVLINK_MSG_ID_GPS_RAW_INT)
        {
            mavlink_gps_raw_int_t gps_int;
            mavlink_msg_gps_raw_int_decode(&msg, &gps_int);
            CRSF_MK_FRAME_T(crsf_sensor_gps_t) crsfgps = {0};

            crsfgps.p.speed = htobe16(gps_int.vel * 36 / 100);
            crsfgps.p.lat = htobe32(gps_int.lat);
            crsfgps.p.lon = htobe32(gps_int.lon);
            crsfgps.p.heading = htobe16(gps_int.cog);
            crsfgps.p.satcnt = gps_int.satellites_visible;
            crsfgps.p.altitude = htobe16(gps_int.alt / 1000 + 1000);

            CRSF::SetHeaderAndCrc((uint8_t *)&crsfgps, CRSF_FRAMETYPE_GPS, sizeof(crsf_sensor_gps_t), CRSF_ADDRESS_CRSF_TRANSMITTER);

            // Wrap in MSP
            mspPacket_t packet;
            packet.reset();
            packet.makeCommand();
            packet.function = MSP_ELRS_BACKPACK_CRSF_TLM;
            for (size_t i = 0; i < sizeof(crsfgps); i++)
            {
                packet.addByte(((uint8_t *)&crsfgps)[i]);
            }
            // Send it out ESPNOW
            ESPNOW::sendMSPViaEspnow(&packet);
        }
    }
}

void
MAVLink::ProcessMAVLinkFromGCS(uint8_t *data, uint16_t len)
{
    mavlink_status_t status;
    mavlink_message_t msg;

    for (uint16_t i = 0; i < len; i++)
    {
        if (mavlink_frame_char(MAVLINK_COMM_1, data[i], &msg, &status) != MAVLINK_FRAMING_INCOMPLETE)
        {
            // Send the message to the tx uart
            uint8_t buf[MAVLINK_MAX_PACKET_LEN];
            uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
            Serial.write(buf, len);
            mavlink_stats.packets_uplink++;
        }
    }
}
#endif
