#pragma once

extern const unsigned char target_name[];
extern const uint8_t target_name_size;
extern const char PROGMEM compile_options[];

typedef struct {
    uint8_t uid[6];
    bool    hasUID;
    char    home_wifi_ssid[33];
    char    home_wifi_password[65];
    char    product_name[65];
} firmware_options_t;

extern firmware_options_t firmwareOptions;

extern bool options_init();