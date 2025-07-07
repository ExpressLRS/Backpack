typedef enum
{
    RACE_STATE_READY            = 0x00,
    RACE_STATE_SCHEDULED        = 0x01,
    RACE_STATE_STAGING          = 0x02,
    RACE_STATE_RACING           = 0x03,
    RACE_STATE_OVERTIME         = 0x04,
    RACE_STATE_PAUSED           = 0x05,
    RACE_STATE_STOPPED          = 0x06
} raceState_e;

// Currently modeled on RotorHazard's Realtime lap information specification
// https://github.com/RotorHazard/RotorHazard/wiki/Specification:-Realtime-lap-information
typedef enum
{
    // Race Data Object
    // The race data object contains information about the race that may be 
    // important to the interpretation of other data.
    
    RACE_TOTAL_PILOTS           = 0x00,
    RACE_LAP_MAX                = 0x01,
    RACE_CONSECUTIVES_BASE      = 0x02,
    RACE_WIN_CONDITION          = 0x03,
    RACE_BEST_LAP               = 0x04,
    RACE_BEST_LAP_CALLSIGN      = 0x05,
    RACE_SPLIT_COUNT            = 0x06,

    // Current Lap Data Object
    // The current lap data object contains data of the pilot currently crossing.
    
    CURRENT_CALLSIGN            = 0x10,
    CURRENT_SEAT                = 0x11,
    CURRENT_POSITION            = 0x12,
    CURRENT_LAP_NUMBER          = 0x13,
    CURRENT_SPLIT_NUMBER        = 0x14,
    CURRENT_TIME                = 0x15,
    CURRENT_TOTAL_TIME          = 0x16,
    CURRENT_TOTAL_TIME_LAPS     = 0x17,
    CURRENT_CONSECUTIVES        = 0x18,
    CURRENT_CONSECUTIVES_BASE   = 0x19,
    CURRENT_IS_BEST             = 0x1A,
    CURRENT_IS_DONE             = 0x1B,

    // Next Rank Data Object
    // The next rank data object contains information about the pilot that occupies 
    // the next higher rank at the time of crossing.
    
    NEXT_CALLSIGN               = 0x20,
    NEXT_SEAT                   = 0x21,
    NEXT_POSITION               = 0x22,
    NEXT_DIFF_TIME              = 0x23,
    NEXT_LAP_NUMBER             = 0x24,
    NEXT_SPLIT_NUMBER           = 0x25,
    NEXT_LAST_TIME              = 0x26,
    NEXT_TOTAL_TIME             = 0x27,

    // First Rank Data Object
    // The first rank data object contains information about the pilot that 
    // occupies the highest rank (1) at the time of crossing.
    
    FIRST_CALLSIGN              = 0x30,
    FIRST_SEAT                  = 0x31,
    FIRST_POSITION              = 0x32,
    FIRST_DIFF_TIME             = 0x33,
    FIRST_LAP_NUMBER            = 0x34,
    FIRST_SPLIT_NUMBER          = 0x35,
    FIRST_LAST_TIME             = 0x36,
    FIRST_TOTAL_TIME            = 0x37,

} timerRaceData_e;

typedef enum
{
    SYSTEM_RACE_TIME            = 0x00
} systemRaceData_e;