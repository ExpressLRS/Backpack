// ImmersionRC Rapidfire - SPI Mode Programming
//
// rapidFIRE_SPI_Protocol.h
// Copyright 2023 GOROman.

#pragma once

// The CS1, CS2, CS3 pins are used as the SPI interface. These are normally used as a 3-bit
// binary interface to communicate the goggle selected channel to the module.
// The SPI interface is configured so that the module is the slave (allowing several modules to
// be connected to the same bus), with CPOL = 0, CPHA = 0, MSB first.
// The speed should be limited to a clock rate of about 80kHz.

#define RAPIDFIRE_SPI_CS1 1
#define RAPIDFIRE_SPI_CS2 2
#define RAPIDFIRE_SPI_CS3 3

#define RAPIDFIRE_SPI_CPOL 0
#define RAPIDFIRE_SPI_CPHA 0

#define RAPIDFIRE_SPI_BIT (SPI_MSBFIRST)

#define RAPIDFIRE_SPI_MAX_CLOCK (80000) // 80kHz

#define RAPIDFIRE_SPI_MODE_ENABLE_DELAY 100 // ms

// SPI Protocol
// ------------
//
// Command Heade
//
// | cmd | dir | len | csum | data0 | ... | |
//
// Where Csum is computed as the 8 - bit checksum of all header bytes, Cmd, Dir, Len, and all(optional) data bytes.
//
// Query Header
// | len | csum | data0 | ... | |
//
// Where Csum is computed as the 8-bit checksum of Len, and all data bytes.

// SPI Commands
// ------------
#define RAPIDFIRE_CMD(Cmd, Dir) (Cmd << 8 | Dir)

// Query
#define RAPIDFIRE_CMD_FIRMWARE_VERSION RAPIDFIRE_CMD('F', '?') // - Firmware Version, Query
#define RAPIDFIRE_CMD_VOLTAGE RAPIDFIRE_CMD('V', '?')          // - Voltage, Query
#define RAPIDFIRE_CMD_RSSI RAPIDFIRE_CMD('R', '?')             // - RSSI, Query

// Action
#define RAPIDFIRE_CMD_SOUND_BUZZER RAPIDFIRE_CMD('S', '>') // - Buzzer

// Command
#define RAPIDFIRE_CMD_SET_OSD_USER_TEXT RAPIDFIRE_CMD('T', '=')
#define RAPIDFIRE_CMD_SET_OSD_MODE RAPIDFIRE_CMD('O', '=')
#define RAPIDFIRE_CMD_SET_RX_MODULE RAPIDFIRE_CMD('M', '=')
#define RAPIDFIRE_CMD_SET_CHANNEL RAPIDFIRE_CMD('C', '=')
#define RAPIDFIRE_CMD_SET_BAND RAPIDFIRE_CMD('B', '=')
#define RAPIDFIRE_CMD_SET_RAPIDFIRE_MODE RAPIDFIRE_CMD('D', '=')

