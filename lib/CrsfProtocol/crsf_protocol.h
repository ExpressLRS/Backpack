#pragma once

#define CRSF_CRC_POLY 0xd5
#define CRSF_SYNC_BYTE 0xc8
#define CRSF_FRAME_NOT_COUNTED_BYTES 2

#define PACKED __attribute__((packed))

/**
 * Define the shape of a standard header
 */
typedef struct crsf_header_s
{
    uint8_t sync_byte;   // CRSF_SYNC_BYTE
    uint8_t frame_size;  // counts size after this byte, so it must be the payload size + 2 (type and crc)
    uint8_t type;        // from crsf_frame_type_e
} PACKED crsf_header_t;

typedef enum : uint8_t
{
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_VARIO = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_BARO_ALTITUDE = 0x09,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_OPENTX_SYNC = 0x10,
    CRSF_FRAMETYPE_RADIO_ID = 0x3A,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    // Extended Header Frames, range: 0x28 to 0x96
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,

    //CRSF_FRAMETYPE_ELRS_STATUS = 0x2E, ELRS good/bad packet count and status flags

    CRSF_FRAMETYPE_COMMAND = 0x32,
    // KISS frames
    CRSF_FRAMETYPE_KISS_REQ  = 0x78,
    CRSF_FRAMETYPE_KISS_RESP = 0x79,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C, // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
    // Ardupilot frames
    CRSF_FRAMETYPE_ARDUPILOT_RESP = 0x80,
} crsf_frame_type_e;

typedef enum : uint8_t
{
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_USB = 0x10,
    CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
    CRSF_ADDRESS_RESERVED1 = 0x8A,
    CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE,
    CRSF_ADDRESS_ELRS_LUA = 0xEF
} crsf_addr_e;

#define CRSF_MK_FRAME_T(payload) struct payload##_frame_s { crsf_header_t h; payload p; uint8_t crc; } PACKED

typedef struct crsf_sensor_gps_s {
    int32_t lat;        // degrees * 1e7
    int32_t lon;        // degrees * 1e7
    uint16_t speed;     // big-endian km/h * 10
    uint16_t heading;   // big-endian degrees * 10
    uint16_t altitude;  // big endian meters + 1000
    uint8_t satcnt;     // number of satellites
} crsf_sensor_gps_t;
typedef CRSF_MK_FRAME_T(crsf_sensor_gps_t) crsf_packet_gps_t;

#if !defined(__linux__)
static inline uint16_t htobe16(uint16_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap16(val);
#endif
}

static inline uint16_t be16toh(uint16_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap16(val);
#endif
}

static inline uint32_t htobe32(uint32_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap32(val);
#endif
}

static inline uint32_t be32toh(uint32_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap32(val);
#endif
}
#endif