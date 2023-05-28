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
};

firmware_options_t firmwareOptions = {
#ifdef MY_UID
    .uid = {MY_UID},
    .hasUid = true,
#else
    .uid = {0, 0, 0, 0, 0, 0},
    .hasUid = false,
#endif
};
