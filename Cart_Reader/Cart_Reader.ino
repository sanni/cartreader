/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   This project represents a community-driven effort to provide
   an easy to build and easy to modify cartridge dumper.

   Date:             02.08.2021
   Version:          6.6

   SD lib: https://github.com/greiman/SdFat
   LCD lib: https://github.com/adafruit/Adafruit_SSD1306
   GFX Lib: https://github.com/adafruit/Adafruit-GFX-Library
   BusIO: https://github.com/adafruit/Adafruit_BusIO
   RGB Tools lib: https://github.com/joushx/Arduino-RGB-Tools
   SI5351 lib: https://github.com/etherkit/Si5351Arduino

   Compiled with Arduino 1.8.13

   Code included from other projects:
   MichlK - ROM Reader for Super Nintendo
   Jeff Saltzman - 4-Way Button
   Wayne and Layne - Video Game Shield menu
   hkz & themanbehindthecurtain - N64 flashram commands
   Andrew Brown & Peter Den Hartog - N64 controller protocol
   Shaun Taylor - N64 controller CRC functions
   Angus Gratton - CRC32
   Snes9x - SuperFX sram fix
   insidegadgets - GBCartRead
   RobinTheHood - GameboyAdvanceRomDumper
   Gens-gs - Megadrive checksum

   And a special Thank You to all contributors on Github and the Arduino forum.

**********************************************************************************/

char ver[5] = "6.6";

/******************************************
   Libraries
 *****************************************/
// Options
#include "options.h"

// SD Card
#include "SdFat.h"
SdFs sd;
FsFile myDir;
FsFile myFile;

// Basic Libs
#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

// AVR Eeprom
#include <EEPROM.h>
// forward declarations for "T" (for non Arduino IDE)
template <class T> int EEPROM_writeAnything(int ee, const T& value);
template <class T> int EEPROM_readAnything(int ee, T& value);

template <class T> int EEPROM_writeAnything(int ee, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value) {
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

// Graphic I2C LCD
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Adafruit Clock Generator
#include <si5351.h>
Si5351 clockgen;

// RGB LED
#include <RGBTools.h>

// Set pins of red, green and blue
RGBTools rgb(12, 11, 10);

typedef enum COLOR_T {
  blue_color,
  red_color,
  purple_color,
  green_color,
  turquoise_color,
  yellow_color,
  white_color,
} color_t;

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

// optimization-safe nop delay
#define NOP __asm__ __volatile__ ("nop\n\t")

// Button timing
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define DCgap 250 // max ms between clicks for a double click event
#define holdTime 2000 // ms hold period: how long to wait for press+hold event
#define longHoldTime 5000 // ms long hold period: how long to wait for press+hold event

/******************************************
   Variables
 *****************************************/
#ifdef enable_OLED
// Button 1
boolean buttonVal1 = HIGH; // value read from button
boolean buttonLast1 = HIGH; // buffered value of the button's previous state
boolean DCwaiting1 = false; // whether we're waiting for a double click (down)
boolean DConUp1 = false; // whether to register a double click on next release, or whether to wait and click
boolean singleOK1 = true; // whether it's OK to do a single click
long downTime1 = -1; // time the button was pressed down
long upTime1 = -1; // time the button was released
boolean ignoreUp1 = false; // whether to ignore the button release because the click+hold was triggered
boolean waitForUp1 = false; // when held, whether to wait for the up event
boolean holdEventPast1 = false; // whether or not the hold event happened already
boolean longholdEventPast1 = false;// whether or not the long hold event happened already
// Button 2
boolean buttonVal2 = HIGH; // value read from button
boolean buttonLast2 = HIGH; // buffered value of the button's previous state
boolean DCwaiting2 = false; // whether we're waiting for a double click (down)
boolean DConUp2 = false; // whether to register a double click on next release, or whether to wait and click
boolean singleOK2 = true; // whether it's OK to do a single click
long downTime2 = -1; // time the button was pressed down
long upTime2 = -1; // time the button was released
boolean ignoreUp2 = false; // whether to ignore the button release because the click+hold was triggered
boolean waitForUp2 = false; // when held, whether to wait for the up event
boolean holdEventPast2 = false; // whether or not the hold event happened already
boolean longholdEventPast2 = false;// whether or not the long hold event happened already
#else
// For incoming serial data
int incomingByte;
#endif

// Variables for the menu
int choice = 0;
// Temporary array that holds the menu option read out of progmem
char menuOptions[7][20];
boolean ignoreError = 0;

// File browser
#define FILENAME_LENGTH 32
#define FILEPATH_LENGTH 64
#define FILEOPTS_LENGTH 20

char fileName[FILENAME_LENGTH];
char filePath[FILEPATH_LENGTH];
byte currPage;
byte lastPage;
byte numPages;
boolean root = 0;
boolean filebrowse = 0;

// Common
char romName[17];
unsigned long sramSize = 0;
int romType = 0;
byte saveType;
word romSize = 0;
word numBanks = 128;
char checksumStr[5];
bool errorLvl = 0;
byte romVersion = 0;
char cartID[5];
unsigned long cartSize;
char flashid[5];
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

//remember folder number to create a new folder for every save
int foldern;
char folder[36];

// Array that holds the data
byte sdBuffer[512];

// soft reset Arduino: jumps to 0
// using the watchdog timer would be more elegant but some Mega2560 bootloaders are buggy with it
void(*resetArduino) (void) = 0;

// Progressbar
void draw_progressbar(uint32_t processedsize, uint32_t totalsize);

//******************************************
// Bitmaps
//******************************************
static const uint8_t PROGMEM icon [] = {
  0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF,
  0xFF, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00, 0x0F, 0xFF,
  0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x00,
  0x0F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0xF8, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xF8,
  0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xF8, 0x00, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00,
  0x00, 0xFF, 0x80, 0x0F, 0xFF, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0xFF, 0x80, 0x0F, 0x00, 0x0F, 0xFF,
  0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00,
  0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80, 0x0F, 0x00, 0x0F, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0x80,
  0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x0F, 0xF0, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xF0, 0xFF, 0x0F, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0x0F, 0xF0, 0xF0, 0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x0F, 0xFF,
  0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0,
  0x0F, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0x00, 0xF0, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00,
  0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF,
  0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0,
  0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00,
  0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F,
  0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0,
  0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x00, 0x0F, 0xF0, 0x00,
  0xFF, 0xF0, 0x00, 0x0F, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0, 0x0F, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xF0,
  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x1F, 0xF0, 0xFF, 0xF0,
  0xFF, 0x0F, 0xFF, 0x0F, 0xFF, 0x1F, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0, 0x1F, 0xF0,
  0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0, 0x1F, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0xF0,
  0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF,
  0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF, 0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xF0, 0xFF,
  0x0F, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01,
  0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
  0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0F, 0x80, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x01, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80
};

static const uint8_t PROGMEM sig [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0xF0, 0x80, 0x40, 0x30, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x90, 0xCC, 0x4E, 0x10, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x90, 0x5C, 0x7B, 0x19, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xB8, 0x56, 0x31, 0x09, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xE0, 0xA8, 0x72, 0x31, 0x0F, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xAC, 0x23, 0x21, 0x86, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE7, 0xA1, 0x00, 0x80, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//******************************************
// Data used by multiple modules
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

// used by MD and NES modules
byte eepbit[8];
byte eeptemp;

/******************************************
  Menu
*****************************************/
// Main menu
static const char modeItem1[] PROGMEM = "Add-ons";
static const char modeItem2[] PROGMEM = "Super Nintendo";
static const char modeItem3[] PROGMEM = "Mega Drive";
static const char modeItem4[] PROGMEM = "Nintendo 64";
static const char modeItem5[] PROGMEM = "Game Boy";
static const char modeItem6[] PROGMEM = "About";
static const char modeItem7[] PROGMEM = "Reset";
static const char* const modeOptions[] PROGMEM = {modeItem1, modeItem2, modeItem3, modeItem4, modeItem5, modeItem6, modeItem7};

// Add-ons submenu
static const char addonsItem1[] PROGMEM = "NES/Famicom";
static const char addonsItem2[] PROGMEM = "Flashrom Programmer";
static const char addonsItem3[] PROGMEM = "PC Engine/TG16";
static const char addonsItem4[] PROGMEM = "Sega Master System";
static const char addonsItem5[] PROGMEM = "WonderSwan";
static const char addonsItem6[] PROGMEM = "NeoGeo Pocket";
static const char addonsItem7[] PROGMEM = "Reset";
static const char* const addonsOptions[] PROGMEM = {addonsItem1, addonsItem2, addonsItem3, addonsItem4, addonsItem5, addonsItem6, addonsItem7};

// Info Screen
void aboutScreen() {
  display_Clear();
  // Draw the Logo
  display.drawBitmap(0, 0, sig, 128, 64, 1);
  println_Msg(F("Cartridge Reader"));
  println_Msg(F("github.com/sanni"));
  print_Msg(F("2021 Version "));
  println_Msg(ver);
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F(""));
  println_Msg(F("Press Button"));
  display_Update();

  while (1) {
#ifdef enable_OLED
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
#else
    wait_serial();
    resetArduino();
#endif
    rgb.setColor(random(0, 255), random(0, 255), random(0, 255));
    delay(random(50, 100));
  }
}

// All included slots
void mainMenu() {
  // create menu with title and 6 options to choose from
  unsigned char modeMenu;
  // Copy menuOptions out of progmem
  convertPgm(modeOptions, 7);
  modeMenu = question_box(F("Cartridge Reader"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (modeMenu)
  {
    case 0:
      addonsMenu();
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
  }
}

// Everything that needs an adapter
void addonsMenu() {
  // create menu with title and 7 options to choose from
  unsigned char addonsMenu;
  // Copy menuOptions out of progmem
  convertPgm(addonsOptions, 7);
  addonsMenu = question_box(F("Choose Adapter"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (addonsMenu)
  {
#ifdef enable_NES
    case 0:
      nesMenu();
      break;
#endif

#ifdef enable_FLASH
    case 1:
      flashMenu();
      break;
#endif

#ifdef enable_PCE
    case 2:
      pcsMenu();
      break;
#endif

#ifdef enable_SMS
    case 3:
      smsMenu();
      break;
#endif

#ifdef enable_WS
    case 4:
      display_Clear();
      display_Update();
      setup_WS();
      mode = mode_WS;
      break;
#endif

#ifdef enable_NGP
    case 5:
      display_Clear();
      display_Update();
      setup_NGP();
      mode = mode_NGP;
      break;
#endif

    case 6:
      resetArduino();
      break;
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
        print_Msg(F("]"));
      }
      else {
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
   Setup
 *****************************************/
void setup() {
  // Set Button Pins(PD7, PG2) to Input
  DDRD &= ~(1 << 7);
  DDRG &= ~(1 << 2);
  // Activate Internal Pullup Resistors
  //PORTD |= (1 << 7);
  //PORTG |= (1 << 2);

  // Read current folder number out of eeprom
  EEPROM_readAnything(0, foldern);

#ifdef enable_OLED
  // GLCD
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Clear the screen buffer.
  display_Clear();

#ifndef fast_start
  delay(100);

  // Draw line
  display.drawLine(0, 32, 127, 32, WHITE);
  display_Update();
  delay(100);

  // Initialize LED
  rgb.setColor(0, 0, 0);

  // Clear the screen.
  display_Clear();
  display_Update();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  display_Update();
  delay(50);

  // Clear the screen.
  display_Clear();
  display_Update();
  delay(25);

  // Draw the Logo
  display.drawBitmap(28, 0, icon, 72, 64, 1);
  for (int s = 1; s < 64; s += 2) {
    // Draw Scanlines
    display.drawLine(0, s, 127, s, BLACK);
  }
  display.setCursor(100, 55);
  display.println(ver);
  display_Update();
  delay(200);
#endif

#else
  // Serial Begin
  Serial.begin(9600);
  Serial.println(F("Cartridge Reader"));
  Serial.println(F("2021 sanni"));
  Serial.println("");
  // LED Error
  rgb.setColor(0, 0, 255);
#endif

  // Init SD card
  if (!sd.begin(SdSpiConfig(SS, DEDICATED_SPI))) {
    display_Clear();
    print_Error(F("SD Error"), true);
  }

#ifndef enable_OLED
  // Print SD Info
  Serial.print(F("SD Card: "));
  Serial.print(sd.card()->cardSize() * 512E-9);
  Serial.print(F("GB FAT"));
  Serial.println(int(sd.vol()->fatType()));
#endif

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
// Converts a progmem array into a ram array
void convertPgm(const char* const pgmOptions[], byte numArrays) {
  for (int i = 0; i < numArrays; i++) {
    strlcpy_P(menuOptions[i], (char*)pgm_read_word(&(pgmOptions[i])), 20);
  }
}

void print_Error(const __FlashStringHelper *errorMessage, boolean forceReset) {
  errorLvl = 1;
  rgb.setColor(255, 0, 0);
  println_Msg(errorMessage);
  display_Update();

  if (forceReset) {
#ifdef enable_OLED
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
    if (ignoreError == 0) {
      resetArduino();
    }
    else {
      ignoreError = 0;
      display_Clear();
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F("  Error Overwrite"));
      display_Update();
      delay(2000);
    }
#else
    println_Msg(F("Fatal Error, please reset"));
    while (1);
#endif
  }
}

void wait() {
#ifdef enable_OLED
  wait_btn();
#else
  wait_serial();
#endif
}

void print_Msg(const __FlashStringHelper *string) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void print_Msg(const char string[]) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void print_Msg(long unsigned int message) {
#ifdef enable_OLED
  display.print(message);
#else
  Serial.print(message);
#endif
}

void print_Msg(byte message, int outputFormat) {
#ifdef enable_OLED
  display.print(message, outputFormat);
#else
  Serial.print(message, outputFormat);
#endif
}

void print_Msg(String string) {
#ifdef enable_OLED
  display.print(string);
#else
  Serial.print(string);
#endif
}

void print_Msg_PaddedHexByte(byte message) {
  if (message < 16) print_Msg(0, HEX);
  print_Msg(message, HEX);
}

void print_Msg_PaddedHex16(word message) {
  print_Msg_PaddedHexByte((message >>  8) & 0xFF);
  print_Msg_PaddedHexByte((message >>  0) & 0xFF);
}

void print_Msg_PaddedHex32(unsigned long message) {
  print_Msg_PaddedHexByte((message >> 24) & 0xFF);
  print_Msg_PaddedHexByte((message >> 16) & 0xFF);
  print_Msg_PaddedHexByte((message >>  8) & 0xFF);
  print_Msg_PaddedHexByte((message >>  0) & 0xFF);
}


void println_Msg(String string) {
#ifdef enable_OLED
  display.println(string);
#else
  Serial.println(string);
#endif
}

void println_Msg(byte message, int outputFormat) {
#ifdef enable_OLED
  display.println(message, outputFormat);
#else
  Serial.println(message, outputFormat);
#endif
}

void println_Msg(const char message[]) {
#ifdef enable_OLED
  display.println(message);
#else
  Serial.println(message);
#endif
}

void println_Msg(const __FlashStringHelper *string) {
#ifdef enable_OLED
  display.println(string);
#else
  Serial.println(string);
#endif
}

void println_Msg(long unsigned int message) {
#ifdef enable_OLED
  display.println(message);
#else
  Serial.println(message);
#endif
}

void display_Update() {
#ifdef enable_OLED
  display.display();
#else
  delay(100);
#endif
}

void display_Clear() {
#ifdef enable_OLED
  display.clearDisplay();
  display.setCursor(0, 0);
#endif
}

unsigned char question_box(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
#ifdef enable_OLED
  return questionBox_OLED(question, answers, num_answers, default_choice);
#else
  return questionBox_Serial(question, answers, num_answers, default_choice);
#endif
}

/******************************************
  Serial Out
*****************************************/
#ifndef enable_OLED
void wait_serial() {
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
            PORTB ^= (1 << 4);
          Serial.write(myFile.read());
        }
        // Close the file:
        myFile.close();
      }
      else {
        print_Error(F("Can't open file"), true);
      }
    }*/
}

byte questionBox_Serial(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
  // Print menu to serial monitor
  //Serial.println(question);
  for (byte i = 0; i < num_answers; i++) {
    Serial.print(i);
    Serial.print(F(")"));
    Serial.println(answers[i]);
  }
  // Wait for user input
  Serial.println("");
  Serial.println(F("Please browse pages with 'u'(up) and 'd'(down)"));
  Serial.println(F("and enter a selection by typing a number(0-6): _ "));
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
        print_Error(F("Can't create file on SD"), true);
      }

      // Read file from serial
      fileSize = 0;
      while (Serial.available() > 0) {
        myFile.write(Serial.read());
        fileSize++;
        // Blink led
        PORTB ^= (1 << 4);
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
      }
      else {
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

/******************************************
  RGB LED
*****************************************/
void rgbLed(byte Color) {
  switch (Color) {
    case blue_color:
      rgb.setColor(0, 0, 255);
      break;
    case red_color:
      rgb.setColor(255, 0, 0);
      break;
    case purple_color:
      rgb.setColor(255, 0, 255);
      break;
    case green_color:
      rgb.setColor(0, 255, 0);
      break;
    case turquoise_color:
      rgb.setColor(0, 255, 255);
      break;
    case yellow_color:
      rgb.setColor(255, 255, 0);
      break;
    case white_color:
      rgb.setColor(255, 255, 255);
      break;
  }
}

/******************************************
  OLED Menu Module
*****************************************/
#ifdef enable_OLED
// Read button state
int checkButton() {
#ifdef enable_Button2
  if (checkButton2() != 0)
    return 3;
  else
    return (checkButton1());
#else
  return (checkButton1());
#endif
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
  if ( buttonVal1 == HIGH && (millis() - upTime1) >= DCgap && DCwaiting1 == true && DConUp1 == false && singleOK1 == true) {
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

  // Read the state of the button (PD7)
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
  if ( buttonVal2 == HIGH && (millis() - upTime2) >= DCgap && DCwaiting2 == true && DConUp2 == false && singleOK2 == true) {
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

  while (1)
  {
    // get input button
    int b = checkButton();

#ifdef enable_N64
#ifndef clockgen_installed
    // Send some clock pulses to the Eeprom in case it locked up
    if ((mode == mode_N64_Cart) && ((saveType == 5) || (saveType == 6))) {
      pulseClock_N64(1);
    }
#endif
#endif

    // if the cart readers input button is pressed shortly
    if (b == 1) {
      errorLvl = 0;
      break;
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      if (errorLvl) {
        // Debug
        //ignoreError = 1;
        errorLvl = 0;
      }
      break;
    }
  }
}

// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(const __FlashStringHelper* question, char answers[7][20], int num_answers, int default_choice) {
  //clear the screen
  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);

  // change the rgb led to the start menu color
  rgbLed(default_choice);

  // print menu
  display.println(question);
  for (unsigned char i = 0; i < num_answers; i++) {
    // Add space for the selection dot
    display.print(" ");
    // Print menu item
    display.println(answers[i]);
  }
  display.display();

  // start with the default choice
  choice = default_choice;

  // draw selection box
  display.drawPixel(0, 8 * choice + 12, WHITE);
  display.display();

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
            currentColor = 2; // skip red as that signifies an error to the user
          }
        }
        else {
          currentColor = 0;
        }
      }
      rgbLed(currentColor);
    }

    /* Check Button
      1 click
      2 doubleClick
      3 hold
      4 longHold */
    int b = checkButton();

    if (b == 2) {
      idleTime = millis();

      // remove selection box
      display.drawPixel(0, 8 * choice + 12, BLACK);
      display.display();

      if ((choice == 0) && (filebrowse == 1)) {
        if (currPage > 1) {
          lastPage = currPage;
          currPage--;
          break;
        }
        else {
          root = 1;
          break;
        }
      }
      else if (choice > 0) {
        choice--;
      }
      else {
        choice = num_answers - 1;
      }

      // draw selection box
      display.drawPixel(0, 8 * choice + 12, WHITE);
      display.display();

      // change RGB led to the color of the current menu option
      rgbLed(choice);
    }

    // go one down in the menu if the Cart Dumpers button is clicked shortly

    if (b == 1) {
      idleTime = millis();

      // remove selection box
      display.drawPixel(0, 8 * choice + 12, BLACK);
      display.display();

      if ((choice == num_answers - 1 ) && (numPages > currPage) && (filebrowse == 1)) {
        lastPage = currPage;
        currPage++;
        break;
      }
      else
        choice = (choice + 1) % num_answers;

      // draw selection box
      display.drawPixel(0, 8 * choice + 12, WHITE);
      display.display();

      // change RGB led to the color of the current menu option
      rgbLed(choice);
    }

    // if the Cart Dumpers button is hold continiously leave the menu
    // so the currently highlighted action can be executed

    if (b == 3) {
      idleTime = millis();
      break;
    }
  }

  // pass on user choice
  rgb.setColor(0, 0, 0);
  return choice;
}
#endif

/******************************************
  Filebrowser Module
*****************************************/
void fileBrowser(const __FlashStringHelper* browserTitle) {
  char fileNames[30][FILENAME_LENGTH];
  int currFile;
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
    print_Error(F("SD Error"), true);
  }

  // Read in File as long as there are files
  while (myFile.openNext(&myDir, O_READ)) {

    // Get name of file
    myFile.getName(nameStr, FILENAME_LENGTH);

    // Ignore if hidden
    if (myFile.isHidden()) {
    }
    // Indicate a directory.
    else if (myFile.isDir()) {
      // Copy full dirname into fileNames
      snprintf(fileNames[currFile], FILENAME_LENGTH, "%s%s", "/", nameStr);
      currFile++;
    }
    // It's just a file
    else if (myFile.isFile()) {
      // Copy full filename into fileNames
      snprintf(fileNames[currFile], FILENAME_LENGTH, "%s", nameStr);
      currFile++;
    }
    myFile.close();
  }
  myDir.close();

  // "Calculate number of needed pages"
  if (currFile < 8)
    numPages = 1;
  else if (currFile < 15)
    numPages = 2;
  else if (currFile < 22)
    numPages = 3;
  else if (currFile < 29)
    numPages = 4;
  else if (currFile < 36)
    numPages = 5;

  // Fill the array "answers" with 7 options to choose from in the file browser
  char answers[7][20];

page:

  // If there are less than 7 entries, set count to that number so no empty options appear
  byte count;
  if (currFile < 8)
    count = currFile;
  else if (currPage == 1)
    count = 7;
  else if (currFile < 15)
    count = currFile - 7;
  else if (currPage == 2)
    count = 7;
  else if (currFile < 22)
    count = currFile - 14;
  else if (currPage == 3)
    count = 7;
  else if (currFile < 29)
    count = currFile - 21;
  else {
    display_Clear();

    println_Msg(F("Too many files"));
    display_Update();
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }

  for (byte i = 0; i < 8; i++ ) {
    // Copy short string into fileOptions
    snprintf( answers[i], FILEOPTS_LENGTH, "%s", fileNames[ ((currPage - 1) * 7 + i)] );
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
  switch (answer)
  {
    case 0:
      strncpy(fileName, fileNames[0 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 1:
      strncpy(fileName, fileNames[1 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 2:
      strncpy(fileName, fileNames[2 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 3:
      strncpy(fileName, fileNames[3 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 4:
      strncpy(fileName, fileNames[4 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 5:
      strncpy(fileName, fileNames[5 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 6:
      strncpy(fileName, fileNames[6 + ((currPage - 1) * 7)], FILENAME_LENGTH - 1);
      break;

    case 7:
      // File import
      break;
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
  }
  else {
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
  }
  else if (mode == mode_N64_Cart) {
    n64CartMenu();
  }
#else
  if (1 == 0) { }
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
  else if (mode == mode_FLASH16) {
    flashromMenu16();
  }
  else if (mode == mode_EPROM) {
    epromMenu();
  }
#endif
#ifdef enable_NP
  else if (mode == mode_SFM) {
    sfmMenu();
  }
#endif
#ifdef enable_GBX
  else if (mode == mode_GB) {
    gbMenu();
  }
  else if (mode == mode_GBA) {
    gbaMenu();
  }
#endif
#ifdef enable_NP
  else if (mode == mode_SFM_Flash) {
    sfmFlashMenu();
  }
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
  }
  else if (mode == mode_GB_GBSmart_Flash) {
    gbSmartFlashMenu();
  }
  else if (mode == mode_GB_GBSmart_Game) {
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
  else {
    display_Clear();
    println_Msg(F("Menu Error"));
    println_Msg("");
    println_Msg("");
    print_Msg(F("Mode = "));
    print_Msg(mode);
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
    resetArduino();
  }
}

//******************************************
// End of File
//******************************************
