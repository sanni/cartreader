/**********************************************************************************
                    Cartridge Reader for Arduino Mega2560

   Author:           sanni
   Date:             2016-10-05
   Version:          V19E

   SD  lib:         https://github.com/greiman/SdFat
   LCD lib:         https://github.com/adafruit/Adafruit_SSD1306
   Clockgen:        https://github.com/etherkit/Si5351Arduino
   RGB Tools lib:   https://github.com/joushx/Arduino-RGB-Tools

   Compiled with Arduino 1.6.11

   Thanks to:
   MichlK - ROM-Reader for Super Nintendo
   Jeff Saltzman - 4-Way Button
   Wayne and Layne - Video-Game-Shield menu
   skaman - SNES enhancements and SA1 sram support
   nocash - Nintendo Power commands and lots of other info
   crazynation - N64 bus timing
   hkz/themanbehindthecurtain - N64 flashram commands
   jago85 - help with N64 stuff
   Andrew Brown/Peter Den Hartog - N64 controller protocol
   bryc - mempak
   Shaun Taylor - N64 controller CRC functions
   Angus Gratton - CRC32
   Tamanegi_taro - SA1 fix
   Snes9x - SuperFX sram fix
   zzattack - multigame pcb fix
   Pickle - SDD1 fix
   insidegadgets - GBCartRead
   RobinTheHood - GameboyAdvanceRomDumper
   YamaArashi - GBA flashrom bank switch command

**********************************************************************************/
char ver[5] = "V19E";

/******************************************
   Define Output
******************************************/
// If you don't have an OLED screen change
// enable_OLED to 0 and enable_Serial to 1
#define enable_OLED 1
#define enable_Serial 0

/******************************************
   Define Input
******************************************/
// If you have two buttons on your cart reader you can remove the //
//#define enable_Button2

/******************************************
   Define SD Speed
******************************************/
// Change to half speed if you get an sd error
#define sdSpeed SPI_FULL_SPEED
//#define sdSpeed SPI_HALF_SPEED

/******************************************
   Pinout
******************************************/
// Please see included pinout.xls

/******************************************
   Libraries
 *****************************************/
// Basic Libs
#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>

// AVR Eeprom
#include <EEPROM.h>
#include "EEPROMAnything.h"

// Graphic I2C LCD
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
// Check if Adafruit_SSD1306.h was setup for 128x64
#if (SSD1306_LCDHEIGHT != 64)
#error("Incorrect height defined in Adafruit_SSD1306.h");
#endif

// Adafruit Clock Generator
#include <si5351.h>
Si5351 clockgen;

// RGB LED
#include <RGBTools.h>

// set pins of red, green and blue
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

// SD Card (Pin 50 = MISO, Pin 51 = MOSI, Pin 52 = SCK, Pin 53 = SS)
#include <SdFat.h>
#include <SdFatUtil.h>
#define chipSelectPin 53
SdFat sd;
SdFile myFile;

/******************************************
  Defines
 *****************************************/
// Mode menu
#define mode_N64_Cart 0
#define mode_N64_Controller 1
#define mode_SNES 2
#define mode_NP 3
#define mode_NPFlash 4
#define mode_NPGame 5
#define mode_GB 6
#define mode_FLASH8 7
#define mode_FLASH16 8
#define mode_GBA 9

/******************************************
   Variables
 *****************************************/
// Button timing
static int debounce = 20; // ms debounce period to prevent flickering when pressing or releasing the button
static int DCgap = 250; // max ms between clicks for a double click event
static int holdTime = 2000; // ms hold period: how long to wait for press+hold event
static int longHoldTime = 5000; // ms long hold period: how long to wait for press+hold event
// Variables for button 1
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
// Variables for button 2
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

// For incoming serial data
int incomingByte;

// Variables for the menu
int choice = 0;
// Temporary array that holds the menu option read out of progmem
char menuOptions[7][20];

// File browser
char fileName[26];
char filePath[36];
byte currPage;
byte lastPage;
byte numPages;
boolean root = 0;
boolean filebrowse = 0;
char fileOptions[30][20];

// Common
char romName[10];
int sramSize = 0;
int romType = 0;
int romSize = 0;
byte numBanks = 128;
char checksumStr[5];
bool errorLvl = 0;
byte romVersion = 0;
char cartID[5];
unsigned long cartSize;
char flashid[5];

// Variable to count errors
unsigned long writeErrors;

// Operation mode
byte mode;

//remember folder number to create a new folder for every save
int foldern;
char folder[24];

// Array that holds the data
byte sdBuffer[512];

//******************************************
// Bitmaps
//******************************************
// Logo
static const unsigned char PROGMEM icon [] = {
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

static const unsigned char PROGMEM sig [] = {
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

/******************************************
  Menu
*****************************************/
// All menus and menu entries are stored in progmem to save on sram
// Main menu
const char modeItem1[] PROGMEM = "Nintendo 64";
const char modeItem2[] PROGMEM = "Super Nintendo";
const char modeItem3[] PROGMEM = "Nintendo Power";
const char modeItem4[] PROGMEM = "Game Boy";
const char modeItem5[] PROGMEM = "Flashrom Programmer";
const char modeItem6[] PROGMEM = "About";
const char* const modeOptions[] PROGMEM = {modeItem1, modeItem2, modeItem3, modeItem4, modeItem5, modeItem6};

// N64 Submenu
const char n64MenuItem1[] PROGMEM = "Cart Slot";
const char n64MenuItem2[] PROGMEM = "Controller";
const char* const menuOptionsN64[] PROGMEM = {n64MenuItem1, n64MenuItem2};

// Flash Submenu
const char flashMenuItem1[] PROGMEM = "8bit slot";
const char flashMenuItem2[] PROGMEM = "16bit slot";
const char* const menuOptionsFlash[] PROGMEM = {flashMenuItem1, flashMenuItem2};

// GBx Submenu
const char gbxMenuItem1[] PROGMEM = "Game Boy Color";
const char gbxMenuItem2[] PROGMEM = "Game Boy Advance";
const char* const menuOptionsGBx[] PROGMEM = {gbxMenuItem1, gbxMenuItem2};

void mainMenu() {
  // create menu with title and 6 options to choose from
  unsigned char modeMenu;
  // Copy menuOptions out of progmem
  convertPgm(modeOptions, 6);
  modeMenu = question_box("Cartridge Reader", menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (modeMenu)
  {
    case 0:
      // create menu with title and 2 options to choose from
      unsigned char n64Dev;
      // Copy menuOptions out of progmem
      convertPgm(menuOptionsN64, 2);
      n64Dev = question_box("Select N64 device", menuOptions, 2, 0);

      // wait for user choice to come back from the question box menu
      switch (n64Dev)
      {
        case 0:
          display_Clear();
          display_Update();
          setup_N64_Cart();
          mode = mode_N64_Cart;
          break;

        case 1:
          display_Clear();
          display_Update();
          setup_N64_Controller();
          mode = mode_N64_Controller;
          break;
      }
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_Snes();
      mode =  mode_SNES;
      break;

    case 2:
      display_Clear();
      display_Update();
      setup_NP();
      mode =  mode_NP;
      break;

    case 3:
      // create menu with title and 2 options to choose from
      unsigned char gbType;
      // Copy menuOptions out of progmem
      convertPgm(menuOptionsGBx, 2);
      gbType = question_box("Select Game Boy", menuOptions, 2, 0);

      // wait for user choice to come back from the question box menu
      switch (gbType)
      {
        case 0:
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;
          break;

        case 1:
          display_Clear();
          display_Update();
          setup_GBA();
          mode =  mode_GBA;
          break;
      }
      break;

    case 4:
      // create menu with title and 2 options to choose from
      unsigned char flashSlot;
      // Copy menuOptions out of progmem
      convertPgm(menuOptionsFlash, 2);
      flashSlot = question_box("Select flashrom slot", menuOptions, 2, 0);

      // wait for user choice to come back from the question box menu
      switch (flashSlot)
      {
        case 0:
          display_Clear();
          display_Update();
          setup_Flash8();
          mode =  mode_FLASH8;
          break;

        case 1:
          display_Clear();
          display_Update();
          setup_Flash16();
          mode =  mode_FLASH16;
          break;
      }
      break;

    case 5:
      display_Clear();
      // Draw the Logo
      display.drawBitmap(0, 0, sig, 128, 64, 1);
      println_Msg(F("Cartridge Reader"));
      println_Msg(F("github.com/sanni"));
      print_Msg(F("2016 "));
      println_Msg(ver);
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F(""));
      println_Msg(F("Press Button"));
      display_Update();

      while (1) {
        if (enable_OLED) {
          // get input button
          int b = checkButton();

          // if the cart readers input button is pressed shortly
          if (b == 1) {
            asm volatile ("  jmp 0");
          }

          // if the cart readers input button is pressed long
          if (b == 3) {
            asm volatile ("  jmp 0");
          }

          // if the button is pressed super long
          if (b == 4) {
            display_Clear();
            println_Msg(F("Resetting folder..."));
            display_Update();
            delay(2000);
            foldern = 0;
            EEPROM_writeAnything(0, foldern);
            asm volatile ("  jmp 0");
          }
        }
        if (enable_Serial) {
          wait_serial();
          asm volatile ("  jmp 0");
        }
        rgb.setColor(random(0, 255), random(0, 255), random(0, 255));
        delay(random(50, 100));
      }
      break;
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

  if (enable_OLED) {
    // GLCD
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    // Clear the buffer.
    display_Clear();

    // Draw the Logo
    display.drawBitmap(28, 0, icon, 72, 64, 1);
    display.setCursor(100, 55);
    display.println(ver);
    display_Update();
    delay(1200);
  }

  if (enable_Serial) {
    // Serial Begin
    Serial.begin(9600);
    Serial.println(F("Cartridge Reader"));
    Serial.println(F("2016 sanni"));
    Serial.println("");

    // Print available RAM
    Serial.print(F("Free Ram: "));
    Serial.print(FreeRam());
    Serial.println(F("Bytes"));
  }

  // Init SD card
  if (!sd.begin(chipSelectPin, sdSpeed)) {
    display_Clear();
    print_Error(F("SD Error"), true);
  }

  if (enable_Serial) {
    // Print SD Info
    Serial.print(F("SD Card: "));
    Serial.print(sd.card()->cardSize() * 512E-9);
    Serial.print(F("GB FAT"));
    Serial.println(int(sd.vol()->fatType()));
  }
  mainMenu();
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
    strcpy_P(menuOptions[i], (char*)pgm_read_word(&(pgmOptions[i])));
  }
}

void print_Error(const __FlashStringHelper *errorMessage, boolean forceReset) {
  errorLvl = 1;
  rgb.setColor(255, 0, 0);
  println_Msg(errorMessage);
  display_Update();

  if (forceReset) {
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
    asm volatile ("  jmp 0");
  }
}

void wait() {
  if (enable_OLED) {
    wait_btn();
  }
  if (enable_Serial) {
    wait_serial();
  }
}

void print_Msg(const __FlashStringHelper *string) {
  if (enable_OLED)
    display.print(string);
  if (enable_Serial)
    Serial.print(string);
}

void print_Msg(const char string[]) {
  if (enable_OLED)
    display.print(string);
  if (enable_Serial)
    Serial.print(string);
}

void print_Msg(long unsigned int message) {
  if (enable_OLED)
    display.print(message);
  if (enable_Serial)
    Serial.print(message);
}

void print_Msg(byte message, int outputFormat) {
  if (enable_OLED)
    display.print(message, outputFormat);
  if (enable_Serial)
    Serial.print(message, outputFormat);
}

void print_Msg(String string) {
  if (enable_OLED)
    display.print(string);
  if (enable_Serial)
    Serial.print(string);
}

void println_Msg(String string) {
  if (enable_OLED)
    display.println(string);
  if (enable_Serial)
    Serial.println(string);
}

void println_Msg(byte message, int outputFormat) {
  if (enable_OLED)
    display.println(message, outputFormat);
  if (enable_Serial)
    Serial.println(message, outputFormat);
}

void println_Msg(const char message[]) {
  if (enable_OLED)
    display.println(message);
  if (enable_Serial)
    Serial.println(message);
}

void println_Msg(const __FlashStringHelper *string) {
  if (enable_OLED)
    display.println(string);
  if (enable_Serial)
    Serial.println(string);
}

void println_Msg(long unsigned int message) {
  if (enable_OLED)
    display.println(message);
  if (enable_Serial)
    Serial.println(message);
}

void display_Update() {
  if (enable_OLED)
    display.display();
}

void display_Clear() {
  if (enable_OLED) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }
}

unsigned char question_box(const char* question, char answers[7][20], int num_answers, int default_choice) {
  if (enable_OLED) {
    return questionBox_OLED(question, answers, num_answers, default_choice);
  }
  if (enable_Serial) {
    return questionBox_Serial(question, answers, num_answers, default_choice);
  }
}

/******************************************
  Serial Out
*****************************************/
void wait_serial() {
  while (Serial.available() == 0) {
  }
  incomingByte = Serial.read() - 48;
  Serial.println("");
}

byte questionBox_Serial(const char* question, char answers[7][20], int num_answers, int default_choice) {
  // Print menu to serial monitor
  Serial.print(question);
  Serial.println(F(" Menu"));
  for (byte i = 0; i < num_answers; i++) {
    Serial.print(i);
    Serial.print(F(")"));
    Serial.println(answers[i]);
  }
  // Wait for user input
  Serial.println("");
  Serial.print(F("Please enter a single number: _ "));
  while (Serial.available() == 0) {
  }

  // Read the incoming byte:
  incomingByte = Serial.read() - 48;

  // Print the received byte for validation e.g. in case of a different keyboard mapping
  Serial.println(incomingByte);
  Serial.println("");
  return incomingByte;
}

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
    if (not ignoreUp1) {
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
    if (not holdEventPast1) {
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
      if (not longholdEventPast1) {
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
    if (not ignoreUp2) {
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
    if (not holdEventPast2) {
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
      if (not longholdEventPast2) {
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
  else
    errorLvl = 0;

  while (1)
  {
    // get input button
    int b = checkButton();

    // Send some clock pulses to the Eeprom in case it locked up
    if (mode == mode_N64_Cart) {
      pulseClock_N64(1);
    }
    // if the cart readers input button is pressed shortly
    if (b == 1) {
      break;
    }

    // if the cart readers input button is pressed long
    if (b == 3) {
      break;
    }
  }
}

// Display a question box with selectable answers. Make sure default choice is in (0, num_answers]
unsigned char questionBox_OLED(const char* question, char answers[7][20], int num_answers, int default_choice) {

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

/******************************************
  Filebrowser Module
*****************************************/
void fileBrowser(const char browserTitle[]) {
  char fileNames[30][26];
  int currFile;
  filebrowse = 1;

  // Empty filePath string
  filePath[0] = '\0';

  // Temporary char array for filename
  char nameStr[26];

browserstart:

  // Set currFile back to 0
  currFile = 0;
  currPage = 1;
  lastPage = 1;

  // Read in File as long as there are files
  while (myFile.openNext(sd.vwd(), O_READ)) {

    // Get name of file
    myFile.getName(nameStr, 27);

    // Ignore if hidden
    if (myFile.isHidden()) {
    }
    // Indicate a directory.
    else if (myFile.isDir()) {
      // Copy full dirname into fileNames
      sprintf(fileNames[currFile], "%s%s", "/", nameStr);
      // Truncate to 19 letters for LCD
      nameStr[19] = '\0';
      // Copy short string into fileOptions
      sprintf(fileOptions[currFile], "%s%s", "/", nameStr);
      currFile++;
    }
    // It's just a file
    else if (myFile.isFile()) {
      // Copy full filename into fileNames
      sprintf(fileNames[currFile], "%s", nameStr);
      // Truncate to 19 letters for LCD
      nameStr[19] = '\0';
      // Copy short string into fileOptions
      sprintf(fileOptions[currFile], "%s", nameStr);
      currFile++;
    }
    myFile.close();
  }

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
    sprintf( answers[i], "%s", fileOptions[ ((currPage - 1) * 7 + i)] );
  }

  // Create menu with title "Filebrowser" and 1-7 options to choose from
  unsigned char answer = question_box(browserTitle, answers, count, 0);

  // Check if the page has been switched
  if (currPage != lastPage) {
    lastPage = currPage;
    goto page;
  }

  // Check if we are supposed to go back to the root dir
  if (root) {
    // Empty filePath string
    filePath[0] = '\0';
    // Rewind filesystem
    //sd.vwd()->rewind();
    // Change working dir to root
    sd.chdir("/");
    // Start again
    root = 0;
    goto browserstart;
  }

  // wait for user choice to come back from the question box menu
  switch (answer)
  {
    case 0:
      strcpy(fileName, fileNames[0 + ((currPage - 1) * 7)]);
      break;

    case 1:
      strcpy(fileName, fileNames[1 + ((currPage - 1) * 7)]);
      break;

    case 2:
      strcpy(fileName, fileNames[2 + ((currPage - 1) * 7)]);
      break;

    case 3:
      strcpy(fileName, fileNames[3 + ((currPage - 1) * 7)]);
      break;

    case 4:
      strcpy(fileName, fileNames[4 + ((currPage - 1) * 7)]);
      break;

    case 5:
      strcpy(fileName, fileNames[5 + ((currPage - 1) * 7)]);
      break;

    case 6:
      strcpy(fileName, fileNames[6 + ((currPage - 1) * 7)]);
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
  if (mode == mode_N64_Controller) {
    n64ControllerMenu();
  }
  else if (mode == mode_N64_Cart) {
    n64CartMenu();
  }
  else if (mode == mode_SNES) {
    snesMenu();
  }
  else if (mode == mode_FLASH8) {
    flashromMenu8();
  }
  else if (mode == mode_FLASH16) {
    flashromMenu16();
  }
  else if (mode == mode_NP) {
    npMenu();
  }
  else if (mode == mode_GB) {
    gbMenu();
  }
  else if (mode == mode_GBA) {
    gbaMenu();
  }
  else if (mode == mode_NPFlash) {
    NPFlashMenu();
  }
  else if (mode == mode_NPGame) {
    NPGameOptions();
  }
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
    asm volatile ("  jmp 0");
  }
}

//******************************************
// End of File
//******************************************
