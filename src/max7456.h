/**
 * @file
 * max7456.h
 *
 *  Created on: 10 oct. 2012
 *      Author: Benoit
 */

#ifndef MAX7456_H
#define MAX7456_H

/**
 * Define MAX7456_TABLE_ASCII only if you changed your table for an ASCII-like one
 * ie. address 0x00 -> ' '
 *     …
 *     address 0x5A -> 'z'
 */
#define MAX7456_TABLE_ASCII

#include "max7456registers.h"

/**
 *  @mainpage Max7456 Arduino library
 */

/**
 *  @example Max7456WriteTable.cpp
 */

/**
 *  @class Max7456
 *  @brief Represents a max7456 device communicating through SPI port
 */
class Max7456
{
public:
  /**
   *  Default constructor
   */
  Max7456();

  /**
   *  Constructor
   *  Initialize communications and device
   *  @param pinCS : pin ~CS of the arduino where max7456 is plugged.
   */
  Max7456(byte pinCS);

  /**
   *  Initialize communications and device
   *  @param pinCS : pin ~CS of the arduino where max7456 is plugged.
   *  @code
   *  Max7456 osd;
   *  osd.init(9); //Note it's that it's the same than usinge constructor Max7456(byte pinCS).
   *  @endcode
   */
  void init(byte pinCS);

  /**
   * Set the base time for blink.
   * @param blinkBase : the base time (see datasheet)
   * @param blinkDC : blink Duty cycle (see datasheet).
   */
  void setBlinkParams(byte blinkBase, byte blinkDC);

  /**
   * Set Horizontal and Vertical display offset
   * @param horizontal : the horizontal offset in pixels (between 0 and 63).
   * @param vertical : the vertical offset in pixels (between 0 and 31).
   */
  void setDisplayOffsets(byte horizontal, byte vertical);

  /**
   *  Erase Display Memory.
   */
  void clearScreen();

  /**
   *  Activate OSD
   *  @param act :
   *  @li if true OSD is activated
   *  @li if false OSD is deactivated
   */
  void activateOSD(bool act = true);

  /**
   *  Activate input video as a background
   *  @param activExtVid :
   *  @li if true external video is displayed
   *  @li if false external video is not displayed (bakground = grey)
   */
  void activateExternalVideo(bool activExtVid = true);

  /**
   *  Put a character in the memory character of max7456
   *  @param array : the byte array representing the character (54 bytes long)
   *  @param x : the horizontal position of character in memory
   *  @param y : the vertical position of character in memory.
   *  @code
   *  charact c={0x44,....} //Whatever charact here
   *  max.sendCharacter(c,4,5); //put c at 4th line 5th column ie. address 54.
   *  max.sendCharacter(c,0x43); //put c in mem at address 43.
   *  @endcode
   */
  void sendCharacter(const charact array, byte x, byte y);

  /**
   *  Get a character from the character memory of max7456
   *  @param array : the byte array representing the character (54 bytes long)
   *  @param x : the horizontal position of character in character memory
   *  @param y : the vertical position of character in character memory
   */
  void getCharacter(charact array, byte x, byte y);

  /**
   *  Put a string in the display memory of max7456
   *  @param string : The string to be displayed
   *  @param x : the horizontal position of the string on screen (NTSC/PAL: 0-29)
   *  @param y : the vertical position of the string on screen (NTSC: 0-12, PAL: 0-15)
   *  @param blink : if 1 then character will blink
   *  @param inv : if 1 then color character will be inverted
   *  @note : This function assumes you have a ASCII-like charset (not default one)
   *  You must rewrite your Max7456 EEPROM and set part of ASCII
   *  characters first: remap from ' ' (0x20) to 'z' (0x7a) at EEPROM start.
   *  ie. ' ' -> 0x00, z -> 0x5a ('z'-' ').
   */
  void print(const char string[], byte x, byte y, byte blink = 0, byte inv = 0);

  /**
   *  Put a float in the display memory of max7456
   *  @param value : The value to be displayed
   *  @param x : the horizontal position of the value on screen
   *  @param y : the vertical position of the value on screen
   *  @param before : number of digits before comma
   *  @param after : number of digits after comma
   *  @param blink : if 1 then character will blink
   *  @param inv : if 1 then color character will be inverted
   *  @note The number of printed characters will be : before + after + 1.
   *  @note Be aware of having a coherence between the number of digits you
   *  enter in the function and the max. value of your double. If not may
   *  result in unpredictable printed string.
   *  @code
   *  max.print(3.14,x,y,3,4);
   *  //Will print "003.1400" on screen
   *  @endcode
   */
  void print(double value, byte x, byte y, byte before, byte after,
             byte blink = 0, byte inv = 0);

  /**
   *  Put some characters in the display memory of max7456.
   *    The characters are given by their address in the
   *    character memory of the max7456.
   *  @param chars : The characters address array to display
   *  @param size : the array size
   *  @param x : the horizontal position of the value on screen
   *  @param y : the vertical position of the value on screen
   *  @param blink : if 1 then character will blink
   *  @param inv : if 1 then color character will be inverted
   *  @code
   *  char chars[0x04,0x45,0x54]; //the chars addresses array to be send to function.
   *  max.printMax7456Chars(chars,3,x,y);
   *  @endcode
   */
  void printMax7456Chars(byte chars[], byte size, byte x, byte y,
                         byte blink = 0, byte inv = 0);

  /**
   *  Put one character from the character memory in the display memory of max7456
   *  @param address : The address in character memory of the character to be displayed
   *  @param x : the horizontal position of the string on screen
   *  @param y : the vertical position of the string on screen
   *  @param blink : if 1 then character will blink
   *  @param inv : if 1 then color character will be inverted
   */
  void printMax7456Char(const byte address, byte x, byte y, byte blink = 0,
                        byte inv = 0);

  /**
   *  Print a character to Serial port
   *  @param array : the byte array representing the character (54 bytes long)
   *  @param img :
   *  @li true : the character will be displayed as a picture
   *  @li false : the character will be displayed as a byte array
   */
  static void printCharacterToSerial(const charact array, bool img = true);

  /**
   *  Converts a CARACT character to a byte array representation.
   *  @param car : the CARACT character
   *  @return : the byte array representing the character (54 bytes long)
   */
  static byte* CARACT2ByteArray(const CARACT car);

  /**
   *  Converts a byte array to a CARACT character.
   *  @param array : the byte array representing the character (54 bytes long)
   *  @return : the CARACT character
   */
  static CARACT byteArray2CARACT(const charact array);

  /**
   * Get the ith character from program memory
   * @param table the address of the array in prog memory
   * @param i the index of character.
   * @param c the returned character.
   * @note The table must be in program memory as an array of bytes (a multiple of 54).
   * @note When accessing this array, 0x55 are interpreted as 0xFF (you can't have 0xFF in program memory.
   * @note See file example for more informations.
   */
  static void getCARACFromProgMem(const char* table, byte i, charact c);

private:
  byte giveMax7456CharFromAsciiChar(char ascii);
  static void printPixel(byte value);

  byte _pinCS;
  bool _isActivatedOsd;
  //registers (only here for convenience : not forced to be used).
  REG_VM0   _regVm0;
  REG_VM1   _regVm1;
  REG_HOS   _regHos;
  REG_VOS   _regVos;
  REG_DMM   _regDmm;
  REG_DMAH  _regDmah; // not used yet
  REG_DMAL  _regDmal; // not used yet
  REG_DMDI  _regDmdi; // not used yet
  REG_CMM   _regCmm;
  REG_CMAH  _regCmah;   // not used yet
  REG_CMAL  _regCmal;   // not used yet
  REG_CMDI  _regCmdi;   // not used yet
  REG_OSDM  _regOsdm;   // not used yet
  REG_RBN   _regRb[16]; // not used yet
  REG_OSDBL _regOsdbl;  // not used yet
  REG_STAT  _regStat;   // not used yet
  DMDO      _regDmdo;   // not used yet
  REG_CMDO  _regCmdo;   // not used yet
};

#endif /* MAX7456_H_ */
