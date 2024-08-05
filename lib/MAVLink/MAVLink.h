#if defined(MAVLINK_ENABLED)
#include "common/mavlink.h"

class MAVLink
{
public:
    static bool handleControlMessage(mavlink_message_t* msg);
};
#endif