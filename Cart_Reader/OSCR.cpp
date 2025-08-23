/********************************************************************
*                   Open Source Cartridge Reader                    */
/*H******************************************************************
* FILENAME :        OSCR.cpp
*
* DESCRIPTION :
*       Contains various enums, variables, etc, for the main program.
*
* PUBLIC FUNCTIONS :
*       void    setClockScale( ClockScale )
*       VOLTS   setVoltage( Voltage )
*       long    configGetLong( Key, OnFailure )
*       String  configGetStr( Key )
*
* NOTES :
*       This file is a WIP, I've been moving things into it on my local working
*       copy, but they are not ready yet. Rather than put this in the main file
*       only to move them back again, I decided to commit it early. If you need
*       to add new globals, enums, defines, etc, please use this file!
*
* LICENSE :
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* CHANGES :
*
* REF NO    VERSION  DATE        WHO            DETAIL
*           13.2     2024-03-02  Ancyker        Add string constants
*           13.2     2024-02-29  Ancyker        Add config support
*           12.5     2023-03-29  Ancyker        Initial version
*
*H*/

#include "OSCR.h"

/*==== CONSTANTS ==================================================*/

/**
 * String Constants
 **/
// Firmware Version
constexpr char PROGMEM FSTRING_VERSION[] = "V15.2";

// Universal
constexpr char PROGMEM FSTRING_RESET[] = "Reset";
constexpr char PROGMEM FSTRING_OK[] = "OK";
constexpr char PROGMEM FSTRING_EMPTY[] = "";
constexpr char PROGMEM FSTRING_SPACE[] = " ";
constexpr char PROGMEM FSTRING_CURRENT_SETTINGS[] = "CURRENT SETTINGS";

// Messages
constexpr char PROGMEM FSTRING_OSCR[] = "OSCR";
constexpr char PROGMEM FSTRING_MODULE_NOT_ENABLED[] = "Module is not enabled.";
constexpr char PROGMEM FSTRING_DATABASE_FILE_NOT_FOUND[] = "Database file not found";
constexpr char PROGMEM FSTRING_FILE_DOESNT_EXIST[] = "File doesn't exist";

// Cart
constexpr char PROGMEM FSTRING_READ_ROM[] = "Read ROM";
constexpr char PROGMEM FSTRING_READ_SAVE[] = "Read Save";
constexpr char PROGMEM FSTRING_WRITE_SAVE[] = "Write Save";
constexpr char PROGMEM FSTRING_SELECT_CART[] = "Select Cart";
constexpr char PROGMEM FSTRING_SELECT_CART_TYPE[] = "Select Cart Type";
constexpr char PROGMEM FSTRING_SELECT_FILE[] = "Select file";
constexpr char PROGMEM FSTRING_SET_SIZE[] = "Set Size";
constexpr char PROGMEM FSTRING_REFRESH_CART[] = "Refresh Cart";
constexpr char PROGMEM FSTRING_MAPPER[] = "Mapper: ";
constexpr char PROGMEM FSTRING_SIZE[] = "Size: ";
constexpr char PROGMEM FSTRING_NAME[] = "Name: ";
constexpr char PROGMEM FSTRING_CHECKSUM[] = "Checksum: ";
constexpr char PROGMEM FSTRING_ROM_SIZE[] = "ROM Size: ";
constexpr char PROGMEM FSTRING_REVISION[] = "Revision: ";
constexpr char PROGMEM FSTRING_SERIAL[] = "Serial: ";

/*==== /CONSTANTS =================================================*/

/*==== VARIABLES ==================================================*/

// Clock speed
unsigned long clock = CS_16MHZ;

// Voltage
VOLTS voltage = VOLTS_SET_5V;

#if defined(ENABLE_CONFIG)

FsFile configFile;
bool useConfig;

# if defined(ENABLE_GLOBAL_LOG)
// Logging
bool loggingEnabled = true;
# endif /* ENABLE_GLOBAL_LOG */
#endif /* ENABLE_CONFIG */

/*==== /VARIABLES =================================================*/

/*F******************************************************************
* NAME :            void printVersionToSerial()
*
* DESCRIPTION :     Prints the version & feature string to serial
*                   
*F*/
#if !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
void printVersionToSerial() {
    ClockedSerial.print(FS(FSTRING_OSCR));
    ClockedSerial.print(F("::"));
    ClockedSerial.print(FS(FSTRING_VERSION));
    ClockedSerial.print(F("//"));
  #if defined(HW1)
    ClockedSerial.print(F("HW1"));
  #elif defined(HW2)
    ClockedSerial.print(F("HW2"));
  #elif defined(HW3)
    ClockedSerial.print(F("HW3"));
  #elif defined(HW4)
    ClockedSerial.print(F("HW4"));
  #elif defined(HW5)
    ClockedSerial.print(F("HW5"));
  #elif defined(SERIAL_MONITOR)
    ClockedSerial.print(F("Serial"));
  #else
    ClockedSerial.print(F("HW?"));
  #endif
  #if defined (ENABLE_VSELECT)
    ClockedSerial.print(F("|VSELECT"));
  #endif
  #if defined (ENABLE_RTC)
    ClockedSerial.print(F("|RTC"));
  #endif
  #if defined (ENABLE_CLOCKGEN)
    ClockedSerial.print(F("|CLOCKGEN"));
  #endif
  #if defined (OPTION_N64_FASTCRC)
    ClockedSerial.print(F("|FASTCRC"));
  #endif
  #if defined (ENABLE_3V3FIX)
    ClockedSerial.print(F("|3V3FIX"));
  #endif

  ClockedSerial.println(FS(FSTRING_EMPTY));
}
#else
void printVersionToSerial() {}
#endif

/*F******************************************************************
* NAME :            void setClockScale( ClockScale )
*
* DESCRIPTION :     Set ATMEGA2560 clock prescaler
*
* INPUTS :
*       PARAMETERS:
*           VOLTS      ClockScale              Clock scale
*           CLKSCALE   ClockScale              Clock scale
*
* PROCESS :
*                   [1]  Enable clock prescaler change
*                   [2]  Apply clock prescaler
*
* NOTES :
*       Changing the clock prescaler to a value other than F_CPU (1 by default)
*       can/will result in some clock-based functions not working, including
*       timers and most communication protocols.
*
* FURTHER READING :
*       ATmega640/V-1280/V-1281/V-2560/V-2561/V § 10.13.2 [PDF: https://rmy.pw/atmega2560]
*                   
*F*/
void setClockScale(VOLTS __x)
{
  uint8_t __tmp = _BV(CLKPCE); /*[1]*/
  __asm__ __volatile__ (
    "in __tmp_reg__,__SREG__" "\n\t"
    "cli" "\n\t"
    "sts %1, %0" "\n\t"
    "sts %1, %2" "\n\t"
    "out __SREG__, __tmp_reg__"
    : /* no outputs */
    : "d" (__tmp),
    "M" (_SFR_MEM_ADDR(CLKPR)),
    "d" (__x)
    : "r0"
  ); /*[2]*/
}

void setClockScale(CLKSCALE __x)
{
  clock = (__x == CLKSCALE_16MHZ ? CS_16MHZ : CS_8MHZ);
  uint8_t __tmp = _BV(CLKPCE); /*[1]*/
  __asm__ __volatile__ (
    "in __tmp_reg__,__SREG__" "\n\t"
    "cli" "\n\t"
    "sts %1, %0" "\n\t"
    "sts %1, %2" "\n\t"
    "out __SREG__, __tmp_reg__"
    : /* no outputs */
    : "d" (__tmp),
    "M" (_SFR_MEM_ADDR(CLKPR)),
    "d" (__x)
    : "r0"
  ); /*[2]*/
}

/*F******************************************************************
* NAME :            VOLTS setVoltage( Voltage )
*
* DESCRIPTION :     Adjust voltage (and clock prescaler, if enabled)
*
* INPUTS :
*       PARAMETERS:
*           VOLTS   Voltage              Voltage
*
* OUTPUTS :
*       RETURN :
*            Type:   VOLTS                  Result:
*            Values: VOLTS_SUCCESS          Successfully set the voltage/clock
*                    VOLTS_ERROR            Something went wrong
*                    VOLTS_NOTENABLED       VSELECT and 3V3FIX are both disabled
*
* PROCESS :
*                   [1]  Apply voltage
*                   [2]  Apply clock prescaler
*
* NOTES :
*       When changing to 5V the voltage is set first so that the MPU
*       will be stable at 16MHz. When going down to 3.3V the clock 
*       is changed to 8MHz first so that the MPU will be stable when
*       the voltage is changed to 3.3V.
*
*       This works best with VSELECT as the firmware controls the
*       timing of all of this. If you are doing this manually, then
*       you'll need to start the OSCR with 5V set and only switch to
*       3.3V once on the main menu.
*                   
*F*/
#if defined(ENABLE_VSELECT) || defined(ENABLE_3V3FIX)
VOLTS setVoltage(VOLTS newVoltage) {
  switch(newVoltage) {
    /* 5V */
    case VOLTS_SET_5V:
      if (
  #if defined(ENABLE_3V3FIX)
        clock == CS_16MHZ
  #endif
  #if defined(ENABLE_VSELECT) && defined(ENABLE_3V3FIX)
        &&
  #endif
  #if defined(ENABLE_VSELECT)
        VOLTS_SET_5V == voltage
  #endif
      ) return VOLTS_SUCCESS; // Just return if already as requested

      // Adjust voltage high if VSELECT is available
  #if defined(ENABLE_VSELECT)
      PORTD &= ~(1 << 7); /*[1]*/
      voltage = VOLTS_SET_5V;
  #endif

      // Adjust clock speed when 3V3FIX is enabled
  #if defined(ENABLE_3V3FIX)
      // Stop serial if running
    #if !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
      ClockedSerial.end();
    #endif
      // Set clock speed        
      clock = CS_16MHZ;
      setClockScale(newVoltage); /*[2]*/
      // Restart serial
    #if !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
      ClockedSerial.begin(UPD_BAUD);
    #endif
  #else
      clock = CS_16MHZ;
  #endif

      // Done
      return VOLTS_SUCCESS;

    /* 3.3V */
    case VOLTS_SET_3V3:
      if (
  #if defined(ENABLE_3V3FIX)
        clock == CS_8MHZ
  #endif
  #if defined(ENABLE_VSELECT) && defined(ENABLE_3V3FIX)
        &&
  #endif
  #if defined(ENABLE_VSELECT)
        VOLTS_SET_3V3 == voltage
  #endif
      ) return VOLTS_SUCCESS; // Just return if already as requested

      // Adjust clock speed when 3V3FIX is enabled
  #if defined(ENABLE_3V3FIX)      
    #if !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
      ClockedSerial.end();
    #endif
      // Set clock speed
      clock = CS_8MHZ;
      setClockScale(newVoltage); /*[2]*/
    #if !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
      ClockedSerial.begin(UPD_BAUD);
    #endif
  #endif

      // Adjust voltage high if VSELECT is available
  #if defined(ENABLE_VSELECT)
      PORTD |= (1 << 7); /*[1]*/
      voltage = VOLTS_SET_3V3;
  #endif

      // Done
      return VOLTS_SUCCESS;

    /* ??? */
    default:
      return VOLTS_ERROR;
  }
}
#else
// The compiler will optimize this out when this condition is met.
// Yes, even though it has a return value it will only be compiled
//   if something reads that value. Currently nothing does.
VOLTS setVoltage(VOLTS newVoltage __attribute__((unused))) {
  return VOLTS_NOTENABLED;
}
#endif

#if defined(ENABLE_CONFIG)

/*F******************************************************************
* NAME :            void configInit()
*
* DESCRIPTION :     Setup the config file.
*                   
*F*/
void configInit() {
  useConfig = configFile.open(CONFIG_FILE, O_READ);
}

/*F******************************************************************
* NAME :            uint8_t configFindKey( Key, Value )
*
* DESCRIPTION :     Search for a key=value pair.
*
* INPUTS :
*       PARAMETERS:
*           __FlashStringHelper   Key       The key to get the value for.
*           char*                 value     Variable to store the value in.
*
* OUTPUTS :
*       RETURN :
*            Type:   uint8_t      Length of the value.
*
* PROCESS :
*                   [1]  Get key length and convert to char array/string.
*                   [2]  Parse file line by line.
*                   [3]  Check if key in file matches.
*                   [4]  Copy string after equals character.
*                   [5]  Add null terminator.
*
* NOTES :
*       You aren't meant to use this function directly. Check the
*       functions configGetStr() and configGetLong().
*
*F*/
uint8_t configFindKey(const __FlashStringHelper* searchKey, char* value) {
  if (!useConfig) return 0;

  char key[CONFIG_KEY_MAX + 1];
  char buffer[CONFIG_KEY_MAX + CONFIG_VALUE_MAX + 4];
  int keyLen = 0;
  int valueLen = 0;

  keyLen = strlcpy_P(key, reinterpret_cast<const char *>(searchKey), CONFIG_KEY_MAX); /*[1]*/

  configFile.rewind();

  while (configFile.available()) { /*[2]*/
    int bufferLen = configFile.readBytesUntil('\n', buffer, CONFIG_KEY_MAX + CONFIG_VALUE_MAX + 3);

    if (buffer[bufferLen - 1] == '\r')
      bufferLen--;

    if (bufferLen > (keyLen + 1)) {
      if (memcmp(buffer, key, keyLen) == 0) { /*[3]*/
        if (buffer[keyLen] == '=') { /*[4]*/
          valueLen = bufferLen - keyLen - 1;
          memcpy(&value[0], &buffer[keyLen + 1], valueLen);
          value[valueLen] = '\0'; /*[5]*/
          break;
        }
      }
    }
  }

  return valueLen;
}

/*F******************************************************************
* NAME :            String configGetStr( Key )
*
* DESCRIPTION :     Return the value of a key as a string.
*
* INPUTS :
*       PARAMETERS:
*           __FlashStringHelper   Key       The key to get the value for.
*
* OUTPUTS :
*       RETURN :
*            Type:   String       The value of the key or an empty string.
*
* PROCESS :
*                   [1]  Find the key via configFindKey().
*                   [2]  Return empty if nothing was found.
*                   [3]  Convert to String type.
*
* NOTES :
*       You can use this to get strings stored in the config file.
*       Take care when allocating memory for strings. You should 
*       probably use malloc for it if it's a global variable. If
*       you do, make sure to free() it after it's not needed.
*
*F*/
String configGetStr(const __FlashStringHelper* key) {
  if (!useConfig) return {};
  char value[CONFIG_VALUE_MAX + 1];
  
  uint8_t valueLen = configFindKey(key, value); /*[1]*/
  if (valueLen < 1) return {}; /*[2]*/

  String stringVal(value); /*[3]*/

  return stringVal;
}

/*F******************************************************************
* NAME :            long configGetLong( Key, OnFailure )
*
* DESCRIPTION :     Return the value of a key as an int/long.
*
* INPUTS :
*       PARAMETERS:
*           __FlashStringHelper   Key      The key to get the value for.
*           int                   onFail   Value to return on failure. (def=0)
*
* OUTPUTS :
*       RETURN :
*            Type:   int          The value of the key or onFail.
*
* PROCESS :
*                   [1]  Find the key via configFindKey().
*                   [2]  Return onFail if nothing was found.
*                   [3]  Convert to long int type and return.
*
* NOTES :
*       You can specify hex, i.e. 0xFF for 255, if you want to.
*
*F*/
long configGetLong(const __FlashStringHelper* key, int onFail) {
  if (!useConfig) return onFail;
  char value[CONFIG_VALUE_MAX + 1];

  uint8_t valueLen = configFindKey(key, value); /*[1]*/
  if (valueLen < 1) return onFail; /*[2]*/

  return strtol(value, NULL, 0); /*[3]*/
}

#endif /* ENABLE_CONFIG */
