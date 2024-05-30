/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#ifndef OSCR_H_
#define OSCR_H_

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "SdFat.h"

#include "Config.h"

/*==== SANITY CHECKS ==============================================*/
# if !(defined(HW1) || defined(HW2) || defined(HW3) || defined(HW4) || defined(HW5) || defined(SERIAL_MONITOR))
#   error !!! PLEASE CHOOSE HARDWARE VERSION IN CONFIG.H !!!
# endif

# if defined(ENABLE_3V3FIX) && !defined(ENABLE_VSELECT)
#   warning Using 3V3FIX is best with VSELECT.
# endif

/*==== CONSTANTS ==================================================*/
/**
 * String Constants
 **/

// Version
extern const char PROGMEM FSTRING_VERSION[];

// Universal
extern const char PROGMEM FSTRING_OK[];
extern const char PROGMEM FSTRING_EMPTY[];
extern const char PROGMEM FSTRING_SPACE[];
extern const char PROGMEM FSTRING_RESET[];
extern const char PROGMEM FSTRING_CURRENT_SETTINGS[];

// Messages
extern const char PROGMEM FSTRING_OSCR[];
extern const char PROGMEM FSTRING_MODULE_NOT_ENABLED[];
extern const char PROGMEM FSTRING_DATABASE_FILE_NOT_FOUND[];
extern const char PROGMEM FSTRING_FILE_DOESNT_EXIST[];

// Cart
extern const char PROGMEM FSTRING_READ_ROM[];
extern const char PROGMEM FSTRING_READ_SAVE[];
extern const char PROGMEM FSTRING_WRITE_SAVE[];
extern const char PROGMEM FSTRING_SELECT_CART[];
extern const char PROGMEM FSTRING_SELECT_CART_TYPE[];
extern const char PROGMEM FSTRING_SET_SIZE[];
extern const char PROGMEM FSTRING_REFRESH_CART[];
extern const char PROGMEM FSTRING_MAPPER[];
extern const char PROGMEM FSTRING_SIZE[];
extern const char PROGMEM FSTRING_ROM_SIZE[];
extern const char PROGMEM FSTRING_NAME[];
extern const char PROGMEM FSTRING_CHECKSUM[];

#define FS(pmem_string) (reinterpret_cast<const __FlashStringHelper *>(pmem_string))

/**
 * Other Constants
 **/
// Updater baud rate
constexpr uint16_t UPD_BAUD = 9600;
// Clock speeds
constexpr unsigned long CS_16MHZ = 16000000UL;
constexpr unsigned long CS_8MHZ = 8000000UL;

enum CORES: uint8_t {
# ifdef ENABLE_N64
  CORE_N64_CART,
  CORE_N64_CONTROLLER,
# endif
# ifdef ENABLE_SNES
  CORE_SNES,
# endif
# ifdef ENABLE_SFM
  CORE_SFM,
#   ifdef ENABLE_FLASH
  CORE_SFM_FLASH,
#   endif
  CORE_SFM_GAME,
# endif
# ifdef ENABLE_GBX
  CORE_GB,
  CORE_GBA,
  CORE_GBM,
  CORE_GB_GBSMART,
  CORE_GB_GBSMART_FLASH,
  CORE_GB_GBSMART_GAME,
# endif
# ifdef ENABLE_FLASH
  CORE_FLASH8,
#   ifdef ENABLE_FLASH16
  CORE_FLASH16,
  CORE_EPROM,
#   endif
# endif
# ifdef ENABLE_MD
  CORE_MD_CART,
  CORE_SEGA_CD,
# endif
# ifdef ENABLE_PCE
  CORE_PCE,
# endif
# ifdef ENABLE_SV
  CORE_SV,
# endif
# ifdef ENABLE_NES
  CORE_NES,
# endif
# ifdef ENABLE_SMS
  CORE_SMS,
# endif
# ifdef ENABLE_WS
  CORE_WS,
# endif
# ifdef ENABLE_NGP
  CORE_NGP,
# endif
# ifdef ENABLE_INTV
  CORE_INTV,
# endif
# ifdef ENABLE_COLV
  CORE_COL,
# endif
# ifdef ENABLE_VBOY
  CORE_VBOY,
# endif
# ifdef ENABLE_WSV
  CORE_WSV,
# endif
# ifdef ENABLE_PCW
  CORE_PCW,
# endif
# ifdef ENABLE_ODY2
  CORE_ODY2,
# endif
# ifdef ENABLE_ARC
  CORE_ARC,
# endif
# ifdef ENABLE_FAIRCHILD
  CORE_FAIRCHILD,
# endif
# ifdef ENABLE_SUPRACAN
  CORE_SUPRACAN,
# endif
# ifdef ENABLE_MSX
  CORE_MSX,
# endif
# ifdef ENABLE_POKE
  CORE_POKE,
# endif
# ifdef ENABLE_LOOPY
  CORE_LOOPY,
# endif
# ifdef ENABLE_C64
  CORE_C64,
# endif
# ifdef ENABLE_2600
  CORE_2600,
# endif
# ifdef ENABLE_5200
  CORE_5200,
# endif
# ifdef ENABLE_7800
  CORE_7800,
# endif
# ifdef ENABLE_VECTREX
  CORE_VECTREX,
# endif
# ifdef ENABLE_ST
  CORE_ST,
# endif
# ifdef ENABLE_GPC
  CORE_GPC,
# endif
  CORE_MAX // Always last
};

enum SYSTEM_MENU: uint8_t {
# if defined(ENABLE_GBX)
  SYSTEM_MENU_GBX,
# endif
# if defined(ENABLE_NES)
  SYSTEM_MENU_NES,
# endif
# if defined(ENABLE_SNES)
  SYSTEM_MENU_SNES,
# endif
# if defined(ENABLE_N64)
  SYSTEM_MENU_N64,
# endif
# if defined(ENABLE_MD)
  SYSTEM_MENU_MD,
# endif
# if defined(ENABLE_SMS)
  SYSTEM_MENU_SMS,
# endif
# if defined(ENABLE_PCE)
  SYSTEM_MENU_PCE,
# endif
# if defined(ENABLE_WS)
  SYSTEM_MENU_WS,
# endif
# if defined(ENABLE_NGP)
  SYSTEM_MENU_NGP,
# endif
# if defined(ENABLE_INTV)
  SYSTEM_MENU_INTV,
# endif
# if defined(ENABLE_COLV)
  SYSTEM_MENU_COLV,
# endif
# if defined(ENABLE_VBOY)
  SYSTEM_MENU_VBOY,
# endif
# if defined(ENABLE_WSV)
  SYSTEM_MENU_WSV,
# endif
# if defined(ENABLE_PCW)
  SYSTEM_MENU_PCW,
# endif
# if defined(ENABLE_2600)
  SYSTEM_MENU_2600,
# endif
# if defined(ENABLE_ODY2)
  SYSTEM_MENU_ODY2,
# endif
# if defined(ENABLE_ARC)
  SYSTEM_MENU_ARC,
# endif
# if defined(ENABLE_FAIRCHILD)
  SYSTEM_MENU_FAIRCHILD,
# endif
# if defined(ENABLE_SUPRACAN)
  SYSTEM_MENU_SUPRACAN,
# endif
# if defined(ENABLE_MSX)
  SYSTEM_MENU_MSX,
# endif
# if defined(ENABLE_POKE)
  SYSTEM_MENU_POKE,
# endif
# if defined(ENABLE_LOOPY)
  SYSTEM_MENU_LOOPY,
# endif
# if defined(ENABLE_C64)
  SYSTEM_MENU_C64,
# endif
# if defined(ENABLE_5200)
  SYSTEM_MENU_5200,
# endif
# if defined(ENABLE_7800)
  SYSTEM_MENU_7800,
# endif
# if defined(ENABLE_VECTREX)
  SYSTEM_MENU_VECTREX,
# endif
# if defined(ENABLE_FLASH)
  SYSTEM_MENU_FLASH,
# endif
# if defined(ENABLE_SELFTEST)
  SYSTEM_MENU_SELFTEST,
# endif
  SYSTEM_MENU_ABOUT,
  SYSTEM_MENU_RESET,

  // Total number of options available.
  SYSTEM_MENU_TOTAL // Always immediately after last menu option
};

// ENUM for VSELECT & 3V3FIX
enum CLKSCALE: uint8_t {
  // Paramters to pass to setVoltage() and setClockScale()
  CLKSCALE_16MHZ = 0, // ClockScale 0 = 16MHz
  CLKSCALE_8MHZ,      // ClockScale 1 = 8MHz
};

// ENUM for VSELECT & 3V3FIX
enum VOLTS: uint8_t {
  // Paramters to pass to setVoltage() and setClockScale()
  VOLTS_SET_5V = 0, // 5V parameter    [ClockScale 0 = 16MHz, Voltage = 5V]
  VOLTS_SET_3V3,    // 3.3V parameter  [ClockScale 1 = 8MHz, Voltage = 3.3V]
  // Don't use the following as parameters
  // Return values:
  VOLTS_SUCCESS,    // Return value for success
  VOLTS_ERROR,      // Return value for error
  VOLTS_NOTENABLED, // Return value for not being enabled
  VOLTS_UNKNOWN     // Return value for all other states
};

/*==== /CONSTANTS =================================================*/

/*==== VARIABLES ==================================================*/
extern unsigned long clock;
//extern char ver[5];
extern VOLTS voltage;

# if defined(ENABLE_CONFIG)
  /**
   * Config File Stuff
   *
   * You can register GLOBAL configuration variables in this section.
   * You should put core-specific config variables in the related file.
   **/

  extern bool useConfig;
#   ifdef ENABLE_GLOBAL_LOG
  extern bool loggingEnabled;
#   endif /* ENABLE_GLOBAL_LOG */
# else /* !ENABLE_CONFIG */
#   ifdef ENABLE_GLOBAL_LOG
  constexpr bool loggingEnabled = true;
#   endif /* ENABLE_GLOBAL_LOG */
# endif /* ENABLE_CONFIG */

/*==== /VARIABLES =================================================*/

/*==== FUNCTIONS ==================================================*/

extern void printVersionToSerial();
extern void setClockScale(VOLTS __x);
extern void setClockScale(CLKSCALE __x);
extern VOLTS setVoltage(VOLTS volts);

# if defined(ENABLE_CONFIG)
extern void configInit();
extern uint8_t configFindKey(const __FlashStringHelper* key, char* value);
extern String configGetStr(const __FlashStringHelper* key);
extern long configGetLong(const __FlashStringHelper* key, int onFail = 0);
# endif /* ENABLE_CONFIG */

/*==== /FUNCTIONS =================================================*/

#include "ClockedSerial.h"

#endif /* OSCR_H_ */
