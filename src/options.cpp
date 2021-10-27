#include <Arduino.h>
#include "options.h"

#define QUOTE(arg) #arg
#define STR(macro) QUOTE(macro)
const unsigned char target_name[] = "\xBE\xEF\xCA\xFE" STR(TARGET_NAME);
const uint8_t target_name_size = sizeof(target_name);

const char PROGMEM compile_options[] = {
#ifdef MY_BINDING_PHRASE
    "-DMY_BINDING_PHRASE=\"" STR(MY_BINDING_PHRASE) "\" "
#endif

#ifdef TARGET_RX
    #ifdef LOCK_ON_FIRST_CONNECTION
        "-DLOCK_ON_FIRST_CONNECTION "
    #endif
    #ifdef USE_R9MM_R9MINI_SBUS
        "-DUSE_R9MM_R9MINI_SBUS "
    #endif
    #ifdef AUTO_WIFI_ON_INTERVAL
        "-DAUTO_WIFI_ON_INTERVAL=" STR(AUTO_WIFI_ON_INTERVAL) " "
    #endif
    #ifdef USE_DIVERSITY
        "-DUSE_DIVERSITY "
    #endif
#endif
};
