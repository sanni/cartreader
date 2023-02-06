/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   This project represents a community-driven effort to provide
   an easy to build and easy to modify cartridge dumper.

   Date:             06.02.2023
   Version:          12.2

   SD lib: https://github.com/greiman/SdFat
   LCD lib: https://github.com/olikraus/u8g2
   Neopixel lib: https://github.com/adafruit/Adafruit_NeoPixel
   Rotary Enc lib: https://github.com/mathertel/RotaryEncoder
   SI5351 lib: https://github.com/etherkit/Si5351Arduino
   RTC lib: https://github.com/adafruit/RTClib
   Frequency lib: https://github.com/PaulStoffregen/FreqCount

   Compiled with Arduino IDE 2.0.3

   Thanks to:
   MichlK - ROM Reader for Super Nintendo
   Jeff Saltzman - 4-Way Button
   Wayne and Layne - Video Game Shield menu
   skaman - Cart ROM READER SNES ENHANCED, Famicom Cart Dumper, Coleco-, Intellivision, Virtual Boy, WSV, PCW, ARC, Atari, ODY2, Fairchild modules
   Tamanegi_taro - PCE and Satellaview modules
   splash5 - GBSmart, Wonderswan and NGP modules
   hkz & themanbehindthecurtain - N64 flashram commands
   Andrew Brown & Peter Den Hartog - N64 controller protocol
   libdragon - N64 controller checksum functions
   Angus Gratton - CRC32
   Snes9x - SuperFX sram fix
   insidegadgets - GBCartRead
   RobinTheHood - GameboyAdvanceRomDumper
   Gens-gs - Megadrive checksum
   fceux - iNes header

   And a special Thank You to all coders and contributors on Github and the Arduino forum:
   jiyunomegami, splash5, Kreeblah, ramapcsx2, PsyK0p4T, Dakkaron, majorpbx, Pickle, sdhizumi,
   Uzlopak, sakman55, Tombo89, scrap-a, borti4938, vogelfreiheit, CaitSith2, Modman,
   philenotfound, karimhadjsalem, nsx0r, ducky92, niklasweber, Lesserkuma, BacteriaMage,
   vpelletier, Ancyker, mattiacci, RWeick

   And to nocash for figuring out the secrets of the SFC Nintendo Power cartridge.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

**********************************************************************************/

char ver[5] = "12.2";

//******************************************
// !!! CHOOSE HARDWARE VERSION !!!
//******************************************
// Remove // in front of the line with your hardware version
// #define HW5
// #define HW4
// #define HW3
// #define HW2
// #define HW1
// #define SERIAL_MONITOR

#if !(defined(HW1) || defined(HW2) || defined(HW3) || defined(HW4) || defined(HW5) || defined(SERIAL_MONITOR))
#error !!! PLEASE CHOOSE HARDWARE VERSION !!!
#endif

//******************************************
// ENABLE MODULES
//******************************************
// add/remove // before #define to disable/enable modules you
// don't need/need to save program storage space and dynamic memory
// If you only get an empty or "Press Button" screen after flashing
// you have enabled too many modules

// Atari 2600
//#define enable_ATARI

// Benesse Pocket Challenge W
//#define enable_PCW

// ColecoVision
//#define enable_COLV

// Emerson Arcadia 2001
//#define enable_ARC

// Fairchild Channel F
//#define enable_FAIRCHILD

// Flashrom Programmer for SNES repros
#define enable_FLASH
//#define enable_FLASH16

// Game Boy (Color) and Advance
#define enable_GBX

// Intellivision
//#define enable_INTV

// Neo Geo Pocket
//#define enable_NGP

// Nintendo 64
#define enable_N64

// Nintendo Entertainment System/Family Computer
#define enable_NES

// Magnavox Odyssey 2
//#define enable_ODY2

// PC Engine/TurboGrafx 16
//#define enable_PCE

// Sega Master System
#define enable_SMS

// Sega Mega Drive/Genesis
#define enable_MD

// Super Famicom SF Memory Cassette
#define enable_SFM

// Super Famicom Satellaview
#define enable_SV

// Super Nintendo
#define enable_SNES

// Virtual Boy
//#define enable_VBOY

// Watara Supervision
//#define enable_WSV

// WonderSwan
//#define enable_WS

//******************************************
// HW CONFIGS
//******************************************
#if (defined(HW4) || defined(HW5))
// #define enable_vselect
#define enable_LCD
#define enable_neopixel
#define background_color 100, 0, 0  //Green, Red, Blue
#define enable_rotary
// #define rotate_counter_clockwise
#define clockgen_installed
#define fastcrc
#define ws_adapter_v2
#endif

#if (defined(HW2) || defined(HW3))
#define enable_OLED
#define enable_Button2
#define clockgen_installed
#define CA_LED
//#define fastcrc
#endif

#if defined(HW1)
#define enable_OLED
// #define clockgen_installed
// #define fastcrc
#endif

#if defined(SERIAL_MONITOR)
#define enable_serial
//#define clockgen_installed
//#define fastcrc
#endif

//******************************************
// OPTIONS
//******************************************
// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

// Write all info to OSCR_LOG.txt in root dir
#define global_log

// Renames ROM if found in database
#define nointro

// Ignores errors that normally force a reset if button 2 is pressed
// #define debug_mode

// Setup RTC if installed.
// #define RTC_installed

// Use calibration data from snes_clk.txt
// #define clockgen_calibration

// Use Adafruit Clock Generator
// #define clockgen_installed

// I don't know
//#define use_md_conf

// The CRC for N64 Roms will be calculated during dumping from memory instead of after dumping from SD card, not compatible to all Cart Readers
// #define fastcrc

// saves a n64log.txt file with rom info in /N64/ROM
// #define savesummarytotxt

/******************************************
   Libraries
 *****************************************/
// Basic Libs
#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

// SD Card
#include "SdFat.h"
SdFs sd;
FsFile myFile;
#ifdef global_log
FsFile myLog;
boolean dont_log = false;
#endif

// AVR Eeprom
#include <EEPROM.h>
// forward declarations for "T" (for non Arduino IDE)
template<class T> int EEPROM_writeAnything(int ee, const T& value);
template<class T> int EEPROM_readAnything(int ee, T& value);

// Graphic SPI LCD
#ifdef enable_LCD
#include <U8g2lib.h>
U8G2_ST7567_OS12864_F_4W_HW_SPI display(U8G2_R2, /* cs=*/12, /* dc=*/11, /* reset=*/10);
#endif

// Rotary Encoder
#ifdef enable_rotary
#include <RotaryEncoder.h>
#define PIN_IN1 18
#define PIN_IN2 19
#ifdef rotate_counter_clockwise
RotaryEncoder encoder(PIN_IN2, PIN_IN1, RotaryEncoder::LatchMode::FOUR3);
#else
RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);
#endif
int rotaryPos = 0;
#endif

// Choose RGB LED type
#ifdef enable_neopixel
// Neopixel
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(3, 13, NEO_GRB + NEO_KHZ800);
#endif

typedef enum COLOR_T {
  blue_color,
  red_color,
  purple_color,
  green_color,
  turquoise_color,
  yellow_color,
  white_color,
} color_t;

// Graphic I2C OLED
#ifdef enable_OLED
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
#endif

// Adafruit Clock Generator
#include <si5351.h>
Si5351 clockgen;
bool i2c_found;

// RTC Library
#ifdef RTC_installed
#define _RTC_H
#include "RTClib.h"
#endif

// Clockgen Calibration
#ifdef clockgen_calibration
#include "FreqCount.h"
#endif

void _print_FatalError(void) __attribute__((noreturn));
void print_FatalError(const __FlashStringHelper* errorMessage) __attribute__((noreturn));
void print_FatalError(byte errorMessage) __attribute__((noreturn));

/******************************************
 End of inclusions and forward declarations
 *****************************************/

template<class T> int EEPROM_writeAnything(int ee, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template<class T> int EEPROM_readAnything(int ee, T& value) {
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

/******************************************
  Common Strings
 *****************************************/
#define press_button_STR 0
#define sd_error_STR 1
#define reset_STR 2
#define did_not_verify_STR 3
#define _bytes_STR 4
#define error_STR 5
#define create_file_STR 6
#define open_file_STR 7
#define file_too_big_STR 8
#define done_STR 9
#define saving_to_STR 10
#define verifying_STR 11
#define flashing_file_STR 12
#define press_to_change_STR 13
#define right_to_select_STR 14
#define rotate_to_change_STR 15
#define press_to_select_STR 16

// This arrays holds the most often uses strings
static const char string_press_button0[] PROGMEM = "Press Button...";
static const char string_sd_error1[] PROGMEM = "SD Error";
static const char string_reset2[] PROGMEM = "Reset";
static const char string_did_not_verify3[] PROGMEM = "did not verify";
static const char string_bytes4[] PROGMEM = " bytes ";
static const char string_error5[] PROGMEM = "Error: ";
static const char string_create_file6[] PROGMEM = "Can't create file";
static const char string_open_file7[] PROGMEM = "Can't open file";
static const char string_file_too_big8[] PROGMEM = "File too big";
static const char string_done9[] PROGMEM = "Done";
static const char string_saving_to10[] PROGMEM = "Saving to ";
static const char string_verifying11[] PROGMEM = "Verifying...";
static const char string_flashing_file12[] PROGMEM = "Flashing file ";
static const char string_press_to_change13[] PROGMEM = "Press left to Change";
static const char string_right_to_select14[] PROGMEM = "and right to Select";
static const char string_rotate_to_change15[] PROGMEM = "Rotate to Change";
static const char string_press_to_select16[] PROGMEM = "Press to Select";

static const char* const string_table[] PROGMEM = { string_press_button0, string_sd_error1, string_reset2, string_did_not_verify3, string_bytes4, string_error5, string_create_file6, string_open_file7, string_file_too_big8, string_done9, string_saving_to10, string_verifying11, string_flashing_file12, string_press_to_change13, string_right_to_select14, string_rotate_to_change15, string_press_to_select16 };

void print_STR(byte string_number, boolean newline) {
  char string_buffer[22];
  strcpy_P(string_buffer, (char*)pgm_read_word(&(string_table[string_number])));
  if (newline)
    println_Msg(string_buffer);
  else
    print_Msg(string_buffer);
}

/******************************************
  Defines
 *****************************************/
// Mode menu
#define mode_N64_Cart 0
#define mode_N64_Controller 1
#define mode_SNES 2
#define mode_SFM 3
#define mode_SFM_Flash 4
#define mode_SFM_Game 5
#define mode_GB 6
#define mode_FLASH8 7
#define mode_FLASH16 8
#define mode_GBA 9
#define mode_GBM 10
#define mode_MD_Cart 11
#define mode_EPROM 12
#define mode_PCE 13
#define mode_SV 14
#define mode_NES 15
#define mode_SMS 16
#define mode_SEGA_CD 17
#define mode_GB_GBSmart 18
#define mode_GB_GBSmart_Flash 19
#define mode_GB_GBSmart_Game 20
#define mode_WS 21
#define mode_NGP 22
#define mode_INTV 23
#define mode_COL 24
#define mode_VBOY 25
#define mode_WSV 26
#define mode_PCW 27
#define mode_ATARI 28
#define mode_ODY2 29
#define mode_ARC 30
#define mode_FAIRCHILD 31

// optimization-safe nop delay
#define NOP __asm__ __volatile__("nop\n\t")

// Button timing
#define debounce 20        // ms debounce period to prevent flickering when pressing or releasing the button
#define DCgap 250          // max ms between clicks for a double click event
#define holdTime 2000      // ms hold period: how long to wait for press+hold event
#define longHoldTime 5000  // ms long hold period: how long to wait for press+hold event

/******************************************
   Variables
 *****************************************/
#ifdef enable_rotary
// Button debounce
boolean buttonState = HIGH;          // the current reading from the input pin
boolean lastButtonState = HIGH;      // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
#endif

#ifdef enable_OLED
// Button 1
boolean buttonVal1 = HIGH;           // value read from button
boolean buttonLast1 = HIGH;          // buffered value of the button's previous state
boolean DCwaiting1 = false;          // whether we're waiting for a double click (down)
boolean DConUp1 = false;             // whether to register a double click on next release, or whether to wait and click
boolean singleOK1 = true;            // whether it's OK to do a single click
long downTime1 = -1;                 // time the button was pressed down
long upTime1 = -1;                   // time the button was released
boolean ignoreUp1 = false;           // whether to ignore the button release because the click+hold was triggered
boolean waitForUp1 = false;          // when held, whether to wait for the up event
boolean holdEventPast1 = false;      // whether or not the hold event happened already
boolean longholdEventPast1 = false;  // whether or not the long hold event happened already
// Button 2
boolean buttonVal2 = HIGH;           // value read from button
boolean buttonLast2 = HIGH;          // buffered value of the button's previous state
boolean DCwaiting2 = false;          // whether we're waiting for a double click (down)
boolean DConUp2 = false;             // whether to register a double click on next release, or whether to wait and click
boolean singleOK2 = true;            // whether it's OK to do a single click
long downTime2 = -1;                 // time the button was pressed down
long upTime2 = -1;                   // time the button was released
boolean ignoreUp2 = false;           // whether to ignore the button release because the click+hold was triggered
boolean waitForUp2 = false;          // when held, whether to wait for the up event
boolean holdEventPast2 = false;      // whether or not the hold event happened already
boolean longholdEventPast2 = false;  // whether or not the long hold event happened already
#endif

#ifdef enable_serial
// For incoming serial data
int incomingByte;
#endif

// Variables for the menu
int choice = 0;
// Temporary array that holds the menu option read out of progmem
char menuOptions[7][20];
boolean ignoreError = 0;

// File browser
#define FILENAME_LENGTH 100
#define FILEPATH_LENGTH 132
#define FILEOPTS_LENGTH 20

char fileName[FILENAME_LENGTH];
char filePath[FILEPATH_LENGTH];
byte currPage;
byte lastPage;
byte numPages;
boolean root = 0;
boolean filebrowse = 0;

// Common
// 21 chars for SNES ROM name, one char for termination
char romName[22];
unsigned long sramSize = 0;
int romType = 0;
byte saveType;
word romSize = 0;
word numBanks = 128;
char checksumStr[9];
bool errorLvl = 0;
byte romVersion = 0;
char cartID[5];
unsigned long cartSize;
unsigned int flashid;
char flashid_str[5];
char vendorID[5];
unsigned long fileSize;
unsigned long sramBase;
unsigned long flashBanks;
bool flashX16Mode;
bool flashSwitchLastBits;

// Variable to count errors
unsigned long writeErrors;

// Operation mode
byte mode;

//remember folder number to create a new folder for every game
int foldern;
// 4 chars for console type, 4 chars for SAVE/ROM, 21 chars for ROM name, 4 chars for folder number, 3 chars for slashes, one char for termination, one char savety
char folder[38];

// Array that holds the data
byte sdBuffer[512];

// soft reset Arduino: jumps to 0
// using the watchdog timer would be more elegant but some Mega2560 bootloaders are buggy with it
void (*resetArduino)(void) __attribute__((noreturn)) = 0;

// Progressbar
void draw_progressbar(uint32_t processedsize, uint32_t totalsize);

// used by MD and NES modules
byte eepbit[8];
byte eeptemp;

#ifdef nointro
// Array to hold iNES header
byte iNES_HEADER[16];
//ID 0-3
//ROM_size 4
//VROM_size 5
//ROM_type 6
//ROM_type2 7
//ROM_type3 8
//Upper_ROM_VROM_size 9
//RAM_size 10
//VRAM_size 11
//TV_system 12
//VS_hardware 13
//reserved 14, 15
#endif

//******************************************
// CRC32
//******************************************
// CRC32 lookup table // 256 entries
static const uint32_t crc_32_tab[] PROGMEM = { /* CRC polynomial 0xedb88320 */
                                               0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
                                               0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
                                               0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
                                               0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
                                               0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
                                               0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
                                               0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
                                               0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
                                               0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
                                               0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
                                               0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
                                               0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
                                               0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
                                               0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
                                               0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
                                               0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
                                               0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
                                               0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
                                               0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
                                               0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
                                               0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
                                               0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
                                               0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
                                               0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
                                               0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
                                               0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
                                               0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
                                               0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
                                               0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
                                               0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
                                               0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
                                               0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
                                               0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
                                               0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
                                               0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
                                               0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
                                               0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
                                               0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
                                               0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
                                               0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
                                               0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
                                               0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
                                               0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

// Defined as a macros, as compiler disregards inlining requests and these are
// performance-critical functions.
#define UPDATE_CRC(crc, ch) \
  do { \
    uint8_t idx = ((crc) ^ (ch)) & 0xff; \
    uint32_t tab_value = pgm_read_dword(crc_32_tab + idx); \
    (crc) = tab_value ^ ((crc) >> 8); \
  } while (0)

uint32_t updateCRC(const byte* buffer, size_t length, uint32_t crc) {
  for (size_t c = 0; c < length; c++) {
    UPDATE_CRC(crc, buffer[c]);
  }
  return crc;
}

uint32_t calculateCRC(const byte* buffer, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  crc = updateCRC(buffer, length, crc);
  return ~crc;
}

uint32_t calculateCRC(FsFile& infile) {
  uint32_t byte_count;
  uint32_t crc = 0xFFFFFFFF;

  while ((byte_count = infile.read(sdBuffer, sizeof(sdBuffer))) != 0) {
    crc = updateCRC(sdBuffer, byte_count, crc);
  }
  return ~crc;
}

// Calculate rom's CRC32 from SD
uint32_t calculateCRC(char* fileName, char* folder, int offset) {
  FsFile infile;
  uint32_t result;

  sd.chdir(folder);
  if (infile.open(fileName, O_READ)) {
    infile.seek(offset);
    result = calculateCRC(infile);
    infile.close();
    return result;
  } else {
    display_Clear();
    print_Msg(F("File "));
    //print_Msg(folder);
    //print_Msg(F("/"));
    //print_Msg(fileName);
    print_FatalError(F(" not found"));
    return 0;
  }
}

/******************************************
   CRC Functions for Atari, Fairchild, Ody2, Arc modules
 *****************************************/
#if (defined(enable_ATARI) || defined(enable_ODY2) || defined(enable_ARC) || defined(enable_FAIRCHILD))

inline uint32_t updateCRC(uint8_t ch, uint32_t crc) {
  uint32_t idx = ((crc) ^ (ch)) & 0xff;
  uint32_t tab_value = pgm_read_dword(crc_32_tab + idx);
  return tab_value ^ ((crc) >> 8);
}

FsFile crcFile;
char tempCRC[9];

uint32_t crc32(FsFile& file, uint32_t& charcnt) {
  uint32_t oldcrc32 = 0xFFFFFFFF;
  charcnt = 0;
  while (file.available()) {
    crcFile.read(sdBuffer, 512);
    for (int x = 0; x < 512; x++) {
      uint8_t c = sdBuffer[x];
      charcnt++;
      oldcrc32 = updateCRC(c, oldcrc32);
    }
  }
  return ~oldcrc32;
}

void calcCRC(char* checkFile, unsigned long filesize, uint32_t* crcCopy, unsigned long offset) {
  uint32_t crc;
  crcFile = sd.open(checkFile);
  crcFile.seek(offset);
  crc = crc32(crcFile, filesize);
  crcFile.close();
  sprintf(tempCRC, "%08lX", crc);

  if (crcCopy != NULL) {
    *crcCopy = crc;
  }

  print_Msg(F("CRC: "));
  println_Msg(tempCRC);
  display_Update();
}
#endif

//******************************************
// Functions for CRC32 database
//******************************************
//Skip line
void skip_line(FsFile* readfile) {
  int i = 0;
  char str_buf;

  while (readfile->available()) {
    //Read 1 byte from file
    str_buf = readfile->read();

    //if end of file or newline found, execute command
    if (str_buf == '\r') {
      readfile->read();  //dispose \n because \r\n
      break;
    }
    i++;
  }  //End while
}

//Get line from file
void get_line(char* str_buf, FsFile* readfile, uint8_t maxi) {
  int read_len;

  // Status LED on
  statusLED(true);

  read_len = readfile->read(str_buf, maxi - 1);

  for (int i = 0; i < read_len; i++) {
    //if end of file or newline found, execute command
    if (str_buf[i] == '\r') {
      str_buf[i] = 0;
      readfile->seekCur(i - read_len + 2);  // +2 to skip over \n because \r\n
      return;
    }
  }
  str_buf[maxi - 1] = 0;
  // EOL was not found, keep looking (slower)
  while (readfile->available()) {
    if (readfile->read() == '\r') {
      readfile->read();  // read \n because \r\n
      break;
    }
  }
}

void rewind_line(FsFile& readfile, byte count = 1) {
  uint32_t position = readfile.curPosition();
  // To seek one line back, this code must step over the first newline it finds
  // in order to exit the current line and enter the end of the previous one.
  // Convert <count> from how-many-lines-back into how-many-newlines-to-look-for
  // by incrementing it by 1.
  count++;
  for (byte count_newline = 0; count_newline < count; count_newline++) {
    // Go to the strictly previous '\n', or file start.
    while (position) {
      // Seek back first (keeping position updated)...
      position--;
      readfile.seekCur(-1);
      // ...and check current byte second.
      // Note: this code assumed all files use ASCII with DOS-style newlines
      // so \n is encountered first when seeking backwards.
      if (readfile.peek() == '\n')
        break;
    }
  }
  // If not at file start, the current character is the '\n' just before the
  // desired line, so advance by one.
  if (position)
    readfile.seekCur(1);
}

// Calculate CRC32 if needed and compare it to CRC read from database
boolean compareCRC(const char* database, char* crcString, boolean renamerom, int offset) {
#ifdef nointro
  char crcStr[9];
  if (crcString == 0) {
    //go to root
    sd.chdir();
    // Calculate CRC32
    print_Msg(F("CRC32... "));
    display_Update();
    sprintf(crcStr, "%08lX", calculateCRC(fileName, folder, offset));
  } else {
    // Use precalculated crc
    print_Msg(F("CRC32... "));
    strcpy(crcStr, crcString);
  }
  // Print checksum
  print_Msg(crcStr);
  display_Update();

  //Search for CRC32 in file
  char gamename[96];
  char crc_search[9];

  //go to root
  sd.chdir();
  if (myFile.open(database, O_READ)) {
    //Search for same CRC in list
    while (myFile.available()) {
      //Read 2 lines (game name and CRC)
      get_line(gamename, &myFile, sizeof(gamename));
      get_line(crc_search, &myFile, sizeof(crc_search));
      skip_line(&myFile);  //Skip every 3rd line

      //if checksum search successful, rename the file and end search
      if (strcmp(crc_search, crcStr) == 0) {
#ifdef enable_NES
        if ((mode == mode_NES) && (offset != 0)) {
          // Rewind to iNES Header
          myFile.seekCur(-36);

          char iNES_STR[33];
          // Read iNES header
          get_line(iNES_STR, &myFile, 33);

          // Convert "4E4553" to (0x4E, 0x45, 0x53)
          unsigned int iNES_BUF;
          for (byte j = 0; j < 16; j++) {
            sscanf(iNES_STR + j * 2, "%2X", &iNES_BUF);
            iNES_HEADER[j] = iNES_BUF;
          }
          //Skip CRLF
          myFile.seekCur(4);
        }
#endif  // enable_NES

        // Close the file:
        myFile.close();

        //Write iNES header
#ifdef enable_NES
        if ((mode == mode_NES) && (offset != 0)) {
          // Write iNES header
          sd.chdir(folder);
          if (!myFile.open(fileName, O_RDWR)) {
            print_FatalError(sd_error_STR);
          }
          for (byte z = 0; z < 16; z++) {
            myFile.write(iNES_HEADER[z]);
          }
          myFile.close();
        }
#endif  // enable_NES
        print_Msg(F(" -> "));
        display_Update();

        if (renamerom) {
          println_Msg(gamename);

          // Rename file to nointro
          sd.chdir(folder);
          delay(100);
          if (myFile.open(fileName, O_READ)) {
            myFile.rename(gamename);
            // Close the file:
            myFile.close();
          }
        } else {
          println_Msg("OK");
        }
        return 1;
        break;
      }
    }
    if (strcmp(crc_search, crcStr) != 0) {
      println_Msg(F(" -> Not found"));
      return 0;
    }
  } else {
    println_Msg(F(" -> Error"));
    println_Msg(F("Database missing"));
    return 0;
  }
#else   // nointro
  println_Msg("");
#endif  // !nointro
  return 0;
}

byte starting_letter() {
#ifdef global_log
  // Disable log to prevent unnecessary logging
  dont_log = true;
#endif

#if (defined(enable_LCD) || defined(enable_OLED))
  byte selection = 0;
  byte line = 0;

  display_Clear();

  println_Msg(F("[#] [A] [B] [C] [D] [E] [F]"));
  println_Msg(F(""));
  println_Msg(F("[G] [H] [ I ] [J] [K] [L] [M]"));
  println_Msg(F(""));
  println_Msg(F("[N] [O] [P] [Q] [R] [S] [T]"));
  println_Msg(F(""));
  println_Msg(F("[U] [V] [W] [X] [Y] [Z] [?]"));

  // Draw selection line
  display.setDrawColor(1);
  display.drawLine(4 + selection * 16, 10 + line * 16, 9 + selection * 16, 10 + line * 16);
  display_Update();

  while (1) {
    int b = checkButton();
    if (b == 2) {  // Previous
      if ((selection == 0) && (line > 0)) {
        line--;
        selection = 6;
      } else if ((selection == 0) && (line == 0)) {
        line = 3;
        selection = 6;
      } else if (selection > 0) {
        selection--;
      }
      display.setDrawColor(0);
      display.drawLine(0, 10 + 0 * 16, 128, 10 + 0 * 16);
      display.drawLine(0, 10 + 1 * 16, 128, 10 + 1 * 16);
      display.drawLine(0, 10 + 2 * 16, 128, 10 + 2 * 16);
      display.drawLine(0, 10 + 3 * 16, 128, 10 + 3 * 16);
      display.setDrawColor(1);
      display.drawLine(4 + selection * 16, 10 + line * 16, 9 + selection * 16, 10 + line * 16);
      display_Update();

    }

    else if (b == 1) {  // Next
      if ((selection == 6) && (line < 3)) {
        line++;
        selection = 0;
      } else if ((selection == 6) && (line == 3)) {
        line = 0;
        selection = 0;
      } else if (selection < 6) {
        selection++;
      }
      display.setDrawColor(0);
      display.drawLine(0, 10 + 0 * 16, 128, 10 + 0 * 16);
      display.drawLine(0, 10 + 1 * 16, 128, 10 + 1 * 16);
      display.drawLine(0, 10 + 2 * 16, 128, 10 + 2 * 16);
      display.drawLine(0, 10 + 3 * 16, 128, 10 + 3 * 16);
      display.setDrawColor(1);
      display.drawLine(4 + selection * 16, 10 + line * 16, 9 + selection * 16, 10 + line * 16);
      display_Update();
    }

    else if (b == 3) {  // Long Press - Execute
      if ((selection + line * 7) != 27) {
        display_Clear();
        println_Msg(F("Please wait..."));
        display_Update();
      }
      break;
    }
  }
  return (selection + line * 7);
#elif defined(SERIAL_MONITOR)
  Serial.println(F("Enter first letter: "));
  while (Serial.available() == 0) {
  }

  // Read the incoming byte:
  byte incomingByte = Serial.read();
  return incomingByte;
#endif

#ifdef global_log
  // Enable log again
  dont_log = false;
#endif
}

void print_MissingModule(void) {
  display_Clear();
  println_Msg(F("Please enable module"));
  print_FatalError(F("in Cart_Reader.ino."));
}

/******************************************
  Main menu optimized for rotary encoder
*****************************************/
#if defined(enable_LCD)
// Main menu
static const char modeItem1[] PROGMEM = "Game Boy";
static const char modeItem2[] PROGMEM = "NES/Famicom";
static const char modeItem3[] PROGMEM = "Super Nintendo/SFC";
static const char modeItem4[] PROGMEM = "Nintendo 64 (3V)";
static const char modeItem5[] PROGMEM = "Mega Drive/Genesis";
static const char modeItem6[] PROGMEM = "SMS/GG/MIII/SG-1000";
static const char modeItem7[] PROGMEM = "PC Engine/TG16";
static const char modeItem8[] PROGMEM = "WonderSwan";
static const char modeItem9[] PROGMEM = "NeoGeo Pocket";
static const char modeItem10[] PROGMEM = "Intellvision";
static const char modeItem11[] PROGMEM = "Colecovision";
static const char modeItem12[] PROGMEM = "Virtual Boy";
static const char modeItem13[] PROGMEM = "Watara Supervision";
static const char modeItem14[] PROGMEM = "Pocket Challenge W";
static const char modeItem15[] PROGMEM = "Atari 2600";
static const char modeItem16[] PROGMEM = "Magnavox Odyssey 2";
static const char modeItem17[] PROGMEM = "Arcadia 2001";
static const char modeItem18[] PROGMEM = "Fairchild Channel F";
static const char modeItem19[] PROGMEM = "Flashrom Programmer";
static const char modeItem20[] PROGMEM = "About";
static const char* const modeOptions[] PROGMEM = { modeItem1, modeItem2, modeItem3, modeItem4, modeItem5, modeItem6, modeItem7, modeItem8, modeItem9, modeItem10, modeItem11, modeItem12, modeItem13, modeItem14, modeItem15, modeItem16, modeItem17, modeItem18, modeItem19, modeItem20 };

// All included slots
void mainMenu() {
  // create menu with title and 20 options to choose from
  unsigned char modeMenu;
  byte num_answers;
  byte option_offset;

  // Main menu spans across three pages
  currPage = 1;
  lastPage = 1;
  numPages = 3;

  while (1) {
    if (currPage == 1) {
      option_offset = 0;
      num_answers = 7;
    } else if (currPage == 2) {
      option_offset = 7;
      num_answers = 7;
    } else {  // currPage == 3
      option_offset = 14;
      num_answers = 6;
    }
    // Copy menuOptions out of progmem
    convertPgm(modeOptions + option_offset, num_answers);
    modeMenu = question_box(F("OPEN SOURCE CART READER"), menuOptions, num_answers, 0);
    if (numPages == 0) {
      // Execute choice
      modeMenu += option_offset;
      break;
    }
  }

  // Reset page number
  currPage = 1;

  // wait for user choice to come back from the question box menu
  switch (modeMenu) {
#ifdef enable_GBX
    case 0:
      gbxMenu();
      break;
#endif

#ifdef enable_NES
    case 1:
      mode = mode_NES;
      display_Clear();
      display_Update();
      setup_NES();
#ifdef nointro
      getMapping();
#endif
      checkStatus_NES();
      nesMenu();
      break;
#endif

#ifdef enable_SNES
    case 2:
      snsMenu();
      break;
#endif

#ifdef enable_N64
    case 3:
      n64Menu();
      break;
#endif

#ifdef enable_MD
    case 4:
      mdMenu();
      break;
#endif

#ifdef enable_SMS
    case 5:
      smsMenu();
      break;
#endif

#ifdef enable_PCE
    case 6:
      pcsMenu();
      break;
#endif

#ifdef enable_WS
    case 7:
      display_Clear();
      display_Update();
      setup_WS();
      mode = mode_WS;
      break;
#endif

#ifdef enable_NGP
    case 8:
      display_Clear();
      display_Update();
      setup_NGP();
      mode = mode_NGP;
      break;
#endif

#ifdef enable_INTV
    case 9:
      setup_INTV();
      intvMenu();
      break;
#endif

#ifdef enable_COLV
    case 10:
      setup_COL();
      colMenu();
      break;
#endif

#ifdef enable_VBOY
    case 11:
      setup_VBOY();
      vboyMenu();
      break;
#endif

#ifdef enable_WSV
    case 12:
      setup_WSV();
      wsvMenu();
      break;
#endif

#ifdef enable_PCW
    case 13:
      setup_PCW();
      pcwMenu();
      break;
#endif

#ifdef enable_ATARI
    case 14:
      setup_ATARI();
      atariMenu();
      break;
#endif

#ifdef enable_ODY2
    case 15:
      setup_ODY2();
      ody2Menu();
      break;
#endif

#ifdef enable_ARC
    case 16:
      setup_ARC();
      arcMenu();
      break;
#endif

#ifdef enable_FAIRCHILD
    case 17:
      setup_FAIRCHILD();
      fairchildMenu();
      break;
#endif

#ifdef enable_FLASH
    case 18:
#ifdef enable_FLASH16
      flashMenu();
#else
      flashromMenu8();
#endif
      break;
#endif

    case 19:
      aboutScreen();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

/******************************************
  Main menu optimized for buttons
*****************************************/
#else
// Main menu
static const char modeItem1[] PROGMEM = "Add-ons";
#if defined(clockgen_installed)
static const char modeItem2[] PROGMEM = "SNES/SFC (CLK0+1)";
#else
static const char modeItem2[] PROGMEM = "Super Nintendo/SFC";
#endif
static const char modeItem3[] PROGMEM = "Mega Drive/Genesis";
static const char modeItem4[] PROGMEM = "N64 (3V EEP CLK1)";
static const char modeItem5[] PROGMEM = "Game Boy";
static const char modeItem6[] PROGMEM = "About";
// static const char modeItem7[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const modeOptions[] PROGMEM = { modeItem1, modeItem2, modeItem3, modeItem4, modeItem5, modeItem6, string_reset2 };

// Add-ons submenu
static const char addonsItem1[] PROGMEM = "70s Consoles";
static const char addonsItem2[] PROGMEM = "80s Consoles";
static const char addonsItem3[] PROGMEM = "Handhelds";
static const char addonsItem4[] PROGMEM = "Flashrom Programmer";
//static const char addonsItem5[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const addonsOptions[] PROGMEM = { addonsItem1, addonsItem2, addonsItem3, addonsItem4, string_reset2 };

// 70s Consoles submenu
static const char consoles70Item1[] PROGMEM = "Atari 2600";
static const char consoles70Item2[] PROGMEM = "Magnavox Odyssey 2";
static const char consoles70Item3[] PROGMEM = "Arcadia 2001";
static const char consoles70Item4[] PROGMEM = "Fairchild Channel F";
static const char consoles70Item5[] PROGMEM = "Intellivision";
static const char consoles70Item6[] PROGMEM = "Colecovision";
//static const char consoles70Item7[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const consoles70Options[] PROGMEM = { consoles70Item1, consoles70Item2, consoles70Item3, consoles70Item4, consoles70Item5, consoles70Item6, string_reset2 };

// 80s Consoles submenu
static const char consoles80Item1[] PROGMEM = "NES/Famicom";
static const char consoles80Item2[] PROGMEM = "PC Engine/TG16";
static const char consoles80Item3[] PROGMEM = "SMS/GG/MIII/SG-1000";
//static const char consoles80Item4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const consoles80Options[] PROGMEM = { consoles80Item1, consoles80Item2, consoles80Item3, string_reset2 };

// Handhelds submenu
static const char handheldsItem1[] PROGMEM = "Virtual Boy";
static const char handheldsItem2[] PROGMEM = "WonderSwan";
static const char handheldsItem3[] PROGMEM = "NeoGeo Pocket";
static const char handheldsItem4[] PROGMEM = "Watara Supervision";
static const char handheldsItem5[] PROGMEM = "Pocket Challenge W";
//static const char handheldsItem6[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const handheldsOptions[] PROGMEM = { handheldsItem1, handheldsItem2, handheldsItem3, handheldsItem4, handheldsItem5, string_reset2 };

// All included slots
void mainMenu() {
  // create menu with title and 6 options to choose from
  unsigned char modeMenu;
  // Copy menuOptions out of progmem
  convertPgm(modeOptions, 7);
  modeMenu = question_box(F("OPENSOURCE CARTREADER"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (modeMenu) {
    case 0:
      addonMenu();
      break;

#ifdef enable_SNES
    case 1:
      snsMenu();
      break;
#endif

#ifdef enable_MD
    case 2:
      mdMenu();
      break;
#endif

#ifdef enable_N64
    case 3:
      n64Menu();
      break;
#endif

#ifdef enable_GBX
    case 4:
      gbxMenu();
      break;
#endif

    case 5:
      aboutScreen();
      break;

    case 6:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

// Everything that needs an adapter
void addonMenu() {
  // create menu with title and 5 options to choose from
  unsigned char addonsMenu;
  // Copy menuOptions out of progmem
  convertPgm(addonsOptions, 5);
  addonsMenu = question_box(F("Type"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (addonsMenu) {
    // 70s Consoles
    case 0:
      consoles70Menu();
      break;

      // 80s Consoles
    case 1:
      consoles80Menu();
      break;

    // Handhelds
    case 2:
      handheldMenu();
      break;

#ifdef enable_FLASH
    case 3:
#ifdef enable_FLASH16
      flashMenu();
#else
      flashromMenu8();
#endif
      break;
#endif

    case 4:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

// Everything that needs an adapter
void consoles70Menu() {
  // create menu with title and 7 options to choose from
  unsigned char consoles70Menu;
  // Copy menuOptions out of progmem
  convertPgm(consoles70Options, 7);
  consoles70Menu = question_box(F("Choose Adapter"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (consoles70Menu) {
#ifdef enable_ATARI
    case 0:
      setup_ATARI();
      atariMenu();
      break;
#endif

#ifdef enable_ODY2
    case 1:
      setup_ODY2();
      ody2Menu();
      break;
#endif

#ifdef enable_ARC
    case 2:
      setup_ARC();
      arcMenu();
      break;
#endif

#ifdef enable_FAIRCHILD
    case 3:
      setup_FAIRCHILD();
      fairchildMenu();
      break;
#endif

#ifdef enable_INTV
    case 4:
      setup_INTV();
      intvMenu();
      break;
#endif

#ifdef enable_COLV
    case 5:
      setup_COL();
      colMenu();
      break;
#endif

    case 6:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

// Everything that needs an adapter
void consoles80Menu() {
  // create menu with title and 6 options to choose from
  unsigned char consoles80Menu;
  // Copy menuOptions out of progmem
  convertPgm(consoles80Options, 4);
  consoles80Menu = question_box(F("Choose Adapter"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (consoles80Menu) {
#ifdef enable_NES
    case 0:
      mode = mode_NES;
      display_Clear();
      display_Update();
      setup_NES();
#ifdef nointro
      getMapping();
#endif
      checkStatus_NES();
      nesMenu();
      break;
#endif

#ifdef enable_PCE
    case 1:
      pcsMenu();
      break;
#endif

#ifdef enable_SMS
    case 2:
      smsMenu();
      break;
#endif

    case 3:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}

// Everything that needs an adapter
void handheldMenu() {
  // create menu with title and 6 options to choose from
  unsigned char handheldsMenu;
  // Copy menuOptions out of progmem
  convertPgm(handheldsOptions, 6);
  handheldsMenu = question_box(F("Choose Adapter"), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (handheldsMenu) {
#ifdef enable_VBOY
    case 0:
      setup_VBOY();
      vboyMenu();
      break;
#endif

#ifdef enable_WS
    case 1:
      display_Clear();
      display_Update();
      setup_WS();
      mode = mode_WS;
      break;
#endif

#ifdef enable_NGP
    case 2:
      display_Clear();
      display_Update();
      setup_NGP();
      mode = mode_NGP;
      break;
#endif

#ifdef enable_WSV
    case 3:
      setup_WSV();
      wsvMenu();
      break;
#endif

#ifdef enable_PCW
    case 4:
      setup_PCW();
      pcwMenu();
      break;
#endif

    case 5:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}
#endif

/******************************************
  About Screen
*****************************************/
// Info Screen
void aboutScreen() {
  display_Clear();
  println_Msg(F("Cartridge Reader"));
  println_Msg(F("github.com/sanni"));
  print_Msg(F("2023 Version "));
  println_Msg(ver);
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();

  while (1) {

#if (defined(enable_LCD) || defined(enable_OLED))
    // get input button
    int b = checkButton();

    // if the cart readers input button is pressed shortly
    if (b == 1) {
      resetArduino();
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      resetArduino();
    }

    // if the button is pressed super long
    if (b == 4) {
      display_Clear();
      println_Msg(F("Resetting folder..."));
      display_Update();
      delay(2000);
      foldern = 0;
      EEPROM_writeAnything(0, foldern);
      resetArduino();
    }
#elif defined(enable_serial)
    wait_serial();
    resetArduino();
#endif
  }
}

/******************************************
  Progressbar
*****************************************/
void draw_progressbar(uint32_t processed, uint32_t total) {
  uint8_t current, i;
  static uint8_t previous;
  uint8_t steps = 20;

  //Find progressbar length and draw if processed size is not 0
  if (processed == 0) {
    previous = 0;
    print_Msg(F("["));
    display_Update();
    return;
  }

  // Progress bar
  current = (processed >= total) ? steps : processed / (total / steps);

  //Draw "*" if needed
  if (current > previous) {
    for (i = previous; i < current; i++) {
      // steps are 20, so 20 - 1 = 19.
      if (i == (19)) {
        //If end of progress bar, finish progress bar by drawing "]"
        println_Msg(F("]"));
      } else {
        print_Msg(F("*"));
      }
    }
    //update previous "*" status
    previous = current;
    //Update display
    display_Update();
  }
}

/******************************************
  RTC Module
*****************************************/
#ifdef RTC_installed
RTC_DS3231 rtc;

// Start Time
void RTCStart() {
  // Start RTC
  if (!rtc.begin()) {
    abort();
  }

  // Set RTC Date/Time of Sketch Build if it lost battery power
  // After initial setup it would have lost battery power ;)
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Set Date/Time Callback Funtion
// Callback for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // Return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


/******************************************
  RTC Time Stamp Setup
  Call in any other script
*****************************************/
// Format a Date/Time stamp
String RTCStamp() {
  // Set a format
  char dtstamp[] = "DDMMMYYYY hh:mm:ssAP";

  // Get current Date/Time
  DateTime now = rtc.now();

  // Convert it to a string and caps lock it
  String dts = now.toString(dtstamp);
  dts.toUpperCase();

  // Print results
  return dts;
}
#endif

/******************************************
   Clockgen Calibration
 *****************************************/
#ifdef clockgen_calibration
int32_t cal_factor = 0;
int32_t old_cal = 0;
int32_t cal_offset = 100;

void clkcal() {
  // Adafruit Clock Generator
  // last number is the clock correction factor which is custom for each clock generator
  cal_factor = readClockOffset();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Read correction: ");
  display.println(cal_factor);
  display.updateDisplay();
  delay(500);

  if (cal_factor > INT32_MIN) {
    i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, cal_factor);
  } else {
    i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    cal_factor = 0;
  }

  if (!i2c_found) {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }

  //clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  //clockgen.pll_reset(SI5351_PLLA);
  //clockgen.pll_reset(SI5351_PLLB);
  clockgen.set_freq(400000000ULL, SI5351_CLK0);
  clockgen.set_freq(100000000ULL, SI5351_CLK1);
  clockgen.set_freq(307200000ULL, SI5351_CLK2);
  clockgen.output_enable(SI5351_CLK1, 1);
  clockgen.output_enable(SI5351_CLK2, 1);
  clockgen.output_enable(SI5351_CLK0, 1);

  // Frequency Counter
  delay(500);
  FreqCount.begin(1000);
  while (1) {
    if (old_cal != cal_factor) {
      display_Clear();
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F("     Adjusting"));
      display_Update();
      clockgen.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
      clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
      clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
      clockgen.pll_reset(SI5351_PLLA);
      clockgen.pll_reset(SI5351_PLLB);
      clockgen.set_freq(400000000ULL, SI5351_CLK0);
      clockgen.set_freq(100000000ULL, SI5351_CLK1);
      clockgen.set_freq(307200000ULL, SI5351_CLK2);
      old_cal = cal_factor;
      delay(500);
    } else {
      clockgen.update_status();
      while (clockgen.dev_status.SYS_INIT == 1) {
      }

      if (FreqCount.available()) {
        float count = FreqCount.read();
        display_Clear();
        println_Msg(F("Clock Calibration"));
        println_Msg(F(""));
        print_Msg(F("Freq:   "));
        print_Msg(count);
        println_Msg(F("Hz"));
        print_Msg(F("Correction:"));
        print_right(cal_factor);
        print_Msg(F("Adjustment:"));
        print_right(cal_offset);
#ifdef enable_Button2
        println_Msg(F("(Hold button to save)"));
        println_Msg(F(""));
        println_Msg(F("Decrease     Increase"));
#else
#ifdef enable_rotary
        println_Msg(F("Rotate to adjust"));
#else
        println_Msg(F("Click/dbl to adjust"));
#endif
#endif
        display_Update();
      }
#ifdef enable_Button2
      // get input button
      int a = checkButton1();
      int b = checkButton2();

      // if the cart readers input button is pressed shortly
      if (a == 1) {
        old_cal = cal_factor;
        cal_factor -= cal_offset;
      }
      if (b == 1) {
        old_cal = cal_factor;
        cal_factor += cal_offset;
      }

      // if the cart readers input buttons is double clicked
      if (a == 2) {
        cal_offset /= 10ULL;
        if (cal_offset < 1) {
          cal_offset = 100000000ULL;
        }
      }
      if (b == 2) {
        cal_offset *= 10ULL;
        if (cal_offset > 100000000ULL) {
          cal_offset = 1;
        }
      }

      // if the cart readers input button is pressed long
      if (a == 3) {
        savetofile();
      }
      if (b == 3) {
        savetofile();
      }
#else
      //Handle inputs for either rotary encoder or single button interface.
      int a = checkButton();

      if (a == 1) {  //clockwise rotation or single click
        old_cal = cal_factor;
        cal_factor += cal_offset;
      }

      if (a == 2) {  //counterclockwise rotation or double click
        old_cal = cal_factor;
        cal_factor -= cal_offset;
      }

      if (a == 3) {  //button short hold
        cal_offset *= 10ULL;
        if (cal_offset > 100000000ULL) {
          cal_offset = 1;
        }
      }

      if (a == 4) {  //button long hold
        savetofile();
      }
#endif
    }
  }
}

void print_right(int32_t number) {
  int32_t abs_number = number;
  if (abs_number < 0)
    abs_number *= -1;
  else
    print_Msg(F(" "));

  if (abs_number == 0)
    abs_number = 1;
  while (abs_number < 100000000ULL) {
    print_Msg(F(" "));
    abs_number *= 10ULL;
  }
  println_Msg(number);
}

void savetofile() {
  display_Clear();
  println_Msg(F("Saving..."));
  println_Msg(cal_factor);
  display_Update();
  delay(2000);

  if (!myFile.open("/snes_clk.txt", O_WRITE | O_CREAT | O_TRUNC)) {
    print_FatalError(sd_error_STR);
  }
  // Write calibration factor to file
  myFile.print(cal_factor);

  // Close the file:
  myFile.close();
  print_STR(done_STR, 1);
  display_Update();
  delay(1000);
  resetArduino();
}
#endif

#ifdef clockgen_calibration
int32_t atoi32_signed(const char* input_string) {
  if (input_string == NULL) {
    return 0;
  }

  int int_sign = 1;
  int i = 0;

  if (input_string[0] == '-') {
    int_sign = -1;
    i = 1;
  }

  int32_t return_val = 0;

  while (input_string[i] != '\0') {
    if (input_string[i] >= '0' && input_string[i] <= '9') {
      return_val = (return_val * 10) + (input_string[i] - '0');
    } else if (input_string[i] != '\0') {
      return 0;
    }

    i++;
  }

  return_val = return_val * int_sign;

  return return_val;
}

int32_t readClockOffset() {
  FsFile clock_file;
  char* clock_buf;
  int16_t i;
  int32_t clock_offset;

  if (!clock_file.open("/snes_clk.txt", O_READ)) {
    return INT32_MIN;
  }

  clock_buf = (char*)malloc(12 * sizeof(char));
  i = clock_file.read(clock_buf, 11);
  clock_file.close();
  if (i == -1) {
    free(clock_buf);
    return INT32_MIN;
  } else if ((i == 11) && (clock_buf[0] != '-')) {
    free(clock_buf);
    return INT32_MIN;
  } else {
    clock_buf[i] = 0;
  }

  for (i = 0; i < 12; i++) {
    if (clock_buf[i] != '-' && clock_buf[i] < '0' && clock_buf[i] > '9') {
      if (i == 0) {
        free(clock_buf);
        return INT32_MIN;
      } else if ((i == 1) && (clock_buf[0] == '-')) {
        free(clock_buf);
        return INT32_MIN;
      } else {
        clock_buf[i] = 0;
      }
    }
  }

  clock_offset = atoi32_signed(clock_buf);
  free(clock_buf);

  return clock_offset;
}
#endif

int32_t initializeClockOffset() {
#ifdef clockgen_calibration
  FsFile clock_file;
  const char zero_char_arr[] = { '0' };
  int32_t clock_offset = readClockOffset();
  if (clock_offset > INT32_MIN) {
    i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, clock_offset);
  } else {
    i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    if (clock_file.open("/snes_clk.txt", O_WRITE | O_CREAT | O_TRUNC)) {
      clock_file.write(zero_char_arr, 1);
      clock_file.close();
    }
  }
  return clock_offset;
#else
  // last number is the clock correction factor which is custom for each clock generator
  i2c_found = clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  return 0;
#endif
}

/******************************************
   Setup
 *****************************************/
void setup() {
  // Set Button Pin PG2 to Input
  DDRG &= ~(1 << 2);
#if defined(HW5) && !defined(enable_vselect)
  // HW5 has status LED connected to PD7
  // Set LED Pin PD7 to Output
  DDRD |= (1 << 7);
  PORTD |= (1 << 7);
#elif defined(enable_vselect)
  DDRD |= (1 << 7);
#else
  // HW1/2/3 have button connected to PD7
  // Set Button Pin PD7 to Input
  DDRD &= ~(1 << 7);
#endif
  // Activate Internal Pullup Resistors
  //PORTG |= (1 << 2);
  //PORTD |= (1 << 7);


  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);

#ifdef enable_LCD
  display.begin();
  display.setContrast(40);
  display.setFont(u8g2_font_haxrcorp4089_tr);
#endif

#ifdef enable_neopixel
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(background_color));
  pixels.setPixelColor(1, pixels.Color(0, 0, 100));
  pixels.setPixelColor(2, pixels.Color(0, 0, 100));
  pixels.show();

  // Set TX0 LED Pin(PE1) to Output for status indication during flashing for HW4
#if !(defined(enable_serial) || defined(HW5))
  DDRE |= (1 << 1);
#endif
#else
#ifndef enable_LCD
#ifdef CA_LED
  // Turn LED off
  digitalWrite(12, 1);
  digitalWrite(11, 1);
  digitalWrite(10, 1);
#endif
  // Configure 4 Pin RGB LED pins as output
  DDRB |= (1 << DDB6);  // Red LED (pin 12)
  DDRB |= (1 << DDB5);  // Green LED (pin 11)
  DDRB |= (1 << DDB4);  // Blue LED (pin 10)
#endif
#endif

#ifdef enable_OLED
  display.begin();
  //isplay.setContrast(40);
  display.setFont(u8g2_font_haxrcorp4089_tr);
#endif

#ifdef enable_serial
  // Serial Begin
  Serial.begin(9600);
  Serial.println("");
  Serial.println(F("Cartridge Reader"));
  Serial.println(F("2023 github.com/sanni"));
  // LED Error
  setColor_RGB(0, 0, 255);
#endif

  // Init SD card
  if (!sd.begin(SS)) {
    display_Clear();
    print_FatalError(sd_error_STR);
  }

#ifdef global_log
  if (!myLog.open("OSCR_LOG.txt", O_RDWR | O_CREAT | O_APPEND)) {
    print_FatalError(sd_error_STR);
  }
  println_Msg(F(""));
#if defined(HW1)
  print_Msg(F("OSCR HW1"));
#elif defined(HW2)
  print_Msg(F("OSCR HW2"));
#elif defined(HW3)
  print_Msg(F("OSCR HW3"));
#elif defined(HW4)
  print_Msg(F("OSCR HW4"));
#elif defined(HW5)
  print_Msg(F("OSCR HW5"));
#elif defined(SERIAL_MONITOR)
  print_Msg(F("OSCR Serial"));
#endif
  print_Msg(F(" V"));
  println_Msg(ver);
#endif

#ifdef RTC_installed
  // Start RTC
  RTCStart();

  // Set Date/Time Callback Funtion
  SdFile::dateTimeCallback(dateTime);
#endif

  // status LED ON
  statusLED(true);

  // Set power to low to protect carts
  vselect(true);

  // Start menu system
  startMenu();
}

/******************************************
   Common I/O Functions
 *****************************************/
// Switch data pins to write
void dataOut() {
  DDRC = 0xFF;
}

// Switch data pins to read
void dataIn() {
  // Set to Input and activate pull-up resistors
  DDRC = 0x00;
  // Pullups
  PORTC = 0xFF;
}

/******************************************
   Helper Functions
 *****************************************/
// Set RGB color
void setColor_RGB(byte r, byte g, byte b) {
#if defined(enable_neopixel)
  // Dim Neopixel LEDs
  if (r >= 100) r = 100;
  if (g >= 100) g = 100;
  if (b >= 100) b = 100;
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(background_color));
  pixels.setPixelColor(1, pixels.Color(g, r, b));
  pixels.setPixelColor(2, pixels.Color(g, r, b));
  pixels.show();
#elif defined(CA_LED)
  // Set color of analog 4 Pin common anode RGB LED
  analogWrite(12, 255 - r);
  analogWrite(11, 255 - g);
  analogWrite(10, 255 - b);
#else
  // Set color of analog 4 Pin common cathode RGB LED
  analogWrite(12, r);
  analogWrite(11, g);
  analogWrite(10, b);
#endif
}

// Extract ASCII printable characters from input, collapsing underscores and spaces.
// Use when extracting titles from cartridges, to build a rom title.
byte buildRomName(char* output, const byte* input, byte length) {
  byte input_char;
  byte output_len = 0;
  for (unsigned int i = 0; i < length; i++) {
    input_char = input[i];
    if (isprint(input_char) && input_char != '<' && input_char != '>' && input_char != ':' && input_char != '"' && input_char != '/' && input_char != '\\' && input_char != '|' && input_char != '?' && input_char != '*') {
      output[output_len++] = input_char;
    } else {
      if (output_len == 0 || output[output_len - 1] != '_') {
        output[output_len++] = '_';
      }
    }
  }
  while (
    output_len && (output[output_len - 1] == '_' || output[output_len - 1] == ' ')) {
    output_len--;
  }
  output[output_len] = 0;
  return output_len;
}

// Converts a progmem array into a ram array
void convertPgm(const char* const pgmOptions[], byte numArrays) {
  for (int i = 0; i < numArrays; i++) {
    strlcpy_P(menuOptions[i], (char*)pgm_read_word(&(pgmOptions[i])), 20);
  }
}

void _print_Error(void) {
  errorLvl = 1;
  setColor_RGB(255, 0, 0);
  display_Update();
}

void print_Error(const __FlashStringHelper* errorMessage) {
  println_Msg(errorMessage);
  _print_Error();
}

void print_Error(byte errorMessage) {
  print_STR(errorMessage, 1);
  _print_Error();
}

void _print_FatalError(void) {
  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
  resetArduino();
}

void print_FatalError(const __FlashStringHelper* errorMessage) {
  print_Error(errorMessage);
  _print_FatalError();
}

void print_FatalError(byte errorMessage) {
  print_Error(errorMessage);
  _print_FatalError();
}

void wait() {
  // Switch status LED off
  statusLED(false);
#if defined(enable_LCD)
  wait_btn();
#elif defined(enable_OLED)
  wait_btn();
#elif defined(enable_serial)
  wait_serial();
#endif
}

#ifdef global_log
// Copies the last part of the current log file to the dump folder
void save_log() {
  // Last found position
  uint64_t lastPosition = 0;

  // Go to first line of log
  myLog.rewind();

  // Find location of OSCR string to determine start of current log
  char tempStr[5];
  while (myLog.available()) {
    // Read first 4 chars of line
    tempStr[0] = myLog.read();

    // Check if it's an empty line
    if (tempStr[0] == '\r') {
      // skip \n
      myLog.read();
    } else {
      // Read more lines
      tempStr[1] = myLog.read();
      tempStr[2] = myLog.read();
      tempStr[3] = myLog.read();
      tempStr[4] = '\0';
      char str_buf;

      // Skip rest of line
      while (myLog.available()) {
        str_buf = myLog.read();

        //break out of loop if CRLF is found
        if (str_buf == '\r') {
          myLog.read();  //dispose \n because \r\n
          break;
        }
      }

      // If string is OSCR remember position in file and test if it's the lastest log entry
      if (strncmp(tempStr, "OSCR", 4) == 0) {
        // Check if current position is newer as old position
        if (myLog.position() > lastPosition) {
          lastPosition = myLog.position();
        }
      }
    }
  }
  // Go to position of last log entry
  myLog.seek(lastPosition - 16);

  // Copy log from there to dump dir
  sd.chdir(folder);
  strcpy(fileName, romName);
  strcat(fileName, ".txt");
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  while (myLog.available()) {
    if (myLog.available() >= 512) {
      for (word i = 0; i < 512; i++) {
        sdBuffer[i] = myLog.read();
      }
      myFile.write(sdBuffer, 512);
    } else {
      int i = 0;
      for (; i < myLog.available(); i++) {
        sdBuffer[i] = myLog.read();
      }
      myFile.write(sdBuffer, i);
    }
  }
  // Close the file:
  myFile.close();
}
#endif

#ifdef global_log
void println_Log(const __FlashStringHelper* string) {
  myLog.println(string);
}
#endif

void print_Msg(const __FlashStringHelper* string) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(string);
#endif
#ifdef enable_serial
  Serial.print(string);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(string);
#endif
}

void print_Msg(const char myString[]) {
#if (defined(enable_LCD) || defined(enable_OLED))
  // test for word wrap
  if ((display.tx + strlen(myString) * 6) > 128) {
    unsigned int strPos = 0;
    // Print until end of display
    while (display.tx < 122) {
      display.print(myString[strPos]);
      strPos++;
    }
    // Newline
    display.setCursor(0, display.ty + 8);
    // Print until end of display and ignore remaining characters
    while ((strPos < strlen(myString)) && (display.tx < 122)) {
      display.print(myString[strPos]);
      strPos++;
    }
  } else {
    display.print(myString);
  }
#endif
#ifdef enable_serial
  Serial.print(myString);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(myString);
#endif
}

void print_Msg(long unsigned int message) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message);
#endif
#ifdef enable_serial
  Serial.print(message);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(message);
#endif
}

void print_Msg(byte message, int outputFormat) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message, outputFormat);
#endif
#ifdef enable_serial
  Serial.print(message, outputFormat);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(message, outputFormat);
#endif
}

void print_Msg(word message, int outputFormat) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message, outputFormat);
#endif
#ifdef enable_serial
  Serial.print(message, outputFormat);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(message, outputFormat);
#endif
}

void print_Msg(int message, int outputFormat) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message, outputFormat);
#endif
#ifdef enable_serial
  Serial.print(message, outputFormat);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(message, outputFormat);
#endif
}

void print_Msg(long unsigned int message, int outputFormat) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message, outputFormat);
#endif
#ifdef enable_serial
  Serial.print(message, outputFormat);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(message, outputFormat);
#endif
}

void print_Msg(String string) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(string);
#endif
#ifdef enable_serial
  Serial.print(string);
#endif
#ifdef global_log
  if (!dont_log) myLog.print(string);
#endif
}

void print_Msg_PaddedHexByte(byte message) {
  if (message < 16) print_Msg(0, HEX);
  print_Msg(message, HEX);
}

void print_Msg_PaddedHex16(word message) {
  print_Msg_PaddedHexByte((message >> 8) & 0xFF);
  print_Msg_PaddedHexByte((message >> 0) & 0xFF);
}

void print_Msg_PaddedHex32(unsigned long message) {
  print_Msg_PaddedHexByte((message >> 24) & 0xFF);
  print_Msg_PaddedHexByte((message >> 16) & 0xFF);
  print_Msg_PaddedHexByte((message >> 8) & 0xFF);
  print_Msg_PaddedHexByte((message >> 0) & 0xFF);
}
void println_Msg(String string) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(string);
  display.setCursor(0, display.ty + 8);
#endif
#ifdef enable_serial
  Serial.println(string);
#endif
#ifdef global_log
  if (!dont_log) myLog.println(string);
#endif
}

void println_Msg(byte message, int outputFormat) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message, outputFormat);
  display.setCursor(0, display.ty + 8);
#endif
#ifdef enable_serial
  Serial.println(message, outputFormat);
#endif
#ifdef global_log
  if (!dont_log) myLog.println(message, outputFormat);
#endif
}

void println_Msg(const char myString[]) {
#if (defined(enable_LCD) || defined(enable_OLED))
  // test for word wrap
  if ((display.tx + strlen(myString) * 6) > 128) {
    unsigned int strPos = 0;
    // Print until end of display
    while ((display.tx < 122) && (myString[strPos] != '\0')) {
      display.print(myString[strPos]);
      strPos++;
    }
    // Newline
    display.setCursor(0, display.ty + 8);
    // Print until end of display and ignore remaining characters
    while ((strPos < strlen(myString)) && (display.tx < 122) && (myString[strPos] != '\0')) {
      display.print(myString[strPos]);
      strPos++;
    }
  } else {
    display.print(myString);
  }
  display.setCursor(0, display.ty + 8);
#endif
#ifdef enable_serial
  Serial.println(myString);
#endif
#ifdef global_log
  if (!dont_log) myLog.println(myString);
#endif
}

void println_Msg(const __FlashStringHelper* string) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(string);
  display.setCursor(0, display.ty + 8);
#endif
#ifdef enable_serial
  Serial.println(string);
#endif
#ifdef global_log
  char myBuffer[15];
  strlcpy_P(myBuffer, (char*)string, 15);
  if ((strncmp(myBuffer, "Press Button...", 14) != 0) && (strncmp(myBuffer, "Select file", 10) != 0)) {
    if (!dont_log) myLog.println(string);
  }
#endif
}

void println_Msg(long unsigned int message) {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.print(message);
  display.setCursor(0, display.ty + 8);
#endif
#ifdef enable_serial
  Serial.println(message);
#endif
#ifdef global_log
  if (!dont_log) myLog.println(message);
#endif
}

void display_Update() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.updateDisplay();
#endif
#ifdef enable_serial
  delay(100);
#endif
#ifdef global_log
  if (!dont_log) myLog.flush();
#endif
}

void display_Clear() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.clearDisplay();
  display.setCursor(0, 8);
#endif
#ifdef global_log
  if (!dont_log) myLog.println("");
#endif
}

void display_Clear_Slow() {
#if (defined(enable_LCD) || defined(enable_OLED))
  display.setDrawColor(0);
  for (byte y = 0; y < 64; y++) {
    display.drawLine(0, y, 128, y);
  }
  display.setDrawColor(1);
  display.setCursor(0, 8);
#endif
}

/******************************************
  RGB LED
*****************************************/
void rgbLed(byte Color) {
  switch (Color) {
    case blue_color:
      setColor_RGB(0, 0, 255);
      break;
    case red_color:
      setColor_RGB(255, 0, 0);
      break;
    case purple_color:
      setColor_RGB(255, 0, 255);
      break;
    case green_color:
      setColor_RGB(0, 255, 0);
      break;
    case turquoise_color:
      setColor_RGB(0, 255, 255);
      break;
    case yellow_color:
      setColor_RGB(255, 255, 0);
      break;
    case white_color:
      setColor_RGB(255, 255, 255);
      break;
  }
}

void blinkLED() {
#if defined(enable_vselect)
  // Nothing
#elif defined(HW5)
  PORTD ^= (1 << 7);
#elif defined(enable_OLED)
  PORTB ^= (1 << 4);
#elif defined(enable_LCD)
  PORTE ^= (1 << 1);
#elif defined(enable_serial)
  PORTB ^= (1 << 4);
  PORTB ^= (1 << 7);
#endif
}

#if defined(HW5) && !defined(enable_vselect)
void statusLED(boolean on) {
  if (!on)
    PORTD |= (1 << 7);
  else
    PORTD &= ~(1 << 7);
  /*
    #elif defined(enable_OLED)
    if (!on)
      PORTB |= (1 << 4);
    else
      PORTB &= ~(1 << 4);

    #elif defined(enable_LCD)
    if (!on)
      PORTE |= (1 << 1);
    else
      PORTE &= ~(1 << 1);

    #elif defined(enable_serial)
    if (!on) {
      PORTB |= (1 << 4);
      PORTB |= (1 << 7);
    }
    else {
      PORTB &= ~(1 << 4);
      PORTB &= ~(1 << 7);
    }
  */
}
void vselect(boolean vlow __attribute__((unused))) {
}
#elif defined(enable_vselect)
void statusLED(boolean on __attribute__((unused))) {
}
void vselect(boolean vlow) {
  if (vlow)
    PORTD |= (1 << 7);
  else
    PORTD &= ~(1 << 7);
}
#else
void statusLED(boolean on __attribute__((unused))) {
}
void vselect(boolean vlow __attribute__((unused))) {
}
#endif

/******************************************
  Menu system
*****************************************/
unsigned char question_box(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
#if (defined(enable_LCD) || defined(enable_OLED))
  return questionBox_Display(question, answers, num_answers, default_choice);
#endif
#ifdef enable_serial
  return questionBox_Serial(question, answers, num_answers, default_choice);
#endif
}

#if defined(enable_serial)
// Serial Monitor
byte questionBox_Serial(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
  // Print menu to serial monitor
  //Serial.println(question);
  Serial.println("");
  for (byte i = 0; i < num_answers; i++) {
    Serial.print(i);
    Serial.print(F(")"));
    Serial.println(answers[i]);
  }
  // Wait for user input
  Serial.println("");
  Serial.println(F("Please browse pages with 'u'(up) and 'd'(down)"));
  Serial.println(F("and enter a selection by typing a number(0-6): _ "));
  Serial.println("");
  while (Serial.available() == 0) {
  }

  // Read the incoming byte:
  incomingByte = Serial.read() - 48;

  /* Import file (i)
    if (incomingByte == 57) {
    if (filebrowse == 1) {
      // Make sure we have an import directory
      sd.mkdir("IMPORT", true);

      // Create and open file on sd card
      EEPROM_readAnything(0, foldern);
      sprintf(fileName, "IMPORT/%d.bin", foldern);
      if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
        print_FatalError(create_file_STR);
      }

      // Read file from serial
      fileSize = 0;
      while (Serial.available() > 0) {
        myFile.write(Serial.read());
        fileSize++;
        // Blink led
        blinkLED();
      }

      // Close the file:
      myFile.close();

      // Write new folder number back to eeprom
      foldern = foldern + 1;
      EEPROM_writeAnything(0, foldern);

      print_Msg(F("Imported "));
      print_Msg(fileSize);
      print_Msg(F(" bytes to file "));
      println_Msg(fileName);
      return 7;
    }
    }*/

  // Page up (u)
  if (incomingByte == 69) {
    if (filebrowse == 1) {
      if (currPage > 1) {
        lastPage = currPage;
        currPage--;
      } else {
        root = 1;
      }
    }
  }

  // Page down (d)
  else if (incomingByte == 52) {
    if ((numPages > currPage) && (filebrowse == 1)) {
      lastPage = currPage;
      currPage++;
    }
  }

  // Print the received byte for validation e.g. in case of a different keyboard mapping
  //Serial.println(incomingByte);
  //Serial.println("");
  return incomingByte;
}
#endif

// OLED & LCD
#if (defined(enable_LCD) || defined(enable_OLED))
// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_Display(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
  //clear the screen
  display.clearDisplay();
  display.updateDisplay();
  display.setCursor(0, 8);
  display.setDrawColor(1);

  // change the rgb led to the start menu color
  rgbLed(default_choice);

  // print menu
  display.println(question);
  display.setCursor(0, display.ty + 8);
  for (unsigned char i = 0; i < num_answers; i++) {
    // Add space for the selection dot
    display.print("   ");
    // Print menu item
    display.println(answers[i]);
    display.setCursor(0, display.ty + 8);
  }
  display.updateDisplay();

  // start with the default choice
  choice = default_choice;

  // draw selection box
  display.drawBox(1, 8 * choice + 11, 3, 3);
  display.updateDisplay();

  unsigned long idleTime = millis();
  byte currentColor = 0;

  // wait until user makes his choice
  while (1) {
    // Attract Mode
    if (millis() - idleTime > 300000) {
      if ((millis() - idleTime) % 4000 == 0) {
        if (currentColor < 7) {
          currentColor++;
          if (currentColor == 1) {
            currentColor = 2;  // skip red as that signifies an error to the user
          }
        } else {
          currentColor = 0;
        }
      }
      rgbLed(currentColor);
    }

    /* Check Button/rotary encoder
      1 click/clockwise rotation
      2 doubleClick/counter clockwise rotation
      3 hold/press
      4 longHold */
    int b = checkButton();

    // if button is pressed twice or rotary encoder turned left/counter clockwise
    if (b == 2) {
      idleTime = millis();

      // remove selection box
      display.setDrawColor(0);
      display.drawBox(1, 8 * choice + 11, 3, 3);
      display.setDrawColor(1);
      display.updateDisplay();

      // If cursor on top list entry
      if (choice == 0) {
        // On 2nd, 3rd, ... page go back one page
        if (currPage > 1) {
          lastPage = currPage;
          currPage--;
          break;
        }
        // In file browser go to root dir
        else if ((filebrowse == 1) && (root != 1)) {
          root = 1;
          break;
        }
        // Else go to bottom of list as a shortcut
        else {
          choice = num_answers - 1;
        }
      }
      // If not top entry go up/back one entry
      else {
        choice--;
      }

      // draw selection box
      display.drawBox(1, 8 * choice + 11, 3, 3);
      display.updateDisplay();

      // change RGB led to the color of the current menu option
      rgbLed(choice);
    }

    // go one down in the menu if the Cart Readers button is clicked shortly
    if (b == 1) {
      idleTime = millis();

      // remove selection box
      display.setDrawColor(0);
      display.drawBox(1, 8 * choice + 11, 3, 3);
      display.setDrawColor(1);
      display.updateDisplay();

      if ((choice == num_answers - 1) && (numPages > currPage)) {
        lastPage = currPage;
        currPage++;
        break;
      } else
        choice = (choice + 1) % num_answers;

      // draw selection box
      display.drawBox(1, 8 * choice + 11, 3, 3);
      display.updateDisplay();

      // change RGB led to the color of the current menu option
      rgbLed(choice);
    }

    // if the Cart Readers button is hold continiously leave the menu
    // so the currently highlighted action can be executed

    if (b == 3) {
      idleTime = millis();
      // All done
      numPages = 0;
      break;
    }
  }

  // pass on user choice
  setColor_RGB(0, 0, 0);

#ifdef global_log
  println_Msg("");
  print_Msg(F("[+] "));
  println_Msg(answers[choice]);
#endif

  return choice;
}
#endif

/******************************************
  User Control
*****************************************/
// Using Serial Monitor
#if defined(enable_serial)
int checkButton() {
  while (Serial.available() == 0) {
  }
  incomingByte = Serial.read() - 48;

  //Next
  if (incomingByte == 52) {
    return 1;
  }

  //Previous
  else if (incomingByte == 69) {
    return 2;
  }

  //Selection
  else if (incomingByte == 240) {
    return 3;
  }

  return 0;
}

void wait_serial() {
  if (errorLvl) {
    // Debug
#ifdef debug_mode
    ignoreError = 1;
#endif
    errorLvl = 0;
  }
  while (Serial.available() == 0) {
  }
  incomingByte = Serial.read() - 48;
  /* if ((incomingByte == 53) && (fileName[0] != '\0')) {
      // Open file on sd card
      sd.chdir(folder);
      if (myFile.open(fileName, O_READ)) {
        // Get rom size from file
        fileSize = myFile.fileSize();

        // Send filesize
        char tempStr[16];
        sprintf(tempStr, "%d", fileSize);
        Serial.write(tempStr);

        // Wait for ok
        while (Serial.available() == 0) {
        }

        // Send file
        for (unsigned long currByte = 0; currByte < fileSize; currByte++) {
          // Blink led
          if (currByte % 1024 == 0)
            blinkLED();
          Serial.write(myFile.read());
        }
        // Close the file:
        myFile.close();
      }
      else {
        print_FatalError(open_file_STR);
      }
    }*/
}
#endif

// Using one or two push buttons (HW1/HW2/HW3)
#if defined(enable_OLED)
// Read button state
int checkButton() {
#ifdef enable_Button2
  byte eventButton2 = checkButton2();
  if ((eventButton2 > 0) && (eventButton2 < 2))
    return 3;
  else if (eventButton2 > 2)
    return 4;
#endif
  return (checkButton1());
}

// Read button 1
int checkButton1() {
  int event = 0;

  // Read the state of the button (PD7)
  buttonVal1 = (PIND & (1 << 7));
  // Button pressed down
  if (buttonVal1 == LOW && buttonLast1 == HIGH && (millis() - upTime1) > debounce) {
    downTime1 = millis();
    ignoreUp1 = false;
    waitForUp1 = false;
    singleOK1 = true;
    holdEventPast1 = false;
    longholdEventPast1 = false;
    if ((millis() - upTime1) < DCgap && DConUp1 == false && DCwaiting1 == true) DConUp1 = true;
    else DConUp1 = false;
    DCwaiting1 = false;
  }
  // Button released
  else if (buttonVal1 == HIGH && buttonLast1 == LOW && (millis() - downTime1) > debounce) {
    if (!ignoreUp1) {
      upTime1 = millis();
      if (DConUp1 == false) DCwaiting1 = true;
      else {
        event = 2;
        DConUp1 = false;
        DCwaiting1 = false;
        singleOK1 = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if (buttonVal1 == HIGH && (millis() - upTime1) >= DCgap && DCwaiting1 == true && DConUp1 == false && singleOK1 == true) {
    event = 1;
    DCwaiting1 = false;
  }
  // Test for hold
  if (buttonVal1 == LOW && (millis() - downTime1) >= holdTime) {
    // Trigger "normal" hold
    if (!holdEventPast1) {
      event = 3;
      waitForUp1 = true;
      ignoreUp1 = true;
      DConUp1 = false;
      DCwaiting1 = false;
      //downTime1 = millis();
      holdEventPast1 = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime1) >= longHoldTime) {
      if (!longholdEventPast1) {
        event = 4;
        longholdEventPast1 = true;
      }
    }
  }
  buttonLast1 = buttonVal1;
  return event;
}

// Read button 2
int checkButton2() {
  int event = 0;

  // Read the state of the button (PG2)
  buttonVal2 = (PING & (1 << 2));
  // Button pressed down
  if (buttonVal2 == LOW && buttonLast2 == HIGH && (millis() - upTime2) > debounce) {
    downTime2 = millis();
    ignoreUp2 = false;
    waitForUp2 = false;
    singleOK2 = true;
    holdEventPast2 = false;
    longholdEventPast2 = false;
    if ((millis() - upTime2) < DCgap && DConUp2 == false && DCwaiting2 == true) DConUp2 = true;
    else DConUp2 = false;
    DCwaiting2 = false;
  }
  // Button released
  else if (buttonVal2 == HIGH && buttonLast2 == LOW && (millis() - downTime2) > debounce) {
    if (!ignoreUp2) {
      upTime2 = millis();
      if (DConUp2 == false) DCwaiting2 = true;
      else {
        event = 2;
        DConUp2 = false;
        DCwaiting2 = false;
        singleOK2 = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if (buttonVal2 == HIGH && (millis() - upTime2) >= DCgap && DCwaiting2 == true && DConUp2 == false && singleOK2 == true) {
    event = 1;
    DCwaiting2 = false;
  }
  // Test for hold
  if (buttonVal2 == LOW && (millis() - downTime2) >= holdTime) {
    // Trigger "normal" hold
    if (!holdEventPast2) {
      event = 3;
      waitForUp2 = true;
      ignoreUp2 = true;
      DConUp2 = false;
      DCwaiting2 = false;
      //downTime2 = millis();
      holdEventPast2 = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime2) >= longHoldTime) {
      if (!longholdEventPast2) {
        event = 4;
        longholdEventPast2 = true;
      }
    }
  }
  buttonLast2 = buttonVal2;
  return event;
}

// Wait for user to push button
void wait_btn() {
  // Change led to green
  if (errorLvl == 0)
    rgbLed(green_color);

  while (1) {
    // get input button
    int b = checkButton();

    // if the cart readers input button is pressed shortly
    if (b == 1) {
      errorLvl = 0;
      break;
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      if (errorLvl) {
        // Debug
#ifdef debug_mode
        ignoreError = 1;
#endif
        errorLvl = 0;
      }
      break;
    }
  }
}
#endif

// Using rotary encoder (HW4/HW5)
#if (defined(enable_LCD) && defined(enable_rotary))
// Read encoder state
int checkButton() {
  // Read rotary encoder
  encoder.tick();
  int newPos = encoder.getPosition();
  // Read button
  boolean reading = (PING & (1 << PING2)) >> PING2;

  // Check if rotary encoder has changed
  if (rotaryPos != newPos) {
    int rotaryDir = (int)encoder.getDirection();
    rotaryPos = newPos;
    if (rotaryDir == 1) {
      return 1;
    } else if (rotaryDir == -1) {
      return 2;
    }
  } else if (reading != buttonState) {
    if (reading != lastButtonState) {
      lastDebounceTime = millis();
      lastButtonState = reading;
    } else if ((millis() - lastDebounceTime) > debounceDelay) {
      buttonState = reading;
      // Button was pressed down
      if (buttonState == 0) {
        setColor_RGB(0, 0, 0);
        unsigned long pushTime = millis();
        // Wait until button was let go again
        while ((PING & (1 << PING2)) >> PING2 == 0) {
          // Signal long press delay reached
          if ((millis() - pushTime) > 2000) {
            rgbLed(green_color);
          }
        }

        // 2 second long press
        if ((millis() - pushTime) > 2000) {
          return 4;
        }
        // normal press
        else {
          return 3;
        }
      }
    }
  }
  return 0;
}

// Wait for user to push button
void wait_btn() {
  // Change led to green
  if (errorLvl == 0)
    rgbLed(green_color);

  while (1) {
    // get input button
    int b = checkButton();

    // if the cart readers input button is pressed shortly
    if (b == 1) {
      errorLvl = 0;
      break;
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      if (errorLvl) {
        // Debug
#ifdef debug_mode
        ignoreError = 1;
#endif
        errorLvl = 0;
      }
      break;
    }
  }
}

// Wait for user to rotate knob
void wait_encoder() {
  // Change led to green
  if (errorLvl == 0)
    rgbLed(green_color);

  while (1) {
    // Get rotary encoder
    encoder.tick();
    int newPos = encoder.getPosition();

    if (rotaryPos != newPos) {
      rotaryPos = newPos;
      errorLvl = 0;
      break;
    }
  }
}
#endif

/******************************************
  Filebrowser Module
*****************************************/
void fileBrowser(const __FlashStringHelper* browserTitle) {
  char fileNames[7][FILENAME_LENGTH];
  int currFile;
  FsFile myDir;
  div_t page_layout;

  filebrowse = 1;

  // Root
  filePath[0] = '/';
  filePath[1] = '\0';

  // Temporary char array for filename
  char nameStr[FILENAME_LENGTH];

browserstart:

  // Print title
  println_Msg(browserTitle);

  // Set currFile back to 0
  currFile = 0;
  currPage = 1;
  lastPage = 1;

  // Open filepath directory
  if (!myDir.open(filePath)) {
    display_Clear();
    print_FatalError(sd_error_STR);
  }

  // Count files in directory
  while (myFile.openNext(&myDir, O_READ)) {
    if (!myFile.isHidden() && (myFile.isDir() || myFile.isFile())) {
      currFile++;
    }
    myFile.close();
  }
  myDir.close();

  page_layout = div(currFile, 7);
  numPages = page_layout.quot + 1;

  // Fill the array "answers" with 7 options to choose from in the file browser
  char answers[7][20];

page:

  // If there are less than 7 entries, set count to that number so no empty options appear
  byte count = currPage == numPages ? page_layout.rem : 7;

  // Open filepath directory
  if (!myDir.open(filePath)) {
    display_Clear();
    print_FatalError(sd_error_STR);
  }

  int countFile = 0;
  byte i = 0;
  // Cycle through all files
  while ((myFile.openNext(&myDir, O_READ)) && (i < 8)) {
    // Get name of file
    myFile.getName(nameStr, FILENAME_LENGTH);

    // Ignore if hidden
    if (myFile.isHidden()) {
    }
    // Directory
    else if (myFile.isDir()) {
      if (countFile == ((currPage - 1) * 7 + i)) {
        snprintf(fileNames[i], FILENAME_LENGTH, "%s%s", "/", nameStr);
        i++;
      }
      countFile++;
    }
    // File
    else if (myFile.isFile()) {
      if (countFile == ((currPage - 1) * 7 + i)) {
        snprintf(fileNames[i], FILENAME_LENGTH, "%s", nameStr);
        i++;
      }
      countFile++;
    }
    myFile.close();
  }
  myDir.close();

  for (byte i = 0; i < 8; i++) {
    // Copy short string into fileOptions
    snprintf(answers[i], FILEOPTS_LENGTH, "%s", fileNames[i]);
  }

  // Create menu with title and 1-7 options to choose from
  unsigned char answer = question_box(browserTitle, answers, count, 0);

  // Check if the page has been switched
  if (currPage != lastPage) {
    lastPage = currPage;
    goto page;
  }

  // Check if we are supposed to go back to the root dir
  if (root) {
    // Change working dir to root
    filePath[0] = '/';
    filePath[1] = '\0';
    sd.chdir("/");
    // Start again
    root = 0;
    goto browserstart;
  }

  // wait for user choice to come back from the question box menu
  switch (answer) {
    case 0:
      strncpy(fileName, fileNames[0], FILENAME_LENGTH - 1);
      break;

    case 1:
      strncpy(fileName, fileNames[1], FILENAME_LENGTH - 1);
      break;

    case 2:
      strncpy(fileName, fileNames[2], FILENAME_LENGTH - 1);
      break;

    case 3:
      strncpy(fileName, fileNames[3], FILENAME_LENGTH - 1);
      break;

    case 4:
      strncpy(fileName, fileNames[4], FILENAME_LENGTH - 1);
      break;

    case 5:
      strncpy(fileName, fileNames[5], FILENAME_LENGTH - 1);
      break;

    case 6:
      strncpy(fileName, fileNames[6], FILENAME_LENGTH - 1);
      break;

      //case 7:
      // File import
      //break;
  }

  // Add directory to our filepath if we just entered a new directory
  if (fileName[0] == '/') {
    // add dirname to path
    strcat(filePath, fileName);
    // Remove / from dir name
    char* dirName = fileName + 1;
    // Change working dir
    sd.chdir(dirName);
    // Start browser in new directory again
    goto browserstart;
  } else {
    // Afer everything is done change SD working directory back to root
    sd.chdir("/");
  }
  filebrowse = 0;
}

/******************************************
  Main loop
*****************************************/
void loop() {
#ifdef enable_N64
  if (mode == mode_N64_Controller) {
    n64ControllerMenu();
  } else if (mode == mode_N64_Cart) {
    n64CartMenu();
  }
#else
  if (1 == 0) {
  }
#endif
#ifdef enable_SNES
  else if (mode == mode_SNES) {
    snesMenu();
  }
#endif
#ifdef enable_FLASH
  else if (mode == mode_FLASH8) {
    flashromMenu8();
  }
#ifdef enable_FLASH16
  else if (mode == mode_FLASH16) {
    flashromMenu16();
  } else if (mode == mode_EPROM) {
    epromMenu();
  }
#endif
#endif
#ifdef enable_SFM
  else if (mode == mode_SFM) {
    sfmMenu();
  }
#endif
#ifdef enable_GBX
  else if (mode == mode_GB) {
    gbMenu();
  } else if (mode == mode_GBA) {
    gbaMenu();
  }
#endif
#ifdef enable_SFM
#ifdef enable_FLASH
  else if (mode == mode_SFM_Flash) {
    sfmFlashMenu();
  }
#endif
  else if (mode == mode_SFM_Game) {
    sfmGameOptions();
  }
#endif
#ifdef enable_GBX
  else if (mode == mode_GBM) {
    gbmMenu();
  }
#endif
#ifdef enable_MD
  else if (mode == mode_MD_Cart) {
    mdCartMenu();
  }
#endif
#ifdef enable_PCE
  else if (mode == mode_PCE) {
    pceMenu();
  }
#endif
#ifdef enable_SV
  else if (mode == mode_SV) {
    svMenu();
  }
#endif
#ifdef enable_NES
  else if (mode == mode_NES) {
    nesMenu();
  }
#endif
#ifdef enable_SMS
  else if (mode == mode_SMS) {
    smsMenu();
  }
#endif
#ifdef enable_MD
  else if (mode == mode_SEGA_CD) {
    segaCDMenu();
  }
#endif
#ifdef enable_GBX
  else if (mode == mode_GB_GBSmart) {
    gbSmartMenu();
  } else if (mode == mode_GB_GBSmart_Flash) {
    gbSmartFlashMenu();
  } else if (mode == mode_GB_GBSmart_Game) {
    gbSmartGameOptions();
  }
#endif
#ifdef enable_WS
  else if (mode == mode_WS) {
    wsMenu();
  }
#endif
#ifdef enable_NGP
  else if (mode == mode_NGP) {
    ngpMenu();
  }
#endif
#ifdef enable_INTV
  else if (mode == mode_INTV) {
    intvMenu();
  }
#endif
#ifdef enable_COLV
  else if (mode == mode_COL) {
    colMenu();
  }
#endif
#ifdef enable_VBOY
  else if (mode == mode_VBOY) {
    vboyMenu();
  }
#endif
#ifdef enable_WSV
  else if (mode == mode_WSV) {
    wsvMenu();
  }
#endif
#ifdef enable_PCW
  else if (mode == mode_PCW) {
    pcwMenu();
  }
#endif
#ifdef enable_ATARI
  else if (mode == mode_ATARI) {
    atariMenu();
  }
#endif
#ifdef enable_ODY2
  else if (mode == mode_ODY2) {
    ody2Menu();
  }
#endif
#ifdef enable_ARC
  else if (mode == mode_ARC) {
    arcMenu();
  }
#endif
#ifdef enable_FAIRCHILD
  else if (mode == mode_FAIRCHILD) {
    fairchildMenu();
  }
#endif
  else {
    display_Clear();
    println_Msg(F("Menu Error"));
    println_Msg("");
    println_Msg("");
    print_Msg(F("Mode = "));
    print_Msg(mode);
    println_Msg(F(""));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
    resetArduino();
  }
}

//******************************************
// End of File
//******************************************