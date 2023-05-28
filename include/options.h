#pragma once

extern const unsigned char target_name[];
extern const uint8_t target_name_size;
extern const char PROGMEM compile_options[];

typedef struct {
    uint8_t uid[6];
    bool hasUid;
} firmware_options_t;

extern firmware_options_t firmwareOptions;