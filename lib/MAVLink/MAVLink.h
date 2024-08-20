#if defined(MAVLINK_ENABLED)
#include "common/mavlink.h"

// This is the port that we will listen for mavlink packets on
constexpr uint16_t MAVLINK_PORT_LISTEN = 14555;
// This is the port that we will send mavlink packets to
constexpr uint16_t MAVLINK_PORT_SEND = 14550;
// Size of the buffer that we will use to store mavlink packets in units of mavlink_message_t
constexpr size_t MAVLINK_BUF_SIZE = 16;
// Threshold at which we will flush the buffer
constexpr size_t MAVLINK_BUF_THRESHOLD = MAVLINK_BUF_SIZE / 2;
// Timeout for flushing the buffer in ms
constexpr size_t MAVLINK_BUF_TIMEOUT = 500;

typedef struct {
  uint32_t packets_downlink; // packets from the aircraft
  uint32_t packets_uplink; // packets to the aircraft
  uint32_t drops_downlink; // dropped packets from the aircraft
  //uint32_t drops_uplink; // framing in the uplink direction cost too much time
  uint32_t overflows_downlink; // buffer overflows from the aircraft
} mavlink_stats_t;

class MAVLink
{
public:
    MAVLink() : mavlink_to_gcs_buf_count(0), expectedSeq(0), expectedSeqSet(false) {}
    void ProcessMAVLinkFromTX(uint8_t c);
    void ProcessMAVLinkFromGCS(uint8_t *data, uint16_t len);

    // Getters
    mavlink_stats_t* GetMavlinkStats() { return &mavlink_stats; }
    mavlink_message_t* GetQueuedMsgs() { return mavlink_to_gcs_buf; }
    uint8_t GetQueuedMsgCount() { return mavlink_to_gcs_buf_count; }

    // Setters
    void ResetQueuedMsgCount() { mavlink_to_gcs_buf_count = 0; }

private:
    mavlink_stats_t     mavlink_stats;
    mavlink_message_t   mavlink_to_gcs_buf[MAVLINK_BUF_SIZE]; // Buffer for storing mavlink packets from the aircraft so that we can send them to the GCS efficiently
    uint8_t             mavlink_to_gcs_buf_count; // Count of the number of messages in the buffer
    uint8_t             expectedSeq;
    bool                expectedSeqSet;
};
#endif
