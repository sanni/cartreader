/**********************************************************************************
                     Nintendo Power Writer for Arduino Mega2560

   Author:           sanni
   Date:             2017-11-13
   Version:          V10

   SD  lib:          https://github.com/greiman/SdFat

   Many thanks to MichlK, skaman and nocash
   Please visit:  http://forum.arduino.cc/index.php?topic=158974.0
   And also:      http://forums.nesdev.com/viewtopic.php?f=12&t=11453
 *********************************************************************************/

/******************************************
   Libraries
 *****************************************/
// SD Card (Pin 50 = MISO, Pin 51 = MOSI, Pin 52 = SCK, Pin 53 = SS)
#include <SPI.h>
#include <SdFat.h>
#define chipSelectPin 53
SdFat sd;
SdBaseFile myFile;

/******************************************
  Pinout
 *****************************************/
// Address Pins
#define a0Pin A0
#define a1Pin A1
#define a2Pin A2
#define a3Pin A3
#define a4Pin A4
#define a5Pin A5
#define a6Pin A6
#define a7Pin A7
#define a8Pin A8
#define a9Pin A9
#define a10Pin A10
#define a11Pin A11
#define a12Pin A12
#define a13Pin A13
#define a14Pin A14
#define a15Pin A15
#define ba0Pin 49 //a16 
#define ba1Pin 48 //a17 
#define ba2Pin 47 //a18 
#define ba3Pin 46 //a19 
#define ba4Pin 45 //a20 
#define ba5Pin 44 //a21
#define ba6Pin 43 //a22
#define ba7Pin 42 //a23

// Control Pins
#define csPin 6
#define irqPin 7
#define wrPin 8
#define rdPin 9

// Data Pins
#define d0Pin 37
#define d1Pin 36
#define d2Pin 35
#define d3Pin 34
#define d4Pin 33
#define d5Pin 32
#define d6Pin 31
#define d7Pin 30

// Clock & Reset Pin
#define rstPin 17
#define clkPin 5

/******************************************
   Variables
 *****************************************/
// Buffer for the SD
byte SDBuffer[512];
byte SDBuffer2[512];

// Filename of SD file
char fileName[13];

// Nintendo Power status
byte NPReady = 0;

// Flashrom ID and size
char flashid[5];
unsigned long flashSize = 4194304;
byte numBanks = 128;
boolean romType = 0;
byte sramSize = 64;

// Variable to count errors
unsigned long writeErrors;

// For incoming serial data
int incomingByte;

/******************************************
   Setup
 *****************************************/
void setup() {
  // Clock Pin to Output
  DDRE |=  (1 << 3);
  // Clock Pin to Low
  PORTE &= ~(1 << 3);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Serial Begin
  Serial.begin(9600);
  Serial.println("--------------------------------------------------------->");
  Serial.println("Nintendo Power Writer V10 2017 sanni");
  Serial.println("--------------------------------------------------------->");

  // Init SD card
  if (!sd.begin(chipSelectPin, SPI_FULL_SPEED)) {
    Serial.println("SD Error.");
    while (1);
  }

  // Print SD Info
  Serial.print("SD Card: ");
  Serial.print(sd.card()->cardSize() * 512E-9);
  Serial.print("GB FAT");
  Serial.println(int(sd.vol()->fatType()));
  Serial.println("");
  Serial.println("FILES---------------------------------------------------->");
  // Print all files in root of SD
  Serial.println("Name - Size");
  sd.vwd()->rewind();
  while (myFile.openNext(sd.vwd(), O_READ)) {
    if (myFile.isHidden()) {
    }
    else {
      if (myFile.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      }
      myFile.printName(&Serial);
      Serial.write(' ');
      myFile.printFileSize(&Serial);
      Serial.println();
    }
    myFile.close();
  }
  Serial.println("");
}

/******************************************
   Helper functions
 *****************************************/
// Prompt a filename from the Serial Monitor
void getfilename() {
  Serial.println("");
  Serial.println("-------> Please enter a filename in 8.3 format: _ <-------");
  Serial.println("");
  while (Serial.available() == 0) {
  }
  String strBuffer;
  strBuffer = Serial.readString();
  strBuffer.toCharArray(fileName, 13);
}

// Send a clock pulse
void pulseClock(int times) {
  for (int i = 0; i < times * 2; i++) {
    // Switch the clock pin to 0 if it's 1 and 0 if it's 1
    PORTE ^= (1 << 3);
    // Wait 62.5ns
    __asm__("nop\n\t");
  }
}

/******************************************
   I/O Functions
 *****************************************/
// Switch control pins to write
void controlOut() {
  // Switch RD(PH6) and WR(PH5) to HIGH
  PORTH |= (1 << 6) | (1 << 5);
  // Switch CS(PH3) to LOW
  PORTH &= ~(1 << 3);
}

// Switch control pins to read
void controlIn() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
}

// Switch data pins to write
void dataOut() {
  DDRC = 0xFF;
}

// Switch data pins to read
void dataIn() {
  DDRC = 0x00;
  // Pullups
  //PORTC = 0xFF;
}

// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank(byte myBank, word myAddress, byte myData) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Switch WE(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");
}

// Write a byte to the data pins and pulse the clock pin, used when sending to the MX15001 chip
void writeBankClock(byte myBank, word myAddress, byte myData) {
  if ( (myAddress & 1) == 1 ) digitalWrite(a0Pin, 1); else digitalWrite(a0Pin, 0);
  if ( (myAddress & 2) == 2 ) digitalWrite(a1Pin, 1); else digitalWrite(a1Pin, 0);
  if ( (myAddress & 4) == 4 ) digitalWrite(a2Pin, 1); else digitalWrite(a2Pin, 0);
  if ( (myAddress & 8) == 8 ) digitalWrite(a3Pin, 1); else digitalWrite(a3Pin, 0);
  if ( (myAddress & 16) == 16 ) digitalWrite(a4Pin, 1); else digitalWrite(a4Pin, 0);
  if ( (myAddress & 32) == 32 ) digitalWrite(a5Pin, 1); else digitalWrite(a5Pin, 0);
  if ( (myAddress & 64) == 64 ) digitalWrite(a6Pin, 1); else digitalWrite(a6Pin, 0);
  if ( (myAddress & 128) == 128 ) digitalWrite(a7Pin, 1); else digitalWrite(a7Pin, 0);
  if ( (myAddress & 256) == 256 ) digitalWrite(a8Pin, 1); else digitalWrite(a8Pin, 0);
  if ( (myAddress & 512) == 512 ) digitalWrite(a9Pin, 1); else digitalWrite(a9Pin, 0);
  if ( (myAddress & 1024) == 1024 ) digitalWrite(a10Pin, 1); else digitalWrite(a10Pin, 0);
  if ( (myAddress & 2048) == 2048 ) digitalWrite(a11Pin, 1); else digitalWrite(a11Pin, 0);
  if ( (myAddress & 4096) == 4096 ) digitalWrite(a12Pin, 1); else digitalWrite(a12Pin, 0);
  if ( (myAddress & 8192) == 8192 ) digitalWrite(a13Pin, 1); else digitalWrite(a13Pin, 0);
  if ( (myAddress & 16384) == 16384 ) digitalWrite(a14Pin, 1); else digitalWrite(a14Pin, 0);
  if ( (myAddress & 32768) == 32768 ) digitalWrite(a15Pin, 1); else digitalWrite(a15Pin, 0);

  if ( (myBank & 1) == 1 ) digitalWrite(ba0Pin, 1); else digitalWrite(ba0Pin, 0);
  if ( (myBank & 2) == 2 ) digitalWrite(ba1Pin, 1); else digitalWrite(ba1Pin, 0);
  if ( (myBank & 4) == 4 ) digitalWrite(ba2Pin, 1); else digitalWrite(ba2Pin, 0);
  if ( (myBank & 8) == 8 ) digitalWrite(ba3Pin, 1); else digitalWrite(ba3Pin, 0);
  if ( (myBank & 16) == 16 ) digitalWrite(ba4Pin, 1); else digitalWrite(ba4Pin, 0);
  if ( (myBank & 32) == 32 ) digitalWrite(ba5Pin, 1); else digitalWrite(ba5Pin, 0);
  if ( (myBank & 64) == 64 ) digitalWrite(ba6Pin, 1); else digitalWrite(ba6Pin, 0);
  if ( (myBank & 128) == 128 ) digitalWrite(ba7Pin, 1); else digitalWrite(ba7Pin, 0);

  if ( (myData & 1) == 1 ) digitalWrite(d0Pin, 1); else digitalWrite(d0Pin, 0);
  if ( (myData & 2) == 2 ) digitalWrite(d1Pin, 1); else digitalWrite(d1Pin, 0);
  if ( (myData & 4) == 4 ) digitalWrite(d2Pin, 1); else digitalWrite(d2Pin, 0);
  if ( (myData & 8) == 8 ) digitalWrite(d3Pin, 1); else digitalWrite(d3Pin, 0);
  if ( (myData & 16) == 16 ) digitalWrite(d4Pin, 1); else digitalWrite(d4Pin, 0);
  if ( (myData & 32) == 32 ) digitalWrite(d5Pin, 1); else digitalWrite(d5Pin, 0);
  if ( (myData & 64) == 64 ) digitalWrite(d6Pin, 1); else digitalWrite(d6Pin, 0);
  if ( (myData & 128) == 128 ) digitalWrite(d7Pin, 1); else digitalWrite(d7Pin, 0);

  // Pull WE low
  digitalWrite(wrPin, LOW);

  // Pulse clock pin 4 times
  pulseClock(8);

  // Pull WE high
  digitalWrite(wrPin, HIGH);

  // Pulse clock pin 4 times
  pulseClock(8);
}

// Read one byte of data from a location specified by bank and address, 00:0000
byte readBank(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;
  return tempByte;
}

// Read a byte from the digital pins and pulse the clock, used when sending to the MX15001 chip
byte readBankClock(byte myBank, word myAddress) {
  if ( (myAddress & 1) == 1 ) digitalWrite(a0Pin, 1); else digitalWrite(a0Pin, 0);
  if ( (myAddress & 2) == 2 ) digitalWrite(a1Pin, 1); else digitalWrite(a1Pin, 0);
  if ( (myAddress & 4) == 4 ) digitalWrite(a2Pin, 1); else digitalWrite(a2Pin, 0);
  if ( (myAddress & 8) == 8 ) digitalWrite(a3Pin, 1); else digitalWrite(a3Pin, 0);
  if ( (myAddress & 16) == 16 ) digitalWrite(a4Pin, 1); else digitalWrite(a4Pin, 0);
  if ( (myAddress & 32) == 32 ) digitalWrite(a5Pin, 1); else digitalWrite(a5Pin, 0);
  if ( (myAddress & 64) == 64 ) digitalWrite(a6Pin, 1); else digitalWrite(a6Pin, 0);
  if ( (myAddress & 128) == 128 ) digitalWrite(a7Pin, 1); else digitalWrite(a7Pin, 0);
  if ( (myAddress & 256) == 256 ) digitalWrite(a8Pin, 1); else digitalWrite(a8Pin, 0);
  if ( (myAddress & 512) == 512 ) digitalWrite(a9Pin, 1); else digitalWrite(a9Pin, 0);
  if ( (myAddress & 1024) == 1024 ) digitalWrite(a10Pin, 1); else digitalWrite(a10Pin, 0);
  if ( (myAddress & 2048) == 2048 ) digitalWrite(a11Pin, 1); else digitalWrite(a11Pin, 0);
  if ( (myAddress & 4096) == 4096 ) digitalWrite(a12Pin, 1); else digitalWrite(a12Pin, 0);
  if ( (myAddress & 8192) == 8192 ) digitalWrite(a13Pin, 1); else digitalWrite(a13Pin, 0);
  if ( (myAddress & 16384) == 16384 ) digitalWrite(a14Pin, 1); else digitalWrite(a14Pin, 0);
  if ( (myAddress & 32768) == 32768 ) digitalWrite(a15Pin, 1); else digitalWrite(a15Pin, 0);

  if ( (myBank & 1) == 1 ) digitalWrite(ba0Pin, 1); else digitalWrite(ba0Pin, 0);
  if ( (myBank & 2) == 2 ) digitalWrite(ba1Pin, 1); else digitalWrite(ba1Pin, 0);
  if ( (myBank & 4) == 4 ) digitalWrite(ba2Pin, 1); else digitalWrite(ba2Pin, 0);
  if ( (myBank & 8) == 8 ) digitalWrite(ba3Pin, 1); else digitalWrite(ba3Pin, 0);
  if ( (myBank & 16) == 16 ) digitalWrite(ba4Pin, 1); else digitalWrite(ba4Pin, 0);
  if ( (myBank & 32) == 32 ) digitalWrite(ba5Pin, 1); else digitalWrite(ba5Pin, 0);
  if ( (myBank & 64) == 64 ) digitalWrite(ba6Pin, 1); else digitalWrite(ba6Pin, 0);
  if ( (myBank & 128) == 128 ) digitalWrite(ba7Pin, 1); else digitalWrite(ba7Pin, 0);

  // Pulse clock 4 times
  pulseClock(8);

  // Read
  byte tempByte = 0;
  if (digitalRead(d0Pin)) tempByte = tempByte + 1;
  if (digitalRead(d1Pin)) tempByte = tempByte + 2;
  if (digitalRead(d2Pin)) tempByte = tempByte + 4;
  if (digitalRead(d3Pin)) tempByte = tempByte + 8;
  if (digitalRead(d4Pin)) tempByte = tempByte + 16;
  if (digitalRead(d5Pin)) tempByte = tempByte + 32;
  if (digitalRead(d6Pin)) tempByte = tempByte + 64;
  if (digitalRead(d7Pin)) tempByte = tempByte + 128;

  return tempByte;
}

/******************************************
   29F1601 flashrom functions
 *****************************************/
// Reset the MX29F1601 flashrom, startbank is 0xC0 for first and 0xE0 for second flashrom
void resetFlash(int startBank) {
  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  // Reset command sequence
  if (romType) {
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0xf0);
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0xf0);
  }

  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn();
}

// Print flashrom manufacturer and device ID
void idFlash(int startBank) {
  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  if (romType) {
    // ID command sequence
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank(startBank, 0x00), readBank(startBank, 0x02));
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank(0, 0x8000), readBank(0, 0x8000 + 0x02));
  }

  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn();
}

// Write the flashroms by reading a file from the SD card, pos defines where in the file the reading/writing should start
void writeFlash(int startBank, uint32_t pos) {
  Serial.print("Writing ");
  Serial.print(flashSize);
  Serial.print(" Bytes into ");
  Serial.print(numBanks);
  Serial.print(" Banks ");
  if (romType)
    Serial.println("(HiROM).");
  else
    Serial.println("(LoRom).");

  // Open file on sd card
  if (myFile.open(fileName, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    // Configure control pins
    controlOut();
    // Set data pins to output
    dataOut();

    if (romType) {
      // Write hirom
      for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
        // Print Status
        Serial.print(".");

        // Fill SDBuffer with 4 pages
        for (unsigned long currBuffer = 0; currBuffer < 0x10000; currBuffer += 512) {
          myFile.read(SDBuffer, 512);

          // Write the four pages out of the buffer to the rom chip one page at a time
          for (unsigned long currPage = 0; currPage < 512; currPage += 128) {
            // Write command sequence
            writeBank(startBank, 0x5555 * 2, 0xaa);
            writeBank(startBank, 0x2AAA * 2, 0x55);
            writeBank(startBank, 0x5555 * 2, 0xa0);

            for (byte currByte = 0; currByte < 128; currByte++) {
              // Write one byte of data
              writeBank(currBank, currBuffer + currPage + currByte, SDBuffer[currPage + currByte]);

              if (currByte == 127) {
                // Write the last byte twice or else it won't write at all
                writeBank(currBank, currBuffer + currPage + currByte, SDBuffer[currPage + currByte]);
              }
            }
            // Wait until write is finished
            busyCheck(startBank);
          }
        }
      }
    }
    else {
      // Write lorom
      for (int currBank = 0; currBank < numBanks; currBank++) {

        // Print Status
        Serial.print(".");

        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 128) {
          myFile.read(SDBuffer, 128);
          // Write command sequence
          writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
          writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
          writeBank(1, 0x8000 + 0x1555 * 2, 0xa0);

          for (byte c = 0; c < 128; c++) {
            // Write one byte of data
            writeBank(currBank, currByte + c, SDBuffer[c]);

            if (c == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank(currBank, currByte + c, SDBuffer[c]);
            }
          }
          // Wait until write is finished
          busyCheck(startBank);
        }
      }
    }
    // Close the file:
    myFile.close();
    Serial.println("");
  }
  else {
    Serial.println("Can't open file on SD.");
  }
}

// Write 4MB to both flashroms in parallel
void writeNP() {
  unsigned long bankSize = 0x10000;
  int bufferSize = 512;
  unsigned long currOffset = 2097152;
  numBanks = 32;

  // Open file on sd card
  if (myFile.open(fileName, O_READ)) {

    // Configure control pins
    controlOut();
    // Set data pins to output
    dataOut();

    // Write 32 hirom banks at 65536B each per chip, so 4MB total
    for (int currBank = 0; currBank < numBanks; currBank++) {
      Serial.print(".");

      // Fill 2 SDBuffers with 4(if bufferSize=512) pages each, one page has 128B
      for (unsigned long currBuffer = 0; currBuffer < bankSize; currBuffer += bufferSize) {

        // Seek to current position in the file
        myFile.seekSet((currBank * bankSize) + currBuffer);
        myFile.read(SDBuffer, bufferSize);

        myFile.seekSet((currBank * bankSize) + currOffset + currBuffer);
        myFile.read(SDBuffer2, bufferSize);

        // Write 2x 128B at once out of the two SDBuffers to the flashroms until all the buffer's content was written
        for (unsigned long currPage = 0; currPage < bufferSize; currPage += 128) {

          // Write command sequence to 1st flashrom
          writeBank(0xC0, 0x5555 * 2, 0xaa);
          writeBank(0xC0, 0x2AAA * 2, 0x55);
          writeBank(0xC0, 0x5555 * 2, 0xa0);

          for (byte currByte = 0; currByte < 128; currByte++) {

            // Write one byte of data
            writeBank(0xC0 + currBank, currBuffer + currPage + currByte, SDBuffer[currPage + currByte]);

            if (currByte == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank(0xC0 + currBank, currBuffer + currPage + currByte, SDBuffer[currPage + currByte]);
            }
          }

          // Write command sequence to 2nd flashrom
          writeBank(0xE0, 0x5555 * 2, 0xaa);
          writeBank(0xE0, 0x2AAA * 2, 0x55);
          writeBank(0xE0, 0x5555 * 2, 0xa0);

          for (byte currByte = 0; currByte < 128; currByte++) {

            // Write one byte of data
            writeBank(0xE0 + currBank, currBuffer + currPage + currByte, SDBuffer2[currPage + currByte]);

            if (currByte == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank(0xE0 + currBank, currBuffer + currPage + currByte, SDBuffer2[currPage + currByte]);
            }
          }
          // Wait until writes are finished
          busyCheck(0xC0);
          busyCheck(0xE0);
        }
      }
    }
    // Close the file:
    myFile.close();
  }
  else {
    Serial.println("Can't open file on SD.");
  }
}

// Delay between write operations based on status register
void busyCheck(byte startBank) {
  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn();

  // Read register
  readBank(startBank, 0x0000);

  // Read D7 while D7 = 0
  //1 = B00000001, 1 << 7 = B10000000, PINC = B1XXXXXXX (X = don't care), & = bitwise and
  while (!(PINC & (1 << 7))) {
    // CE or OE must be toggled with each subsequent status read or the
    // completion of a program or erase operation will not be evident.
    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");
  }

  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();
}

// Erase the flashrom to 0xFF
void eraseFlash(int startBank) {
  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  if (romType) {
    // Erase command sequence
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x80);
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x10);
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x80);
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x10);
  }

  // Wait for erase to complete
  busyCheck(startBank);
}

// Check if an erase succeeded, return 1 if blank and 0 if not
byte blankcheck(int startBank) {
  // Set data pins to input again
  dataIn();
  // Set control pins to input
  controlIn();

  byte blank = 1;
  if (romType) {
    for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte++) {
        if (readBank(currBank, currByte) != 0xFF) {
          currBank =  startBank + numBanks;
          blank = 0;
        }
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte++) {
        if (readBank(currBank, currByte) != 0xFF) {
          currBank = numBanks;
          blank = 0;
        }
      }
    }
  }
  return blank;
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyFlash(int startBank, uint32_t pos) {
  unsigned long  verified = 0;

  // Open file on sd card
  if (myFile.open(fileName, O_READ)) {
    // Set file starting position
    myFile.seekCur(pos);

    // Set data pins to input
    dataIn();
    // Set control pins to input
    controlIn();

    if (romType) {
      for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
        for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(SDBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank(currBank, currByte + c) != SDBuffer[c]) {
              verified++;
            }
          }
        }
      }
    }
    else {
      for (int currBank = 0; currBank < numBanks; currBank++) {
        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(SDBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank(currBank, currByte + c) != SDBuffer[c]) {
              verified++;
            }
          }
        }
      }
    }
    // Close the file:
    myFile.close();
  }
  else {
    // SD Error
    verified = 999999;
    Serial.println("Can't open file on SD.");
  }
  // Return 0 if verified ok, or number of errors
  return verified;
}

// Read flashroms and save them to the SD card
void readFlash() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn();

  Serial.print("Reading flash into file ");
  Serial.println(fileName);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    Serial.println("Can't create file on SD.");
    while (1);
  }
  if (romType) {
    for (int currBank = 0xC0; currBank < 0xC0 + numBanks; currBank++) {

      // Print Status
      Serial.print(".");

      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          SDBuffer[c] = readBank(currBank, currByte + c);
        }
        myFile.write(SDBuffer, 512);
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {

      // Print Status
      Serial.print(".");

      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          SDBuffer[c] = readBank(currBank, currByte + c);
        }
        myFile.write(SDBuffer, 512);
      }
    }
  }
  // Close the file:
  myFile.close();
  Serial.println("");
}

// Print part of the flash to the serial monitor
void printFlash(boolean hirom, unsigned long startByte, unsigned long startBank, int numBytes) {

  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn();

  char buffer[3];
  if (hirom) {
    for (unsigned long currByte = 0; currByte < numBytes; currByte += 16) {
      for (int c = 0; c < 16; c++) {
        itoa (readBank(startBank, currByte + c), buffer, 16);
        for (int i = 0; i < 2 - strlen(buffer); i++) {
          Serial.print('0');
        }
        // Now print the significant bits
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }
  else {
    for (unsigned long currByte = 0x8000; currByte < 0x8000 + numBytes; currByte += 16) {
      for (int c = 0; c < 16; c++) {
        itoa (readBank(startBank, currByte + c), buffer, 16);
        for (int i = 0; i < 2 - strlen(buffer); i++) {
          Serial.print('0');
        }
        // Now print the significant bits
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }
}

// Print multiple 64 bytes sections out of different parts of the flashroms
void printFlashAll() {
  // Romtype, startByte, numBytes
  Serial.println("0x00/0xC0");
  printFlash(romType, 0, 0xC0, 64);
  Serial.println("0x80000/0xC8");
  printFlash(romType, 0x80000, 0xC8, 64);
  Serial.println("0x100000/0xD0");
  printFlash(romType, 0x100000, 0xD0, 64);
  Serial.println("0x180000/0xD8");
  printFlash(romType, 0x180000, 0xD8, 64);
  Serial.println("0x200000/0xE0");
  printFlash(romType, 0x200000, 0xE0, 64);
  Serial.println("0x280000/0xE8");
  printFlash(romType, 0x280000, 0xE8, 64);
  Serial.println("0x300000/0xF0");
  printFlash(romType, 0x300000, 0xF0, 64);
  Serial.println("0x380000/0xF8");
  printFlash(romType, 0x380000, 0xF8, 64);
}

// Display protected sectors/banks as 0xc2 and unprotected as 0x00
byte readSectorProtection(byte startBank) {
  byte isProtected = 0;

  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  // Display Sector Protection Status
  if (romType) {
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x90);

    // Configure control pins
    controlIn();
    // Set data pins to output
    dataIn();

    for (int i = 0; i <= 0x1F; i++) {
      if (readBank(startBank + i, 0x04) == 0xC2) {
        Serial.print("Sector 0x");
        Serial.print(startBank + i, HEX);
        Serial.println(" is protected");
        isProtected = 1;
      }
      else if (readBank(startBank + i, 0x04) == 0x0) {
        /*Serial.print("Sector 0x");
          Serial.print(startBank + i, HEX);
          Serial.println(" is not protected");*/
      }
      else {
        /*Serial.print("Sector 0x");
          Serial.print(startBank + i, HEX);
          Serial.println(" READ ERROR");*/
        isProtected = 2;
      }
    }
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x90);

    // Configure control pins
    controlIn();
    // Set data pins to output
    dataIn();

    for (int i = 0; i <= 0x1F; i++) {
      if (readBank(0, 0x8000 + 0x04) == 0xC2) {
        Serial.print("Sector 0x");
        Serial.print(startBank + i, HEX);
        Serial.println(" is protected");
        isProtected = 1;
      }
      else if (readBank(0, 0x8000 + 0x04) == 0x0) {
      }
      else {
        isProtected = 2;
      }
    }
  }
  return isProtected;
}

void unlockSectorProtection(byte startBank) {
  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  // Remove Sector Protection
  if (romType) {
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x60);
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x0000 * 2, 0x40);
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x60);
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(0, 0x8000 + 0x0000 * 2, 0x40);
  }
  // Wait until write is finished
  busyCheck(startBank);
}

void setSectorProtection(byte startBank) {
  // Configure control pins
  controlOut();
  // Set data pins to output
  dataOut();

  // Set Sector Protection
  if (romType) {
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x60);
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x0000 * 2, 0x20);
  }
  else {
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x60);
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(0, 0x8000 + 0x0000 * 2, 0x20);
  }
  // Wait until write is finished
  busyCheck(startBank);
}

/******************************************
   Nintendo Power
 *****************************************/
// Switch to HiRom All and unlock Write Protection
boolean unlockedHirom() {
  romType = 1;

  if (sendNP(0x04) == 0x2A) {
    // Unlock Write Protection
    sendNP(0x02);
    if (readBankClock(0, 0x2401) == 0x4) {
      return 1;
    }
    else {
      Serial.println("Error: Unlocking WP failed.");
      Serial.println("Please power-cycle NP cart.");
      readPorts();
      return 0;
    }
  }
  else {
    Serial.println("Error: Switching to HiRom All failed.");
    Serial.println("Please power-cycle NP cart.");
    readPorts();
    return 0;
  }
}

// Read the ports at 0x2400-0x2407 and print the results
void readPorts() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn();

  for (unsigned long myAddr = 0x2400; myAddr < 0x2408; myAddr++) {
    Serial.print(readBankClock(0, myAddr), HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}

void eraseMapping(byte startBank) {
  // Switch to write
  dataOut();
  controlOut();

  if (romType) {
    // Prepare to erase/write Page Buffer
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0x77);
    // Erase Page Buffer
    writeBank(startBank, 0x5555 * 2, 0xaa);
    writeBank(startBank, 0x2AAA * 2, 0x55);
    writeBank(startBank, 0x5555 * 2, 0xe0);
  }
  else {
    // Prepare to erase/write Page Buffer
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0x77);

    // Erase Page Buffer
    writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
    writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
    writeBank(1, 0x8000 + 0x1555 * 2, 0xe0);
  }

  // Wait until erase is finished
  busyCheck(startBank);

  // Switch to read
  dataIn();
  controlIn();
}

// Read the current mapping from the hidden "page buffer"
void readMapping() {

  Serial.print("Reading mapping into file ");
  Serial.println(fileName);

  // Switch to write
  dataOut();
  controlOut();

  // Reset to defaults
  writeBank(0xC0, 0x0000, 0x38);
  writeBank(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn();

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    Serial.println("SD Error");
  }

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank(0xC0, currByte));
  }

  // Switch to write
  dataOut();
  controlOut();

  // Reset to defaults
  writeBank(0xE0, 0x0000, 0x38);
  writeBank(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank(0xE0, currByte));
  }

  // Close the file:
  myFile.close();

  // Switch to write
  dataOut();
  controlOut();

  // Reset Flash
  writeBank(0xC0, 0x5555 * 2, 0xaa);
  writeBank(0xC0, 0x2AAA * 2, 0x55);
  writeBank(0xC0, 0x5555 * 2, 0xf0);

  // Reset Flash
  writeBank(0xE0, 0x5555 * 2, 0xaa);
  writeBank(0xE0, 0x2AAA * 2, 0x55);
  writeBank(0xE0, 0x5555 * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn();

  // Signal end of process
  Serial.println("Done.");
}

void writeMapping(byte startBank, uint32_t pos) {
  // Switch to write
  dataOut();
  controlOut();

  // Open file on sd card
  if (myFile.open(fileName, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);


    if (romType) {
      // Write to Page Buffer
      for (unsigned long currByte = 0x00; currByte < 0x100; currByte += 128) {
        // Prepare to erase/write Page Buffer
        writeBank(startBank, 0x5555 * 2, 0xaa);
        writeBank(startBank, 0x2AAA * 2, 0x55);
        writeBank(startBank, 0x5555 * 2, 0x77);
        // Write Page Buffer Command
        writeBank(startBank, 0x5555 * 2, 0xaa);
        writeBank(startBank, 0x2AAA * 2, 0x55);
        writeBank(startBank, 0x5555 * 2, 0x99);

        myFile.read(SDBuffer, 128);

        for (byte c = 0; c < 128; c++) {
          writeBank(startBank, currByte + c, SDBuffer[c]);
          // Write last byte twice
          if (c == 127) {
            writeBank(startBank, currByte + c, SDBuffer[c]);
          }
        }
        busyCheck(startBank);
      }
    }
    else {
      // Write to Page Buffer
      for (unsigned long currByte = 0x0000; currByte < 0x100; currByte += 128) {
        // Prepare to erase/write Page Buffer
        writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
        writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
        writeBank(1, 0x8000 + 0x1555 * 2, 0x77);
        // Write Page Buffer Command
        writeBank(1, 0x8000 + 0x1555 * 2, 0xaa);
        writeBank(0, 0x8000 + 0x2AAA * 2, 0x55);
        writeBank(1, 0x8000 + 0x1555 * 2, 0x99);

        myFile.read(SDBuffer, 128);

        for (byte c = 0; c < 128; c++) {
          writeBank(1, 0x8000 + currByte + c, SDBuffer[c]);
          // Write last byte twice
          if (c == 127) {
            writeBank(1, 0x8000 + currByte + c, SDBuffer[c]);
          }
        }
        busyCheck(startBank);
      }
    }
    // Close the file:
    myFile.close();
  }
  else {
    Serial.println("Can't open file on SD");
  }

  // Switch to read
  dataIn();
  controlIn();
}

// Print the current mapping from the hidden "page buffer"
void printMapping(byte startBank) {
  // Switch to write
  dataOut();
  controlOut();

  if (romType) {
    // Unlock read map
    writeBank(startBank, 0x0000, 0x38);
    writeBank(startBank, 0x0000, 0xd0);
    // Read Extended Status Register (GSR and PSR)
    writeBank(startBank, 0x0000, 0x71);
    // Page Buffer Swap
    writeBank(startBank, 0x0000, 0x72);
    // Read Page Buffer
    writeBank(startBank, 0x0000, 0x75);

    // Switch to read
    dataIn();
    controlIn();

    char buffer[3];
    for (unsigned long currByte = 0xFF00; currByte < 0xFFFF; currByte += 16) {
      Serial.print("0x");
      Serial.print(startBank, HEX);
      Serial.print(currByte, HEX);
      Serial.print(" ");
      for (int c = 0; c < 16; c++) {
        itoa (readBank(startBank, currByte + c), buffer, 16);
        for (int i = 0; i < 2 - strlen(buffer); i++) {
          Serial.print('0');
        }
        // Now print the significant bits
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println("");
    }

  }
  else {
    writeBank(0, 0x8000 + 0x0000, 0x38);
    writeBank(0, 0x8000 + 0x0000, 0xd0);
    // Read Extended Status Register (GSR and PSR)
    writeBank(0, 0x8000 + 0x0000, 0x71);
    // Page Buffer Swap
    writeBank(0, 0x8000 + 0x0000, 0x72);
    // Read Page Buffer
    writeBank(0, 0x8000 + 0x0000, 0x75);

    // Switch to read
    dataIn();
    controlIn();

    char buffer[3];
    for (unsigned long currByte = 0x0000; currByte < 0x100; currByte += 16) {
      Serial.print("0x");
      Serial.print(currByte, HEX);
      Serial.print(" ");
      for (int c = 0; c < 16; c++) {
        itoa (readBank(0, 0x8000 + currByte + c), buffer, 16);
        for (int i = 0; i < 2 - strlen(buffer); i++) {
          Serial.print('0');
        }
        // Now print the significant bits
        Serial.print(buffer);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }

  // Switch to read
  dataIn();
  controlIn();
}

// Send a command to the MX15001 chip
byte sendNP(byte command) {
  // Switch to write
  dataOut();
  controlOut();

  // Write command then pulse clock pin 2 times
  writeBankClock(0, 0x2400, 0x09);
  pulseClock(4);

  // Switch to read
  dataIn();
  controlIn();

  // Read status
  NPReady = readBankClock(0, 0x2400);

  // Switch to write
  dataOut();
  controlOut();

  writeBankClock(0, 0x2401, 0x28);
  pulseClock(4);
  writeBankClock(0, 0x2401, 0x84);
  pulseClock(4);

  // NP_CMD_06h, send this only if above read has returned 7Dh, not if it's already returning 2Ah
  if (NPReady == 0x7D) {
    writeBankClock(0, 0x2400, 0x06);
    pulseClock(4);
    writeBankClock(0, 0x2400, 0x39);
    pulseClock(4);
  }

  // Write the command
  writeBankClock(0, 0x2400, command);

  // Switch to read
  dataIn();
  controlIn();

  // Read status
  NPReady = readBankClock(0, 0x2400);
  return NPReady;
}

// This function will erase and program one of the flashroms(0xC0 or 0xE0) from a file off the SD card
void programFlash(int startBank, uint32_t pos) {
  // Switch NP cart's mapping
  if (unlockedHirom()) {

    // Get ID
    idFlash(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      Serial.print("Flash ID: ");
      Serial.println(flashid);
      resetFlash(startBank);

      // Erase flash
      Serial.print("Checking if flash is empty...");
      if (blankcheck(startBank))
        Serial.println("Success.");
      else {
        Serial.println("Flash is not blank.");
        Serial.print("Erasing...");
        eraseFlash(startBank);
        resetFlash(startBank);
        Serial.println("Done.");
        Serial.print("Checking if flash is empty...");
        if (blankcheck(startBank))
          Serial.println("Success.");
        else {
          Serial.println("Abort: Could not erase flash.");
          while (1);
        }
      }
      // Write flash
      writeFlash(startBank, pos);

      // Reset flash
      resetFlash(startBank);

      // Checking for errors
      Serial.print("Checking for write errors...");
      writeErrors = verifyFlash(startBank, pos);
      if (writeErrors == 0) {
        Serial.println("Flashrom verified ok");
      }
      else {
        Serial.print("Number of Errors: ");
        Serial.println(writeErrors);
      }
    }
    else {
      Serial.println("Error: Wrong Flash ID");
    }
  }
}

// This function will erase and program the NP cart(both flashroms) from a 4MB file off the SD card
void programNP() {
  // Switch NP cart's mapping
  if (unlockedHirom()) {

    // Get ID
    for (int currFlash = 0; currFlash < 33; currFlash += 32) {
      idFlash(0xC0 + currFlash);
      if (strcmp(flashid, "c2f3") == 0) {
        resetFlash(0xC0 + currFlash);

        // Erase flash
        Serial.print("Checking if flash 0x");
        Serial.print(0xC0 + currFlash, HEX);
        Serial.print(" is empty...");
        if (blankcheck(0xC0 + currFlash))
          Serial.println("Success.");
        else {
          Serial.println("Flash is not blank.");
          Serial.print("Erasing...");
          eraseFlash(0xC0 + currFlash);
          resetFlash(0xC0 + currFlash);
          if (blankcheck(0xC0 + currFlash))
            Serial.println("Done.");
          else {
            Serial.println("Abort: Could not erase flash.");
            while (1);
          }
        }
      }
      else {
        Serial.println("Error: Wrong Flash ID");
        while (1);
      }
    }
    // Write flash
    Serial.print("Writing...");
    writeNP();
    Serial.println("");

    // Reset flash
    resetFlash(0xC0);
    resetFlash(0xE0);

    // Checking for errors
    Serial.println("Checking for write errors...");
    writeErrors = verifyFlash(0xC0, 0);
    if (writeErrors == 0) {
      Serial.println("Flash 0xC0 verified OK.");
    }
    else {
      Serial.print("Number of Errors: ");
      Serial.println(writeErrors);
    }
    // Checking for errors
    writeErrors = verifyFlash(0xE0, 2097152);
    if (writeErrors == 0) {
      Serial.println("Flash 0xE0 verified OK.");
    }
    else {
      Serial.print("Number of Errors: ");
      Serial.println(writeErrors);
    }
  }
}

/******************************************
  SNES SRAM Functions
*****************************************/
// Dump the SRAM to the SD card
void writeSRAM () {
  //open file on sd card
  if (myFile.open(fileName, O_READ)) {

    // Set pins to output
    dataOut();
    // Set RST RD WR to High and CS to Low
    controlOut();

    // Dump HiRom
    if (romType) {
      // Writing SRAM on HiRom needs CS to be high
      digitalWrite(csPin, HIGH);
      // Sram size
      long lastByte = (long(sramSize) * 128) + 24576;
      for (long currByte = 24576; currByte < lastByte; currByte++) { //startAddr = 0x6000
        writeBank(48, currByte, myFile.read()); //startBank = 0x30
      }
    }
    // Dump LoRom
    else {
      // Sram size
      long lastByte = (long(sramSize) * 128);
      for (long currByte = 0; currByte <  lastByte; currByte++) { //startAddr = 0x0000
        writeBank(112, currByte, myFile.read()); //startBank = 0x70
      }
    }

    // set control
    controlIn();
    // Set pins to input
    dataIn();

    // Close the file:
    myFile.close();
    Serial.println("SRAM writing finished");
  }
  else {
    Serial.println("File doesnt exist");
  }
}

void readSRAM () {
  // set control
  controlIn();
  // Set pins to input
  dataIn();

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    Serial.println("SD Error");
  }
  if (romType) {
    // Dumping SRAM on HiRom needs CS to be high
    digitalWrite(csPin, HIGH);
    // Sram size
    long lastByte = (long(sramSize) * 128) + 24576;
    for (long currByte = 24576; currByte < lastByte; currByte++) { //startAddr = 0x6000
      myFile.write(readBank(48, currByte)); //startBank = 0x30
    }
  }
  else {
    // Sram size
    long lastByte = (long(sramSize) * 128);
    for (long currByte = 0; currByte < lastByte; currByte++) { //startAddr = 0x0000
      myFile.write(readBank(112, currByte)); //startBank = 0x70
    }
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  Serial.print("Saved to ");
  Serial.println(fileName);
}

// Check if the SRAM was written without any error
unsigned long verifySRAM() {
  //open file on sd card
  if (myFile.open(fileName, O_READ)) {

    // Variable for errors
    writeErrors = 0;

    // set control
    controlIn();
    // Set pins to input
    dataIn();

    if (romType) {
      // Dumping SRAM on HiRom needs CS to be high
      digitalWrite(csPin, HIGH);
      // Sram size
      long lastByte = (long(sramSize) * 128) + 24576;
      for (long currByte = 24576; currByte < lastByte; currByte += 512) {
        //fill sdBuffer
        myFile.read(SDBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          if ((readBank(48, currByte + c)) != SDBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }
    else {
      // Sram size
      long lastByte = (long(sramSize) * 128);
      for (long currByte = 0; currByte < lastByte; currByte += 512) {
        //fill sdBuffer
        myFile.read(SDBuffer, 512);
        for (unsigned long c = 0; c < 512; c++) {
          if ((readBank(112, currByte + c)) != SDBuffer[c]) {
            writeErrors++;
          }
        }
      }
    }

    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    Serial.println("Can't open file");
  }
}

/******************************************
  Main loop
*****************************************/
void loop() {
  // Print menu to serial monitor
  Serial.println("MENU----------------------------------------------------->");
  Serial.println("Change Mode: ");
  Serial.println("(0)Menu  (2)GAME2 (4)GAME4 (6)GAME6 (8)HIROM ALL");
  Serial.println("(1)GAME1 (3)GAME3 (5)GAME5 (7)GAME7 (9)HIROM MENU");
  Serial.println("");
  Serial.println("Flash Commands: ");
  Serial.println("(A)Read ID (B)Reset (C)Print Flash");
  Serial.println("");
  Serial.println("Read / Write Rom: ");
  Serial.println("(D)Read Rom (E)Erase Flash (F)Write Rom");
  Serial.println("");
  Serial.println("Read / Write Mapping: ");
  Serial.println("(G)Print mapping (H)Save mapping");
  Serial.println("(I)Erase mapping (J)Write mapping");
  Serial.println("");
  Serial.println("Sector Protection: ");
  Serial.println("(K)Read Sector Protection");
  Serial.println("(L)Unlock Sector Protection");
  Serial.println("(M)Set Sector Protection");
  Serial.println("");
  Serial.println("Read / Write SRAM: ");
  Serial.println("(N)Read SRAM");
  Serial.println("(O)Write SRAM");
  Serial.println("");
  Serial.println("Misc: ");
  Serial.println("(P)Print Ports");
  Serial.println("(Q)Unlock WP");

  Serial.println("");

  // Wait for user input
  Serial.println("-------> Please enter a single digit or letter: _  <-------");
  Serial.println("");

  while (Serial.available() == 0) {
  }

  // Read the incoming byte:
  incomingByte = Serial.read();

  // Execute user choice
  switch (incomingByte) {
    case 48:
      Serial.println("(0)Menu 0x80");
      // Reset to Menu
      romType = 0;
      sendNP(0x80);
      readPorts();
      break;

    case 49:
      Serial.println("(1)GAME1 0x81");
      // Reset to GAME1
      romType = 0;
      sendNP(0x81);
      readPorts();
      break;

    case 50:
      Serial.println("(2)GAME2 0x82");
      // Reset to GAME2
      romType = 0;
      sendNP(0x82);
      readPorts();
      break;

    case 51:
      Serial.println("(3)GAME3 0x83");
      // Reset to GAME3
      romType = 0;
      sendNP(0x83);
      readPorts();
      break;

    case 52:
      Serial.println("(4)GAME4 0x84");
      // Reset to GAME4
      romType = 0;
      sendNP(0x84);
      readPorts();
      break;

    case 53:
      Serial.println("(5)GAME5 0x85");
      // Reset to GAME5
      romType = 0;
      sendNP(0x85);
      readPorts();
      break;

    case 54:
      Serial.println("(6)GAME6 0x86");
      // Reset to GAME6
      romType = 0;
      sendNP(0x86);
      readPorts();
      break;

    case 55:
      Serial.println("(7)GAME7 0x87");
      // Reset to GAME7
      romType = 0;
      sendNP(0x87);
      readPorts();
      break;

    case 56:
      Serial.println("(8)HIROM ALL 0x04");
      // Reset to HIROM ALL
      romType = 1;
      if (sendNP(0x04) == 0x2A)
        Serial.println("Success");
      else
        Serial.println("Failed");
      readPorts();
      break;

    case 57:
      Serial.println("(9)HIROM MENU 0x05");
      // Reset to HIROM MENU
      romType = 1;
      sendNP(0x05);
      readPorts();
      break;

    case 97:
      Serial.println("(A)Read Flash ID");
      // Read Flash ID
      idFlash(0xC0);
      Serial.print("Flash ID 1: ");
      Serial.println(flashid);
      resetFlash(0xC0);
      idFlash(0xE0);
      Serial.print("Flash ID 2: ");
      Serial.println(flashid);
      resetFlash(0xE0);
      break;

    case 98:
      Serial.println("(B)Reset flash");
      // Reset flash #1
      resetFlash(0xC0);
      // Reset flash #2
      resetFlash(0xE0);
      // Reset flash #2
      resetFlash(0xE0);
      break;

    case 99:
      Serial.println("(C)Print Flash");
      // Print flash
      Serial.println("Flash: ");
      printFlashAll();
      Serial.println(" ");
      break;

    case 100:
      Serial.println("(D)Dump NP");
      resetFlash(0xC0);
      resetFlash(0xE0);
      romType = 1;
      flashSize = 4194304;
      numBanks = 64;
      Serial.print("Switching Mapping to HiRom All...");
      if (sendNP(0x04) == 0x2A) {
        Serial.println("Success.");
        // Read flash
        getfilename();
        readFlash();
      }
      else {
        Serial.println("Error: Switching to HiRom All failed.");
        Serial.println("Please power - cycle NP cart.");
        readPorts();
      }
      break;

    case 101:
      Serial.println("(E)Erase NP");
      flashSize = 2097152;
      numBanks = 32;
      if (unlockedHirom()) {
        Serial.println("Erasing flashrom #1");
        // Erase Flash #1
        eraseFlash(0xC0);
        resetFlash(0xC0);
        Serial.print("Blankcheck...");
        if (blankcheck(0xC0))
          Serial.println("Success.");
        else
          Serial.println("failed.");
        // Erase Flash #2
        Serial.println("Erasing flashrom #2");
        eraseFlash(0xE0);
        resetFlash(0xE0);
        Serial.print("Blankcheck...");
        if (blankcheck(0xE0))
          Serial.println("Success.");
        else
          Serial.println("failed.");
      }
      break;

    case 102:
      Serial.println("(F)Write NP");
      flashSize = 2097152;
      numBanks = 32;
      // Prompt user input
      getfilename();

      //Serial Programming
      Serial.println("Programming 1st flashrom.");
      // Program 1st flashrom
      programFlash(0xC0, 0);
      Serial.println("");
      Serial.println("Programming 2nd flashrom.");
      // Program 2nd flashrom
      programFlash(0xE0, 2097152);

      //Parallel Programming
      //programNP();
      break;

    case 103:
      Serial.println("(G)Print mapping");
      if (unlockedHirom()) {
        printMapping(0xC0);
        printMapping(0xE0);
      }
      break;

    case 104:
      Serial.println("(H)Save mapping");
      if (unlockedHirom()) {
        getfilename();
        readMapping();
      }
      break;

    case 105:
      Serial.println("(I)Erase mapping");
      if (unlockedHirom()) {
        eraseMapping(0xD0); // 0xC0 not working
        eraseMapping(0xE0);
        printMapping(0xC0);
        printMapping(0xE0);
      }
      break;

    case 106:
      Serial.println("(J)Write mapping");
      if (unlockedHirom()) {
        getfilename();
        writeMapping(0xD0, 0); // 0xC0 not working
        writeMapping(0xE0, 256);
        printMapping(0xC0);
        printMapping(0xE0);
      }
      break;

    case 107:
      Serial.println("(K)Read Sector Protection");
      if (unlockedHirom()) {
        byte protectStatus;
        protectStatus = readSectorProtection(0xC0);
        if (protectStatus == 2)
          Serial.println("Can't access Flash C0");
        else if (protectStatus == 0)
          Serial.println("Flash C0 unprotected");
        protectStatus = readSectorProtection(0xE0);
        if (protectStatus == 2)
          Serial.println("Can't access Flash E0");
        else if (protectStatus == 0)
          Serial.println("Flash E0 unprotected");
      }
      break;

    case 108:
      Serial.println("(L)Unlock Sector Protection");
      if (unlockedHirom()) {
        unlockSectorProtection(0xC1);
        unlockSectorProtection(0xE0);
      }
      break;

    case 109:
      Serial.println("(M)Set Sector Protection");
      if (unlockedHirom()) {
        setSectorProtection(0xC1);
        setSectorProtection(0xE0);
      }
      break;

    case 110:
      Serial.println("(N)Read SRAM");
      if (sendNP(0x04) == 0x2A) {
        romType = 1;
        getfilename();
        readSRAM();
      }
      else
        Serial.println("Switching to Hirom failed.");
      break;

    case 111:
      Serial.println("(O)Write SRAM");
      if (sendNP(0x04) == 0x2A) {
        romType = 1;
        getfilename();
        writeSRAM();
        Serial.print("Verifying...");
        if (verifySRAM() == 0)
          Serial.println("OK.");
        else
          Serial.println("failed.");
      }
      else
        Serial.println("Switching to Hirom failed.");
      break;

    case 112:
      Serial.println("(P)Print Ports");
      readPorts();
      break;

    case 113:
      Serial.println("(Q)Unlock WP");
      sendNP(0x02);
      if (readBankClock(0, 0x2401) == 0x4) {
        Serial.println("Success.");
        readPorts();
      }
      else {
        Serial.println("Error: Unlocking WP failed.");
        Serial.println("Please power - cycle NP cart.");
        readPorts();
      }
      break;
  }
  Serial.println("");
}

//******************************************
// End of File
//******************************************
