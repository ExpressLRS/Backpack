/*
 * max7456Registers.h
 *
 *  Created on: 13 oct. 2012
 *      Author: Benoit
 */

#ifndef MAX7456REGISTERS_H_
#define MAX7456REGISTERS_H_

#include <Arduino.h>

/**
 * @typedef charact
 * @brief Represents a character as stored in max7456 character memory.
 */
typedef byte charact[54];

#define VM0_ADDRESS_WRITE 0x00
#define VM0_ADDRESS_READ 0x80

/**
 * @union REG_VM0
 * @brief Represents a Video Mode 0 Register value
 */
union REG_VM0 {
  /**@brief The whole value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Video BUffer Enable
     * @li 0 = Enable
     * @li 1 = Disable(VOUT is high impedance)
     */
    unsigned char videoBuffer : 1;

    /**@brief Software Reset Bit
     * @li When bit set all register are set to default.
     */
    unsigned char softwareResetBit : 1;

    /**@brief Vertical Synchronization of On-Screen Data
     * @li 0 = Enable osd immediately
     * @li 1 = Enable osd at the next ~VSYNC
     */
    unsigned char verticalSynch : 1;

    /**@brief Enable Display of OSD Image
     * @li 0 = Off
     * @li 1 = On
     */
    unsigned char enableOSD : 1;

    /**@brief Synch Select Mode
     * @li 0x (0 ou 1) = Autosynch select
     * @li 10 (2) = external
     * @li 11 (3) = internal
     */
    unsigned char synchSelect : 2;

    /**@brief Video Standard Select
     * @li 0 = NTSC
     * @li 1 = PAL
     */
    unsigned char videoSelect : 1;

    /**@brief don't care*/
    unsigned char unused : 1;
  } bits;
};

#define VM1_ADDRESS_WRITE 0x01
#define VM1_ADDRESS_READ 0x81

/**@union REG_VM1
 * @brief Represents a Video Mode 1 Register value.
 */
union REG_VM1 {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Blinking Duty Cycle (On:Off)
     * @li b00 (0) = BT:BT
     * @li b01 (1) = BT:(2*BT)
     * @li b10 (2) = BT:(3*BT)
     * @li b11 (3) = (3*BT):BT
     */
    unsigned char blinkingDutyCycle : 2;

    /**@brief Blinking Time (BT)
     * @li b00 (0) = 2 fields (NTSC = 33ms ; PAL = 40ms)
     * @li b01 (1) = 4 fields (NTSC = 67ms ; PAL = 80ms)
     * @li b10 (2) = 6 fields (NTSC = 100ms ; PAL = 120ms)
     * @li b11 (3) = 8 fields (NTSC = 133ms ; PAL = 160ms)
     */
    unsigned char blinkingTime : 2;

    /**@brief Background Mode Brightness
     * @li b000 (0) = 0%
     * @li b001 (1) = 7%
     * @li b010 (2) = 14%
     * @li b011 (3) = 21%
     * @li b100 (4) = 28%
     * @li b101 (5) = 35%
     * @li b110 (6) = 43%
     * @li b111 (7) = 49%
     */
    unsigned char backgroundModeBrightness : 3;

    /**@brief Background Mode
     * @li 0 = DMM[5] & DMM[7] sets the state of each character background
     * @li 1 = Sets all displayed background pixel to gray
     */
    unsigned char backgroundMode : 1;
  } bits;
};

#define HOS_ADDRESS_WRITE 0x02
#define HOS_ADDRESS_READ 0x82

/**@union REG_HOS
 * @brief Represents a Horizontal Offset Register value.
 */
union REG_HOS {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Vertical Position Offset
     * @li b00 0000 (0) = Farthest left (-32 pixels)
     * @li .
     * @li .
     * @li b10 0000 (32) = No horizontal offset
     * @li .
     * @li .
     * @li b11 1111 (31) = Farthest right (+31pixels)
     */
    unsigned char horizontalPositionOffset : 6;

    /**@brief Don't care*/
    unsigned char unsused : 2;
  } bits;
};

#define VOS_ADDRESS_WRITE 0x03
#define VOS_ADDRESS_READ 0x83

/**@union REG_VOS
 * @brief Represents a Vertical Offset Register value.
 */
union REG_VOS {
  /**@brief The whole register value*/
  unsigned char whole;

  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Vertical Position Offset
     * @li b0 0000 (0) = Farthest up (+16 pixels)
     * @li .
     * @li .
     * @li b1 0000 (16) = No vertical offset
     * @li .
     * @li .
     * @li b1 1111 (31) = Farthest down (-15 pixels)
     */
    unsigned char verticalPositionOffset : 5;

    /**@brief Don't care*/
    unsigned char unsused : 3;
  } bits;
};

#define DMM_ADDRESS_WRITE 0x04
#define DMM_ADDRESS_READ 0x84
/**@union REG_DMM
 * @brief Represents a Display Memory Mode value.
 */
union REG_DMM {
  /**@brief The whole register value*/
  unsigned char whole;

  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Auto-Increment Mode
     * @li 0 = Disabled
     * @li 1 = Enabled
     * @note When this bit is enabled for the first time, data in the Display Memory Address (DMAH[0] and
    DMAL[7:0]) registers are used as the starting location to which the data is written. When performing
    the auto-increment write for the display memory, the 8-bit address is internally generated, and
    therefore only 8-bit data is required by the SPI-compatible interface (Figure 21). The content is to
    be interpreted as a Character Address byte if DMAH[1] = 0 or a Character Attribute byte if
    DMAH[1] = 1. This mode is disabled by writing the escape character 1111 1111.
    If the Clear Display Memory bit is set, this bit is reset internally.
     */
    unsigned char autoIncrementMode : 1;

    /**@brief Vertical Sync Clear (Valid only when clear display memory = 1, (DMM[2] = 1) )
     * @li 0 = Immediately applies the clear display-memory command, DMM[2] = 1
     * @li 1 = Applies the clear display-memory command, DMM[2] = 1, at the next VSYNC time
     */
    unsigned char verticalSynchClear : 1;

    /**@brief Clear Display Memory
     * @li 0 = Inactive
     * @li 1 = Clear (fill all display memories with zeros)
     * @note This bit is automatically cleared after the operation is completed (the operation requires
     * 20us). The user does not need to write a 0 afterwards. The status of the bit can be checked by
     * reading this register.
     * This operation is automatically performed:
     * a) On power-up
     * b) Immediately following the rising edge of RESET
     * c) Immediately following the rising edge of CS after VM0[1] has been set to 1
     */
    unsigned char clearDisplayMemory : 1;

    /**@brief Invert Bit (applies to characters written in 16-bit operating mode)
     * @li 0 = Normal (white pixels display white, black pixels display black)
     * @li 1 = Invert (white pixels display black, black pixels display white)
     */
    unsigned char INV : 1;

    /**@brief Blink Bit (applies to characters written in 16-bit operating mode)
     * @li 0 = Blinking off
     * @li 1 = Blinking on
     * @note Blinking rate and blinking duty cycle data in the Video Mode 1 (VM1) register are used for blinking control
     */
    unsigned char BLK : 1;

    /**@brief Local Background Control Bit (applies to characters written in 16-bit operating mode)
     * @li 0 = sets the background pixels of the character to the video input (VIN) when in external sync mode.
     * @li 1 = sets the background pixels of the character to the background mode brightness level defined by VM1[6:4] in external or internal sync mode.
     * @note In internal sync mode, the local background control bit behaves as if it is set to 1
     */
    unsigned char LBC : 1;

    /**@brief Operation Mode Selection
     * @li 0 = 16-bit operation mode
     * @li 1 = 8-bit operation mode
     */
    unsigned char operationModeSelection : 1;

    /**@brief Don't care*/
    unsigned char unsused : 1;
  } bits;
};

#define DMAH_ADDRESS_WRITE 0x05
#define DMAH_ADDRESS_READ 0x85
/**@union REG_DMAH
 * @brief Represents a Display Memory Address High Register value
 */
union REG_DMAH {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief 8th bit for Display Memory Address.
     */
    unsigned char DisplayMemoryAdressBit8 : 1;
    unsigned char byteSelectionBit : 1;
    /**@brief Don't care*/
    unsigned char unsused : 6;
  } bits;
};

#define DMAL_ADDRESS_WRITE 0x06
#define DMAL_ADDRESS_READ 0x86
typedef unsigned char REG_DMAL;

#define DMDI_ADDRESS_WRITE 0x07
#define DMDI_ADDRESS_READ 0x87
typedef unsigned char REG_DMDI;

#define CMM_ADDRESS_WRITE 0x08
#define CMM_ADDRESS_READ 0x88
typedef unsigned char REG_CMM;

#define CMAH_ADDRESS_WRITE 0x09
#define CMAH_ADDRESS_READ 0x89

/**@typedef REG_CMAH
 * @brief Represents a Character Memory Address HIGH value
 */
typedef unsigned char REG_CMAH;

#define CMAL_ADDRESS_WRITE 0x0A
#define CMAL_ADDRESS_READ 0x8A
/**@typedef REG_CMAL
 * @brief Represents a Character Memory Address Low value
 */
typedef unsigned char REG_CMAL;

#define CMDI_ADDRESS_WRITE 0x0B
#define CMDI_ADDRESS_READ 0x8B

/**@union REG_CMDI
 * @brief Represents a Character Memory Data In Register value
 */
union REG_CMDI {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {

    /**@brief value of the right most pixel*/
    unsigned char rightMostPixel : 2;

    /**@brief value of the right center pixel*/
    unsigned char rightCenterPixel : 2;

    /**@brief value of the left center pixel*/
    unsigned char leftCenterPixel : 2;

    /**@brief value of the left most pixel*/
    unsigned char leftMostPixel : 2;

  } bits;
};

#define OSDM_ADDRESS_WRITE 0x0C
#define OSDM_ADDRESS_READ 0x8C

/**@union REG_OSDM
 * @brief Represents an OSD Insersion Mux Register value
 */
union REG_OSDM {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits
   * */
  struct {
    /**@brief OSD Insersion Mux Switching Time
     * @li b000 (0) : 30ns (maximum sharpness/maximum crosscolor artifacts )
     * @li b001 (1) : 35ns
     * @li b010 (2) : 50ns
     * @li b011 (3) : 75ns
     * @li b100 (4) : 100ns
     * @li b101 (5) : 120ns (minimum sharpness/minimum crosscolor artifacts)
     */
    unsigned char osdInsertionMuxSwitchingTime : 3;

    /**@brief OSD Rise And Fall Time
     * @li b000 (0) : 20ns (maximum sharpness/maximum crosscolor artifacts )
     * @li b001 (1) : 30ns
     * @li b010 (2): 35ns
     * @li b011 (3) : 60ns
     * @li b100 (4) : 80ns
     * @li b101 (5) : 110ns (minimum sharpness/minimum crosscolor artifacts)
     */
    unsigned char osdRiseAndFallTime : 3;

    /**@brief don't care*/
    unsigned char unused : 2;

  } bits;
};

#define RB0_ADDRESS_WRITE 0x10
#define RB0_ADDRESS_READ 0x90

#define RB1_ADDRESS_WRITE 0x11
#define RB1_ADDRESS_READ 0x91

#define RB2_ADDRESS_WRITE 0x12
#define RB2_ADDRESS_READ 0x92

#define RB3_ADDRESS_WRITE 0x13
#define RB3_ADDRESS_READ 0x93

#define RB4_ADDRESS_WRITE 0x14
#define RB4_ADDRESS_READ 0x94

#define RB5_ADDRESS_WRITE 0x15
#define RB5_ADDRESS_READ 0x95

#define RB6_ADDRESS_WRITE 0x16
#define RB6_ADDRESS_READ 0x96

#define RB7_ADDRESS_WRITE 0x17
#define RB7_ADDRESS_READ 0x97

#define RB8_ADDRESS_WRITE 0x18
#define RB8_ADDRESS_READ 0x98

#define RB9_ADDRESS_WRITE 0x19
#define RB9_ADDRESS_READ 0x99

#define RBA_ADDRESS_WRITE 0x1A
#define RBA_ADDRESS_READ 0x9A

#define RBB_ADDRESS_WRITE 0x1B
#define RBB_ADDRESS_READ 0x9B

#define RBC_ADDRESS_WRITE 0x1C
#define RBC_ADDRESS_READ 0x9C

#define RBD_ADDRESS_WRITE 0x1D
#define RBD_ADDRESS_READ 0x9D

#define RBE_ADDRESS_WRITE 0x1E
#define RBE_ADDRESS_READ 0x9E

#define RBF_ADDRESS_WRITE 0x1F
#define RBF_ADDRESS_READ 0x9F

/**@union REG_RBN
 * @brief Represents a Row Brithness Register value (15 of them)
 */
union REG_RBN {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Character white level
     * @li b00 (0) = 120%
     * @li b01 (1) = 100%
     * @li b10 (2) = 90%
     * @li b11 (3) = 80%
     */
    unsigned char characterWhiteLevel : 2;

    /**@brief Character black level
     * @li b00 (0) = 0%
     * @li b01 (1) = 10%
     * @li b10 (2) = 20%
     * @li b11 (3) = 20%
     */
    unsigned char characterBlackLevel : 2;

    /**@brief don't care*/
    unsigned char unused : 4;
  } bits;
};

#define OSDBL_ADDRESS_WRITE 0x6C
#define OSDBL_ADDRESS_READ 0xEC

/**@union REG_OSDBL
 * @brief Represents an OSD Black Level Register value
 */
union REG_OSDBL {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief do not change those bits : factory preset*/
    unsigned char doNotChange : 4;
    /**@brief OSD Image Black Level Control.
     * @li 0 = automatic.
     * @li 1 = manual.
     */
    unsigned char osdImageBlackLevelControl : 1;

    /**@brief don't care*/
    unsigned char unused : 3;
  } bits;
};

#define STAT_ADDRESS_READ 0xA0 //Read only

/**@union REG_STAT
 * @brief Represents a Status Register value
 */
union REG_STAT {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief Detected PAL.
     * @li 0 = PAL signal is not detected ad VIN.
     * @li 1 = PAL signal is detected at VIN.
     */
    unsigned char PALDetected : 1;
    /**@brief Detected NTSC.
     * @li 0 = NTSC signal is not detected ad VIN.
     * @li 1 = NTSC signal is detected at VIN.
     */
    unsigned char NTSCDetected : 1;
    /**@brief Loos-Of-Sync.
     * @li 0 = Sunc Active. Asserted after 32 consecutive input video lines.
     * @li 1 = No Sync. Asserted after 32 consecutive missing input video lines.
     */
    unsigned char LOS : 1;
    /**@brief ~HSYNC Ouput Level.
     * @li 0 = Active during horizontal synch time.
     * @li 1 = inactive otherwise.
     */
    unsigned char NOTHsynchOutputLevel : 1;
    /**@brief ~VSYNC Ouput Level.
     * @li 0 = Active during vertical synch time.
     * @li 1 = inactive otherwise.
     */
    unsigned char NOTVsynchOutputLevel : 1;
    /**@brief Character Memory Status.
     * @li 0 = Available access.
     * @li 1 = Unavailable access.
     */
    unsigned char characterMemoryStatus : 1;
    /**@brief Reset Mode.
     * @li 0 = Clear when power up reset mode is complete.
     * @li 1 = Set when in power-up reset mode.
     */
    unsigned char resetMode : 1;
    /**@brief don't care.*/
    unsigned char unused : 1;
  } bits;
};

#define DMDO_ADDRESS_READ 0xB0

/**@typedef DMDO
 * @brief represents a Display Memory Data Out Register value.
 */
typedef unsigned char DMDO;

#define CMDO_ADDRESS_READ 0xC0

/**@union REG_CMDO
 * @brief Represents a Character Memory Data Out value.
 */
union REG_CMDO {
  /**@brief The whole register value*/
  unsigned char whole;
  /**
   * @var bits
   * @brief access to individual bits*/
  struct {
    /**@brief right most pixel*/
    unsigned char rightMostPowel : 2;
    /**@brief right center pixel*/
    unsigned char rightCenterPixel : 2;
    /**@brief left center pixel*/
    unsigned char leftCenterPixel : 2;
    /**@brief left most pixel*/
    unsigned char leftMostPixel : 2;
  } bits;
};

/**\def COLOR_BLACK
 * \brief Black value for a pixel (2bits)
 */
#define COLOR_BLACK 0

/**\def COLOR_WHITE
 * \brief White value for a pixel (2bits)
 */
#define COLOR_WHITE 2

/**\def COLOR_TRANSPARENT
 * \brief Transparent value for a pixel (2bits)
 */
#define COLOR_TRANSPARENT 1

/**\def COLOR_GREY
 * \brief Grey value for a pixel (2bits)
 */
#define COLOR_GREY COLOR_TRANSPARENT

/**@struct PIXEL
 * @brief represent a 4-pixels value
 */
struct PIXEL {
  /**@brief 4th pixel*/
  byte pix3 : 2;
  /**@brief 3rd pixel*/
  byte pix2 : 2;
  /**@brief 2nd pixel*/
  byte pix1 : 2;
  /**@brief 1st pixel*/
  byte pix0 : 2;
};

/**@union LINE
 * @brief Represents a line in a max7456 character ie. 12 pixels
 */
union LINE {
  /**@brief the whole line*/
  byte whole[3];
  /**@brief individual 4-pixels access*/
  struct PIXEL pixels[3];
};

/**
 * @union CARACT
 * @brief Represents a character with lines and pixels.
 *   example : myCarac.line[3].pixels[2].pix2 = COLOR_TRANSPARENT ;
 */
union CARACT {
  /**@brief the whole CARACT as in max7456 Character Memory*/
  charact whole;
  /**@brief acces with lines*/
  union LINE line[18];
};

enum
{
  _BT_BT = 0,
  _BT_2BT,
  _BT_3BT,
  _3BT_BT
};

enum
{
  _2fields = 0,
  _4fields,
  _6fields,
  _8fields
};

#endif /* MAX7456REGISTERS_H_ */
