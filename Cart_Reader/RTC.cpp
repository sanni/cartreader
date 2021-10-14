#include "RTC.h"
#include "SdFat.h"

RTC_DS3231 rtc;

// Start Time
void RTCStart() {
  // Start RTC
  if (! rtc.begin()) {
    abort();
  }

  // Set RTC Date/Time of Sketch Build if it lost battery power
  // After initial setup it would have lost battery power ;)
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Set Date/Time Callback Funtion
// Callback for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // Return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // Return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


/******************************************
  RTC Time Stamp Setup
  Call in any other script
*****************************************/
// Format a Date/Time stamp
String RTCStamp() {
  // Set a format
  char dtstamp[] = "DDMMMYYYY hh:mm:ssAP";

  // Get current Date/Time
  DateTime now = rtc.now();

  // Convert it to a string and caps lock it
  String dts = now.toString(dtstamp);
  dts.toUpperCase();

  // Print results
  return dts;
}
