//******************************************
// GLOBAL OPTIONS
//******************************************
// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

// Comment out to change to Serial Output
// be sure to change the Arduino Serial Monitor to no line ending
#define enable_OLED
// Skip OLED start-up animation
// #define fast_start

// Enable the second button
#define enable_Button2

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
