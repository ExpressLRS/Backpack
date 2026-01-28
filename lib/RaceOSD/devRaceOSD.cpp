#include <cstring>
#include <typeinfo>
#include "msp.h"
#include "msptypes.h"
#include "devRaceOSD.h"

#if defined(RACEOSD_HEIGHT) && defined(RACEOSD_WIDTH)

// FreeRTOS management
#if defined(PLATFORM_ESP32)
static StaticSemaphore_t xMutexBuffer;
static SemaphoreHandle_t xSemaphore = NULL;

#define STACK_SIZE 1028
static TaskHandle_t xOSDUpdateTask = NULL;
StaticTask_t xTaskBuffer;
StackType_t xStack[ STACK_SIZE ];

static TimerHandle_t xOSDTimer = NULL;
StaticTimer_t pxTimerBuffer;
#endif

// OSD Data
static mspPacket_t displayPacket;
static raceState_e state = RACE_STATE_READY;
static uint32_t raceClockStartTime;
static uint32_t raceDuration;
static bool rowHasUpdate[RACEOSD_HEIGHT];
static RowOSDElements *assignedRowElements[RACEOSD_HEIGHT];

// Create allocate memory for elements from timer
static TimerOSDElement timerElements[35] = {
    {RACE_TOTAL_PILOTS, 1},
    {RACE_LAP_MAX, 2},
    {RACE_CONSECUTIVES_BASE, 1},
    {RACE_WIN_CONDITION, 1},
    {RACE_BEST_LAP, 1},
    {RACE_BEST_LAP_CALLSIGN, 16},
    {RACE_SPLIT_COUNT, 1},

    {CURRENT_CALLSIGN, 16},
    {CURRENT_SEAT, 1},
    {CURRENT_POSITION, 1},
    {CURRENT_LAP_NUMBER, 2},
    {CURRENT_SPLIT_NUMBER, 1},
    {CURRENT_TIME, 10},
    {CURRENT_TOTAL_TIME, 10},
    {CURRENT_TOTAL_TIME_LAPS, 10},
    {CURRENT_CONSECUTIVES, 10},
    {CURRENT_CONSECUTIVES_BASE, 1},
    {CURRENT_IS_BEST, 1},
    {CURRENT_IS_DONE, 1},

    {NEXT_CALLSIGN, 16},
    {NEXT_SEAT, 1},
    {NEXT_POSITION, 1},
    {NEXT_DIFF_TIME, 10},
    {NEXT_LAP_NUMBER, 1},
    {NEXT_SPLIT_NUMBER, 1},
    {NEXT_LAST_TIME, 10},
    {NEXT_TOTAL_TIME, 10},

    {FIRST_CALLSIGN, 16},
    {FIRST_SEAT, 1},
    {FIRST_POSITION, 1},
    {FIRST_DIFF_TIME, 1},
    {FIRST_LAP_NUMBER, 1},
    {FIRST_SPLIT_NUMBER, 1},
    {FIRST_LAST_TIME, 10},
    {FIRST_TOTAL_TIME, 10}};

// Setup iterator for system elements
static void getRaceClockData(uint8_t *buffer);
static SystemOSDElement systemElements[1] = {
    {SYSTEM_RACE_TIME, getRaceClockData, 10}};

TimerOSDElement::TimerOSDElement(timerRaceData_e id, size_t size)
{
    this->id = id;
    this->size = size;
    data = new uint8_t[size];
    memset(data, EMPTY_VALUE, sizeof(*data));
}

TimerOSDElement::~TimerOSDElement()
{
    delete[] data;
    data = nullptr;
}

void
TimerOSDElement::getData(uint8_t *data)
{
    memcpy(data, this->data, sizeof(*this->data));
}

SystemOSDElement::SystemOSDElement(systemRaceData_e id, std::function<void(uint8_t*)> call, size_t size)
{
    this->id = id;
    this->call = call;
    this->size = size;
}

RowOSDElements::RowOSDElements(size_t size)
{
    this->size = size;
}

void
SystemOSDElement::getData(uint8_t *data)
{
    call(data);
}

/*
 * Process incoming race state packet
*/
void processRaceStatePacket(mspPacket_t *packet)
{
    // TODO: Handle potential conditions from
    // previous state

    switch (packet->readByte())
    {
    case RACE_STATE_READY:
        state = RACE_STATE_READY;
        break;
    case RACE_STATE_SCHEDULED:
        state = RACE_STATE_SCHEDULED;
        break;
    case RACE_STATE_STAGING:
        state = RACE_STATE_STAGING;
        break;
    case RACE_STATE_RACING:
        state = RACE_STATE_RACING;
        break;
    case RACE_STATE_OVERTIME:
        state = RACE_STATE_OVERTIME;
        break;
    case RACE_STATE_PAUSED:
        state = RACE_STATE_PAUSED;
        break;
    case RACE_STATE_STOPPED:
        state = RACE_STATE_STOPPED;
        break;
    default:
        break;
    }
}

/*
 * Saves incoming timer race data
*/
static void saveRaceOSDPacket(mspPacket_t *packet, TimerOSDElement *element)
{
    static u_int16_t packet_size;

    #if defined(PLATFORM_ESP32)
    if (element->enabled && xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
    #else
    if (element.enabled)
    #endif
    {
        // Clear any previously stored data
        memset(element->data, EMPTY_VALUE, sizeof(*element->data));
        packet_size = packet->payloadSize - 1; // One byte was already read from payload

        // Write new data
        for (int i = 0; i < packet_size && i < element->size; i++)
        {
            element->data[i] = packet->readByte();
        }

        // Mark flag to render row updates
        rowHasUpdate[element->row] = true;

        #if defined(PLATFORM_ESP32)
        xSemaphoreGive(xSemaphore);
        #endif
    }
    
}

/*
 * Process incoming timer race data packet
*/
void processRaceOSDPacket(mspPacket_t *packet)
{
    switch (packet->readByte())
    {
    case RACE_TOTAL_PILOTS:
        saveRaceOSDPacket(packet, &timerElements[0]);
        break;
    case RACE_LAP_MAX:
        saveRaceOSDPacket(packet, &timerElements[1]);
        break;
    case RACE_CONSECUTIVES_BASE:
        saveRaceOSDPacket(packet, &timerElements[2]);
        break;
    case RACE_WIN_CONDITION:
        saveRaceOSDPacket(packet, &timerElements[3]);
        break;
    case RACE_BEST_LAP:
        saveRaceOSDPacket(packet, &timerElements[4]);
        break;
    case RACE_BEST_LAP_CALLSIGN:
        saveRaceOSDPacket(packet, &timerElements[5]);
        break;
    case RACE_SPLIT_COUNT:
        saveRaceOSDPacket(packet, &timerElements[6]);
        break;
    case CURRENT_CALLSIGN:
        saveRaceOSDPacket(packet, &timerElements[7]);
        break;
    case CURRENT_SEAT:
        saveRaceOSDPacket(packet, &timerElements[8]);
        break;
    case CURRENT_POSITION:
        saveRaceOSDPacket(packet, &timerElements[9]);
        break;
    case CURRENT_LAP_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[10]);
        break;
    case CURRENT_SPLIT_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[11]);
        break;
    case CURRENT_TIME:
        saveRaceOSDPacket(packet, &timerElements[12]);
        break;
    case CURRENT_TOTAL_TIME:
        saveRaceOSDPacket(packet, &timerElements[13]);
        break;
    case CURRENT_TOTAL_TIME_LAPS:
        saveRaceOSDPacket(packet, &timerElements[14]);
        break;
    case CURRENT_CONSECUTIVES:
        saveRaceOSDPacket(packet, &timerElements[15]);
        break;
    case CURRENT_CONSECUTIVES_BASE:
        saveRaceOSDPacket(packet, &timerElements[16]);
        break;
    case CURRENT_IS_BEST:
        saveRaceOSDPacket(packet, &timerElements[17]);
        break;
    case CURRENT_IS_DONE:
        saveRaceOSDPacket(packet, &timerElements[18]);
        break;
    case NEXT_CALLSIGN:
        saveRaceOSDPacket(packet, &timerElements[19]);
        break;
    case NEXT_SEAT:
        saveRaceOSDPacket(packet, &timerElements[20]);
        break;
    case NEXT_POSITION:
        saveRaceOSDPacket(packet, &timerElements[21]);
        break;
    case NEXT_DIFF_TIME:
        saveRaceOSDPacket(packet, &timerElements[22]);
        break;
    case NEXT_LAP_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[23]);
        break;
    case NEXT_SPLIT_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[24]);
        break;
    case NEXT_LAST_TIME:
        saveRaceOSDPacket(packet, &timerElements[25]);
        break;
    case NEXT_TOTAL_TIME:
        saveRaceOSDPacket(packet, &timerElements[26]);
        break;
    case FIRST_CALLSIGN:
        saveRaceOSDPacket(packet, &timerElements[27]);
        break;
    case FIRST_SEAT:
        saveRaceOSDPacket(packet, &timerElements[28]);
        break;
    case FIRST_POSITION:
        saveRaceOSDPacket(packet, &timerElements[29]);
        break;
    case FIRST_DIFF_TIME:
        saveRaceOSDPacket(packet, &timerElements[30]);
        break;
    case FIRST_LAP_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[31]);
        break;
    case FIRST_SPLIT_NUMBER:
        saveRaceOSDPacket(packet, &timerElements[32]);
        break;
    case FIRST_LAST_TIME:
        saveRaceOSDPacket(packet, &timerElements[33]);
        break;
    case FIRST_TOTAL_TIME:
        saveRaceOSDPacket(packet, &timerElements[34]);
        break;
    default:
        break;
    }
}

/*
 * Calculate and format race clock data
*/
static void getRaceClockData(uint8_t* buffer)
{
    static uint32_t clock;
    
    clock = millis() - raceClockStartTime;
    if (raceDuration > 0)
        clock = raceDuration - clock;

    // TODO: Write formated clock data into buffer
}

/*
 * Renders the elements in a row
*/
static void renderOSDRowUpdate(uint8_t* rowData, uint8_t row_num)
{
    static uint8_t* elementData;
    static OSDElement *element;

    #if defined(PLATFORM_ESP32)
    // Take mutex to prevent updates to row while rendering elements
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
    {
    #endif
        
        for (int i = 0; i < assignedRowElements[row_num]->size; i++)
        {
            element = assignedRowElements[row_num]->elements[i];
            
            uint8_t buffer[element->size];
            elementData = buffer;
            element->getData(elementData);

            for (int i = 0; i < element->size && i + element->col < RACEOSD_WIDTH; i++)
            {
                //Break early if two consecutive EMPTY_VALUEs are detected
                if (elementData[i] == EMPTY_VALUE && i + 1 < element->size && elementData[i + 1] == EMPTY_VALUE)
                    break;

                rowData[element->col + i] = elementData[i];
            }
        }

    rowHasUpdate[row_num] = false;
    #if defined(PLATFORM_ESP32)
        xSemaphoreGive(xSemaphore);
    }
    #endif
}

/*
 * @brief Send rendered elements to host
 *
 * Render each updated row and send data to
 * the host device.
*/
static void runOSDUpdates(void *)
{
    static MSP msp;
    static bool updated_flag;
    static mspPacket_t packet;
    static uint8_t rowData[RACEOSD_WIDTH];

    #if defined(PLATFORM_ESP32)
    while (1)
    {
        // Reset the notification state and wait for new
        // notification to proceed
        xTaskNotifyStateClear(xOSDUpdateTask);
        xTaskNotifyWait(0x00, ULONG_MAX, NULL, portMAX_DELAY);
    #endif

        updated_flag = false;
        for (int i = 0; i < RACEOSD_HEIGHT; i++)
        {
            if (rowHasUpdate[i])
            {
                memset(rowData, EMPTY_VALUE, sizeof(rowData));
                renderOSDRowUpdate(rowData, i);
                packet.reset();
                packet.makeCommand();
                packet.function = MSP_ELRS_SET_OSD;
                packet.addByte(0x03);
                packet.addByte(i);
                packet.addByte(0);

                for (int j = 0; j < RACEOSD_WIDTH; j++)
                {
                    packet.addByte(rowData[j]);
                }

                msp.sendPacket(&packet, &Serial);
                updated_flag = true;
            }
        }
    

        // Send packet to render updated row(s)
        if (updated_flag)
        {
            msp.sendPacket(&displayPacket, &Serial);
        }

    #if defined(PLATFORM_ESP32)
    }
    #endif
}

/*
 * @brief Trigger the OSD updates
 *
 * Either spawn in a new task to update the race OSD or perform 
 * the updates as a function call (platform dependent) 
*/
#if defined(PLATFORM_ESP32)
static void triggerRaceOSDUpdates(TimerHandle_t timer)
{
    // Notify OSD updater task to proceed if in a active
    // race state
    if (state == RACE_STATE_STAGING || 
        state == RACE_STATE_RACING ||
        state == RACE_STATE_OVERTIME)
        xTaskNotify(xOSDUpdateTask, 0, eSetValueWithOverwrite);
}
#else
void triggerRaceOSDUpdates(void)
{

    static uint32_t lastUpdateTime;
    static uint32_t now;

    now = millis();
    if (now - lastUpdateTime > OSD_REFRESH_MILLIS)
    {
        runOSDUpdates();
        lastUpdateTime = now;
    }
}
#endif

/*
 * @brief Builds element array for a row
 *
 * Builds an array that contains the element pointers
 * for each row to allow for efficiently rendering OSD updates.
 * This is expected to only be called at startup.
 * 
 * @param row_num The row to build the array for
*/
static void buildRowElementsArray(uint8_t row_num)
{
    uint8_t element_count = 0;

    // Count the total number of elements in row
    for (const TimerOSDElement &element : timerElements)
    {
        if (element.enabled && element.row == row_num)
            element_count++;
    }

    for (const SystemOSDElement &element : systemElements)
    {
        if (element.enabled && element.row == row_num)
            element_count++;
    }

    RowOSDElements *assignedElements = new RowOSDElements(element_count);
    element_count = 0;
    
    for (TimerOSDElement element : timerElements)
    {
        if (element.enabled && element.row == row_num)
            assignedElements->elements[element_count++] = &element;
    }

    for (SystemOSDElement element : systemElements)
    {
        if (element.enabled && element.row == row_num)
            assignedElements->elements[element_count++] = &element;
    }

    assignedRowElements[row_num] = assignedElements;
}

/*
 * @brief RaceOSD startup function
 *
 * Sets up and configures the systems the
 * RaceOSD relies on to function. 
*/
void setupRaceOSD(void)
{

    // Load element settings from EEPROM
    // TODO

    // Build element array for each row
    for (int i = 0; i < RACEOSD_HEIGHT; i++)
    {
        buildRowElementsArray(i);
    }

    // Build display packet for repeated use
    displayPacket.reset();
    displayPacket.makeCommand();
    displayPacket.function = MSP_ELRS_SET_OSD;
    displayPacket.addByte(0x04);

    #if defined(PLATFORM_ESP32)
    // Setup osd element mutex
    xSemaphore = xSemaphoreCreateMutexStatic(&xMutexBuffer);

    // Create task to run OSD updates
    xOSDUpdateTask = xTaskCreateStatic(
                        runOSDUpdates, 
                        "OSD Updater", 
                        STACK_SIZE, 
                        NULL, 
                        8, 
                        xStack, 
                        &xTaskBuffer);

    // Setup timer for generating osd update notifications
    xOSDTimer = xTimerCreateStatic(
                        "OSD Trigger Timer",
                        OSD_REFRESH_MILLIS / portTICK_PERIOD_MS,
                        pdTRUE,
                        (void *)0,
                        triggerRaceOSDUpdates,
                        &pxTimerBuffer);

    xTimerStart(xOSDTimer, portMAX_DELAY);
    #endif
}

#endif
