//******************************************
// CHOOSE HARDWARE VERSION
//******************************************
//#define HW4
#define HW3
//#define HW2
//#define HW1
//#define SERIAL

#if defined(HW4)
#define enable_LCD
#define enable_neopixel
#define enable_rotary
#endif

#if defined(HW2) || defined(HW3)
#define enable_OLED
#define enable_Button2
#endif

#if defined(HW1)
#define enable_OLED
#endif

#if defined(SERIAL)
#define enable_serial
#endif

//******************************************
// GLOBAL OPTIONS
//******************************************
// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

// Skip start-up animation
// #define fast_start

// Setup RTC if installed.
// remove // if you have an RTC installed
// #define RTC_installed

// Use calibration data from snes_clk.txt
// #define clockgen_calibration

//******************************************
// ENABLED MODULES
//******************************************
// add // before #define to disable a module
#define enable_FLASH
#define enable_GBX
#define enable_MD
#define enable_N64
#define enable_NES
#define enable_NGP
#define enable_NP
#define enable_PCE
#define enable_SMS
#define enable_SNES
#define enable_SV
#define enable_WS

//******************************************
// N64 OPTIONS
//******************************************
// Read N64 Eeprom with Adadruit clockgen, CLK1 switch needs to be switch to ON
// add // and disable CLK1 switch if you don't have the clockgen installed or if you want to read a repros save
// #define clockgen_installed

// The CRC for N64 Roms will be calculated during dumping from memory instead of after dumping from SD card, not compatible to all Cart Readers
// #define fastcrc

// saves a n64log.txt file with rom info in /N64/ROM
// #define savesummarytotxt
