//******************************************
// !!! CHOOSE HARDWARE VERSION !!!
//******************************************
// Remove // in front of the line with your hardware version
//#define HW5
//#define HW4
//#define HW3
//#define HW2
//#define HW1
//#define SERIAL_MONITOR

#if !(defined(HW1) || defined(HW2) || defined(HW3) || defined(HW4) || defined(HW5) || defined(SERIAL_MONITOR))
# error !!! PLEASE CHOOSE HARDWARE VERSION IN OPTIONS.H !!!
#endif
//******************************************
//
//******************************************

//******************************************
// Hardware Configurations
//******************************************
#if (defined(HW4) || defined(HW5))
#define enable_LCD
#define enable_neopixel
#define background_color 100,0,0 //Green, Red, Blue
#define enable_rotary
//#define rotate_counter_clockwise
#define clockgen_installed
#define fastcrc
#define ws_adapter_v2
#endif

#if (defined(HW2) || defined(HW3))
#define enable_OLED
#define enable_Button2
//#define clockgen_installed
//#define fastcrc
#endif

#if defined(HW1)
#define enable_OLED
#endif

#if defined(SERIAL_MONITOR)
#define enable_serial
#endif

//******************************************
// GLOBAL OPTIONS
//******************************************
// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

//Ignores errors that normally force a reset if button 2 is pressed
//#define debug_mode

// Setup RTC if installed.
// remove // if you have an RTC installed
// #define RTC_installed

// Use calibration data from snes_clk.txt
// #define clockgen_calibration

// Write all info to log.txt in root dir
//#define global_log

// Use Adafruit Clock Generator
// #define clockgen_installed

//******************************************
// GB OPTIONS
//******************************************
// Renames ROM if found in database (slow)
//#define no-intro

//******************************************
// N64 OPTIONS
//******************************************
// The CRC for N64 Roms will be calculated during dumping from memory instead of after dumping from SD card, not compatible to all Cart Readers
// #define fastcrc

// saves a n64log.txt file with rom info in /N64/ROM
// #define savesummarytotxt

//******************************************
// DISABLE MODULES
//******************************************
// add // before #define to disable a module
#define enable_SNES
#define enable_NP
#define enable_SV

#define enable_MD
#define enable_SMS

#define enable_N64
#define enable_GBX
#define enable_NES
#define enable_FLASH
#define enable_PCE
#define enable_WS
#define enable_NGP
