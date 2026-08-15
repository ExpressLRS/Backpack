#include <stdint.h>
#include "crsf_protocol.h"

class CRSF {
public:
    static void SetHeaderAndCrc(uint8_t *frame, crsf_frame_type_e frameType, uint8_t frameSize, crsf_addr_e destAddr);
};

