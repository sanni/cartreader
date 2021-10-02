// RTC Library
#ifndef _RTC_H
#define _RTC_H

#include "RTClib.h"

void RTCStart();

void dateTime(uint16_t* date, uint16_t* time);

String RTCStamp();

#endif
