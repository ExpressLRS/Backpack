#pragma once

#define CRSF_CRC_POLY 0xd5
#define CRSF_SYNC_BYTE 0xc8

#define CRSF_FRAMETYPE_GPS 0x02
#define CRSF_FRAMETYPE_LINK_STATISTICS 0x14
#define CRSF_FRAMETYPE_BATTERY_SENSOR 0x08

#define CRSF_CHANNEL_VALUE_1000 191
#define CRSF_CHANNEL_VALUE_MID  992
#define CRSF_CHANNEL_VALUE_2000 1792

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
