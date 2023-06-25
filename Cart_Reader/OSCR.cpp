/********************************************************************
*                   Open Source Cartridge Reader                    */
/*H******************************************************************
* FILENAME :        OSCR.cpp
*
* DESCRIPTION :
*       Contains various enums, variables, etc, for the main program.
*
* PUBLIC FUNCTIONS :
*       void    setClockScale( VOLTS )
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
*           12.5     2023-03-29  Ancyker        Initial version
*
*H*/

#include "OSCR.h"

/*==== VARIABLES ==================================================*/

// Firmware Version
char ver[5] = "12.6";

// Clock speed
unsigned long clock = CS_16MHZ;

// Voltage
VOLTS voltage = VOLTS_SET_5V;

/*==== /VARIABLES =================================================*/

/*F******************************************************************
* NAME :            void printVersionToSerial()
*
* DESCRIPTION :     Prints the version & feature string to serial
*                   
*F*/
#if !defined(enable_serial) && defined(ENABLE_UPDATER)
void printVersionToSerial() {
    ClockedSerial.print(F("OSCR"));
    ClockedSerial.print(F("::"));
    ClockedSerial.print(ver);
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
  #if defined (clockgen_installed)
    ClockedSerial.print(F("|CLOCKGEN"));
  #endif
  #if defined (fastcrc)
    ClockedSerial.print(F("|FASTCRC"));
  #endif
  #if defined (ENABLE_3V3FIX)
    ClockedSerial.print(F("|3V3FIX"));
  #endif

  ClockedSerial.println(F(""));
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
*           VOLTS   ClockScale              Clock scale
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
VOLTS setVoltage(VOLTS volts) {
  switch(volts) {
    /* 5V */
    case VOLTS_SET_5V:
      if (clock == CS_16MHZ && volts == VOLTS_SET_5V) return VOLTS_SUCCESS; // Just return if already as requested

      // Adjust voltage high if VSELECT is available
      #if defined(ENABLE_VSELECT)
        PORTD &= ~(1 << 7); /*[1]*/
        voltage = VOLTS_SET_5V;
      #endif

      // Adjust clock speed when 3V3FIX is enabled
      #if defined(ENABLE_3V3FIX)
        // Stop serial if running
        #if !defined(enable_serial) && defined(ENABLE_UPDATER)
          ClockedSerial.end();
        #endif
          // Set clock speed        
          clock = CS_16MHZ;
          setClockScale(volts); /*[2]*/
        // Restart serial
        #if !defined(enable_serial) && defined(ENABLE_UPDATER)
          ClockedSerial.begin(UPD_BAUD);
        #endif
      #else
          clock = CS_16MHZ;
      #endif

      // Done
      return VOLTS_SUCCESS;

    /* 3.3V */
    case VOLTS_SET_3V3:
      if (clock == CS_8MHZ && volts == VOLTS_SET_3V3) return VOLTS_SUCCESS; // Just return if already as requested

      // Adjust clock speed when 3V3FIX is enabled
      #if defined(ENABLE_3V3FIX)      
        #if !defined(enable_serial) && defined(ENABLE_UPDATER)
          ClockedSerial.end();
        #endif
        clock = CS_8MHZ;
        setClockScale(volts); /*[2]*/
        #if !defined(enable_serial) && defined(ENABLE_UPDATER)
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
VOLTS setVoltage(VOLTS volts __attribute__((unused))) {
  return VOLTS_NOTENABLED;
}
#endif
