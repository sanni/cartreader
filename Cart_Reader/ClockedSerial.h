/********************************************************************
*                   Open Source Cartridge Reader                    *
********************************************************************/
#ifndef CLOCKEDSERIAL_H_
#define CLOCKEDSERIAL_H_

#include <HardwareSerial.h>
#include <HardwareSerial_private.h>

/*C******************************************************************
* NAME :            ClockedSerial
*
* DESCRIPTION :     HardwareSerial wrapper that allows for dynamic clock speed adjustments.
*
* USAGE :           See HardwareSerial
*
* NOTES :           If this class isn't used the compiler will optimize it out, so there is
*                   no harm in leaving it.
*C*/
class DynamicClockSerial : public HardwareSerial
{
  using HardwareSerial::HardwareSerial;
  public:
  // Various functions to allow parameter omission and automatic handling.
  void begin(unsigned long baud) { begin(baud, SERIAL_8N1, clock); }
  void begin(unsigned long baud, byte config) { begin(baud, config, clock); }
  void begin(unsigned long baud, unsigned long sclock) { begin(baud, SERIAL_8N1, sclock); }
  void begin(unsigned long baud, byte config, unsigned long sclock);
};

extern DynamicClockSerial ClockedSerial;

#endif /* CLOCKEDSERIAL_H_ */
