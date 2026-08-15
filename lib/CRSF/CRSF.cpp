#include "CRSF.h"
#include <crc.h>

GENERIC_CRC8 crsf_crc(CRSF_CRC_POLY);

void CRSF::SetHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e destAddr)
{
    auto *header = (crsf_header_t *)frame;
    header->sync_byte = destAddr;
    header->frame_size = frameSize;
    header->type = frameType;

    uint8_t crc = crsf_crc.calc(&frame[CRSF_FRAME_NOT_COUNTED_BYTES], frameSize - 1, 0);
    frame[frameSize + CRSF_FRAME_NOT_COUNTED_BYTES - 1] = crc;
}