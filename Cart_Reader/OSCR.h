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
#if !(defined(HW1) || defined(HW2) || defined(HW3) || defined(HW4) || defined(HW5) || defined(SERIAL_MONITOR))
  #error !!! PLEASE CHOOSE HARDWARE VERSION IN CONFIG.H !!!
#endif

#if defined(ENABLE_3V3FIX) && !defined(ENABLE_VSELECT)
  #warning Using 3V3FIX is best with VSELECT.
#endif

/*==== CONSTANTS ==================================================*/
// Updater baud rate
const uint16_t UPD_BAUD = 9600;
// Clock speeds
const unsigned long CS_16MHZ = 16000000UL;
const unsigned long CS_8MHZ = 8000000UL;

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

/*==== VARIABLES ==================================================*/
extern unsigned long clock;
extern char ver[5];
extern VOLTS voltage;

#if defined(ENABLE_CONFIG)
  /**
   * Config File Stuff
   *
   * You can register GLOBAL configuration variables in this section.
   * You should put core-specific config variables in the related file.
   **/

  extern bool useConfig;
# ifdef global_log
  extern bool loggingEnabled;
# endif /* global_log */
#else /* !ENABLE_CONFIG */
# ifdef global_log
  extern const bool loggingEnabled;
# endif /* global_log */
#endif /* ENABLE_CONFIG */

/*==== FUNCTIONS ==================================================*/
extern void printVersionToSerial();
extern void setClockScale(VOLTS __x);
extern void setClockScale(CLKSCALE __x);
extern VOLTS setVoltage(VOLTS volts);

#if defined(ENABLE_CONFIG)
extern void configInit();
extern uint8_t configFindKey(const __FlashStringHelper* key, char* value);
extern String configGetStr(const __FlashStringHelper* key);
extern long configGetLong(const __FlashStringHelper* key, int onFail = 0);
#endif /* ENABLE_CONFIG */

#include "ClockedSerial.h"

#endif /* OSCR_H_ */
