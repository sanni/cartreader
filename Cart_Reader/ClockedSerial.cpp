/********************************************************************
*        Open Source Cartridge Reader for Arduino Mega 2560        */
/*H******************************************************************
* FILENAME :        ClockedSerial.cpp
*
* DESCRIPTION :
*       Modified HardwareSerial class for using with a dynamic clock speed.
*
* PUBLIC FUNCTIONS :
*       void    DynamicClockSerial::begin(baud, config, sclock)
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
#include "ClockedSerial.h"

/*
* This function is unchanged, including comments, from HardwareSerial. Comments not from
*  the original function are denoted with a prefix of "(ClockedSerial)".
*
* The parameter `sclock` is used to let it know the clockspeed. It replaces the usage of
*  the F_CPU preprocessor variable. Unlike `clock_prescale_set` this parameter is the 
*  speed in MHz, i.e. 16000000 (16MHz).
*/
void DynamicClockSerial::begin(unsigned long baud, byte config, unsigned long sclock)
{ 
  // Try u2x mode first
  uint16_t baud_setting = (sclock / 4 / baud - 1) / 2;
  *_ucsra = 1 << U2X0;

  // hardcoded exception for 57600 for compatibility with the bootloader
  // shipped with the Duemilanove and previous boards and the firmware
  // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
  // be > 4095, so switch back to non-u2x mode if the baud rate is too
  // low.
  if (((sclock == 16000000UL) && (baud == 57600)) || (baud_setting > 4095)) /* (ClockedSerial) F_CPU -> sclock variable/parameter */
  {
    *_ucsra = 0;
    baud_setting = (sclock / 8 / baud - 1) / 2; /* (ClockedSerial) This is where we adjust things based on clock speed; F_CPU -> sclock variable/parameter */
  }

  // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  _written = false;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;
  
  sbi(*_ucsrb, RXEN0);
  sbi(*_ucsrb, TXEN0);
  sbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
}

// ClockedSerial setup
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL) && !defined(ENABLE_SERIAL) && defined(ENABLE_UPDATER)
  #if defined(UBRRH) && defined(UBRRL)
    DynamicClockSerial ClockedSerial(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR);
  #else
    DynamicClockSerial ClockedSerial(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);
  #endif

  #if defined(USART_RX_vect)
    ISR(USART_RX_vect)
  #elif defined(USART0_RX_vect)
    ISR(USART0_RX_vect)
  #elif defined(USART_RXC_vect)
    ISR(USART_RXC_vect) // ATmega8
  #else
    #error "Don't know what the Data Received vector is called for Serial"
  #endif
    {
      ClockedSerial._rx_complete_irq();
    }

  #if defined(UART0_UDRE_vect)
  ISR(UART0_UDRE_vect)
  #elif defined(UART_UDRE_vect)
  ISR(UART_UDRE_vect)
  #elif defined(USART0_UDRE_vect)
  ISR(USART0_UDRE_vect)
  #elif defined(USART_UDRE_vect)
  ISR(USART_UDRE_vect)
  #else
    #error "Don't know what the Data Register Empty vector is called for Serial"
  #endif
  {
    ClockedSerial._tx_udr_empty_irq();
  }

  bool Serial0_available() {
    return ClockedSerial.available();
  }
#endif
