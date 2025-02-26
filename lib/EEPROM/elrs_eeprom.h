#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <cstddef>

#define RESERVED_EEPROM_SIZE 1024

class ELRS_EEPROM
{
public:
    void Begin();
    uint8_t ReadByte(const uint32_t address);
    void WriteByte(const uint32_t address, const uint8_t value);
    void Commit();
};
