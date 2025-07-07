#include "msp.h"
#include "racetypes.h"

#if defined(RACEOSD_HEIGHT) && defined(RACEOSD_WIDTH)

#ifndef RACEOSD_ENABLED
#define RACEOSD_ENABLED 1
#endif

#define EMPTY_VALUE 0
#define OSD_REFRESH_MILLIS 100

class OSDElement
{
public:
    bool enabled;
    uint8_t col;
    uint8_t row;
    size_t size;

    virtual void getData(uint8_t *) {};
};

class TimerOSDElement : public OSDElement
{
public:
    timerRaceData_e id;
    uint8_t *data;

    TimerOSDElement(timerRaceData_e id, size_t size);
    ~TimerOSDElement();
    void getData(uint8_t *);
};

class SystemOSDElement : public OSDElement
{
public:
    systemRaceData_e id;
    std::function<void(uint8_t *)> call;

    SystemOSDElement(systemRaceData_e id, std::function<void(uint8_t *)> call, size_t size);
    void getData(uint8_t *);
};

typedef struct RowOSDElements
{
    size_t size;
    OSDElement **elements;

    RowOSDElements(size_t size);
} RowOSDElements_t;

// Public functions
void processRaceOSDPacket(mspPacket_t *packet);
void processRaceStatePacket(mspPacket_t *packet);

#if defined(PLATFORM_ESP8266)
void runRaceOSDUpdates(void);
#endif

void setupRaceOSD(void);

#endif
