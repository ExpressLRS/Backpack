#include "msp.h"
#include "msptypes.h"

class ESPNOW {
public:
    static void sendMSPViaEspnow(mspPacket_t *packet);
};