//******************************************
// CASIO LOOPY MODULE
//******************************************
// IMPORTANT: All data are stored as BIG-ENDIAN. Many ROM dumps online are little endian.
// See https://github.com/kasamikona/Loopy-Tools/blob/master/ROM%20Structure.md
//
// By @partlyhuman
// Special thanks to @kasamikona
#ifdef enable_LOOPY

// SH-1 memory map locations, ROM starts here
const uint32_t LOOPY_MAP_ROM_ZERO = 0x0E000000;
const uint32_t LOOPY_MAP_SRAM_ZERO = 0x02000000;
const uint32_t LOOPY_SRAM_SIZE = 0x2000;

// Control pins
const int LOOPY_ROMCE = 42;
const int LOOPY_OE = 43;
const int LOOPY_RAMWE = 6;
const int LOOPY_RAMCS1 = 7;
const int LOOPY_RAMCS2 = A7;

// The internal checksum read from the cart header at 08h, will be checked against an actual sum
uint32_t loopyChecksum;
uint32_t loopyChecksumStart;
uint32_t loopyChecksumEnd;
// Whether the cart was found (by checksum) in the database
bool loopyCartRecognized;

//******************************************
// SETUP
//******************************************

void setup_LOOPY() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // PK1-PK7, PA1-PA7, PC0-PC3, PL0-PL3 // Take whole port and unset the exceptions later
  DDRK = DDRA = DDRC = DDRL = 0xFF;

  // Control pins, all active low
  pinMode(LOOPY_ROMCE, OUTPUT);
  pinMode(LOOPY_OE, OUTPUT);
  pinMode(LOOPY_RAMWE, OUTPUT);
  pinMode(LOOPY_RAMCS1, OUTPUT);
  pinMode(LOOPY_RAMCS2, OUTPUT);
  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_OE, HIGH);
  digitalWrite(LOOPY_RAMWE, HIGH);
  digitalWrite(LOOPY_RAMCS1, HIGH);
  digitalWrite(LOOPY_RAMCS2, HIGH);

  // Set Pins (D0-D15) to Input
  dataIn_LOOPY();

  strcpy(romName, "unknown");
  getCartInfo_LOOPY();

  mode = mode_LOOPY;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char loopyMenuItem0[] PROGMEM = "Refresh Cart";
static const char loopyMenuItem1[] PROGMEM = "Read ROM";
static const char loopyMenuItem2[] PROGMEM = "Read SRAM";
static const char loopyMenuItem3[] PROGMEM = "Write SRAM";
//static const char loopyMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsLOOPY[] PROGMEM = { loopyMenuItem0, loopyMenuItem1, loopyMenuItem2, loopyMenuItem3, string_reset2 };

void loopyMenu() {
  convertPgm(menuOptionsLOOPY, 4);
  uint8_t mainMenu = question_box(F("CASIO LOOPY MENU"), menuOptions, 4, 0);
  display_Clear();
  display_Update();

  switch (mainMenu) {
    case 0:
      setup_LOOPY();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_LOOPY();
      sd.chdir("/");
      break;

    case 2:
      // Read SRAM
      sd.chdir("/");
      println_Msg(F("Reading SRAM..."));
      display_Update();
      readSRAM_LOOPY();
      sd.chdir("/");
#if (defined(enable_OLED) || defined(enable_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 3:
      // Write SRAM
      // Change working dir to root
      sd.chdir("/");
      fileBrowser(F("Select SAV file"));
      display_Clear();
      writeSRAM_LOOPY();
      writeErrors = verifySRAM_LOOPY();
      if (writeErrors == 0) {
        println_Msg(F("SRAM verified OK"));
        display_Update();
      } else {
        print_STR(error_STR, 0);
        print_Msg(writeErrors);
        print_STR(_bytes_STR, 1);
        print_Error(did_not_verify_STR);
      }

#if (defined(enable_OLED) || defined(enable_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 4:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  LOW LEVEL FUNCTIONS
//******************************************

void setAddress_LOOPY(unsigned long A) {
  // PK1  A0
  // PK2  A1
  // PK3  A21
  // PK4  A3
  // PK5  A20
  // PK6  A15
  // PK7  A13
  PORTK = (bitRead(A, 0) << 1)
          | (bitRead(A, 1) << 2)
          | (bitRead(A, 21) << 3)
          | (bitRead(A, 3) << 4)
          | (bitRead(A, 20) << 5)
          | (bitRead(A, 15) << 6)
          | (bitRead(A, 13) << 7);
  // PA1  A2
  // PA2  A4
  // PA3  A19
  // PA4  A18
  // PA5  A16
  // PA6  A17
  // PA7  A14
  PORTA = (bitRead(A, 2) << 1)
          | (bitRead(A, 4) << 2)
          | (bitRead(A, 19) << 3)
          | (bitRead(A, 18) << 4)
          | (bitRead(A, 16) << 5)
          | (bitRead(A, 17) << 6)
          | (bitRead(A, 14) << 7);
  // PC0  A6
  // PC1  A8
  // PC2  A10
  // PC3  A12
  PORTC = (bitRead(A, 6))
          | (bitRead(A, 8) << 1)
          | (bitRead(A, 10) << 2)
          | (bitRead(A, 12) << 3);
  // PL0  A5
  // PL1  A7
  // PL2  A9
  // PL3  A11
  PORTL = (bitRead(A, 5))
          | (bitRead(A, 7) << 1)
          | (bitRead(A, 9) << 2)
          | (bitRead(A, 11) << 3);
}

uint16_t getWord_LOOPY() {
  // A8   PK0 D0
  // D22  PA0 D1
  // A6   PF6 D2
  // A5   PF5 D3
  // A3   PF3 D4
  // D40  PG1 D5
  // A2   PF2 D6
  // D41  PG0 D7
  // A1   PF1 D8
  // D3   PE5 D9
  // A0   PF0 D10
  // D2   PE4 D11
  // D14  PJ1 D12
  // D15  PJ0 D13
  // A4   PF4 D14
  // D4   PG5 D15

  return bitRead(PINK, 0)
         | (bitRead(PINA, 0) << 1)
         | (bitRead(PINF, 6) << 2)
         | (bitRead(PINF, 5) << 3)
         | (bitRead(PINF, 3) << 4)
         | (bitRead(PING, 1) << 5)
         | (bitRead(PINF, 2) << 6)
         | (bitRead(PING, 0) << 7)
         | (bitRead(PINF, 1) << 8)
         | (bitRead(PINE, 5) << 9)
         | (bitRead(PINF, 0) << 10)
         | (bitRead(PINE, 4) << 11)
         | (bitRead(PINJ, 1) << 12)
         | (bitRead(PINJ, 0) << 13)
         | (bitRead(PINF, 4) << 14)
         | (bitRead(PING, 5) << 15);

  // return digitalRead(A8)
  //        | (digitalRead(22) << 1)
  //        | (digitalRead(A6) << 2)
  //        | (digitalRead(A5) << 3)
  //        | (digitalRead(A3) << 4)
  //        | (digitalRead(40) << 5)
  //        | (digitalRead(A2) << 6)
  //        | (digitalRead(41) << 7)
  //        | (digitalRead(A1) << 8)
  //        | (digitalRead(3) << 9)
  //        | (digitalRead(A0) << 10)
  //        | (digitalRead(2) << 11)
  //        | (digitalRead(14) << 12)
  //        | (digitalRead(15) << 13)
  //        | (digitalRead(A4) << 14)
  //        | (digitalRead(4) << 15);
}

uint8_t getByte_LOOPY() {
  return bitRead(PINK, 0)
         | (bitRead(PINA, 0) << 1)
         | (bitRead(PINF, 6) << 2)
         | (bitRead(PINF, 5) << 3)
         | (bitRead(PINF, 3) << 4)
         | (bitRead(PING, 1) << 5)
         | (bitRead(PINF, 2) << 6)
         | (bitRead(PING, 0) << 7);
  // return digitalRead(A8)
  //        | (digitalRead(22) << 1)
  //        | (digitalRead(A6) << 2)
  //        | (digitalRead(A5) << 3)
  //        | (digitalRead(A3) << 4)
  //        | (digitalRead(40) << 5)
  //        | (digitalRead(A2) << 6)
  //        | (digitalRead(41) << 7);
}

void setByte_LOOPY(uint8_t D) {
  digitalWrite(A8, bitRead(D, 0));
  digitalWrite(22, bitRead(D, 1));
  digitalWrite(A6, bitRead(D, 2));
  digitalWrite(A5, bitRead(D, 3));
  digitalWrite(A3, bitRead(D, 4));
  digitalWrite(40, bitRead(D, 5));
  digitalWrite(A2, bitRead(D, 6));
  digitalWrite(41, bitRead(D, 7));
}

byte readByte_LOOPY(unsigned long myAddress) {
  setAddress_LOOPY(myAddress);

  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_OE, LOW);

  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  byte b = getByte_LOOPY();

  digitalWrite(LOOPY_RAMCS1, HIGH);
  digitalWrite(LOOPY_OE, HIGH);

  return b;
}

void writeByte_LOOPY(unsigned long myAddress, byte myData) {
  setAddress_LOOPY(myAddress);

  setByte_LOOPY(myData);

  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_OE, HIGH);
  digitalWrite(LOOPY_RAMWE, LOW);

  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  digitalWrite(LOOPY_RAMWE, HIGH);
  digitalWrite(LOOPY_RAMCS1, HIGH);
}

word readWord_LOOPY(unsigned long myAddress) {
  setAddress_LOOPY(myAddress);

  digitalWrite(LOOPY_ROMCE, LOW);
  digitalWrite(LOOPY_OE, LOW);

  // 16mhz = 62.5ns
  NOP;
  NOP;

  word tempWord = getWord_LOOPY();

  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_OE, HIGH);

  return tempWord;
}

// Switch data pins to write
void dataOut_LOOPY() {
  // // PA0
  // DDRA |= 0x01;
  // // PK0
  // DDRK |= 0x01;
  // // PG0, PG1, PG5 (rest unused?)
  // DDRG = 0xFF;
  // // PJ0-1 (rest unused?)
  // DDRJ = 0xFF;
  // // PE4-PE5 (rest unused?)
  // DDRE = 0xFF;
  // // PF0-PF6
  // DDRF |= 0b0111111;

  pinMode(A8, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(41, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(4, OUTPUT);
}

// Switch data pins to read
void dataIn_LOOPY() {
  // // PA0
  // DDRA &= ~0x01;
  // // PK0
  // DDRK &= ~0x01;
  // // PG0, PG1, PG5 (rest unused?)
  // DDRG = 0x00;
  // // PJ0-1 (rest unused?)
  // DDRJ = 0x00;
  // // PE4-PE5 (rest unused?)
  // DDRE = 0x00;
  // // PF0-PF6
  // DDRF &= ~0b0111111;
  pinMode(A8, INPUT);
  pinMode(22, INPUT);
  pinMode(A6, INPUT);
  pinMode(A5, INPUT);
  pinMode(A3, INPUT);
  pinMode(40, INPUT);
  pinMode(A2, INPUT);
  pinMode(41, INPUT);
  pinMode(A1, INPUT);
  pinMode(3, INPUT);
  pinMode(A0, INPUT);
  pinMode(2, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(A4, INPUT);
  pinMode(4, INPUT);
}

//******************************************
// CART INFO
//******************************************

void getCartInfo_LOOPY() {
  display_Clear();

  // Set control
  dataIn_LOOPY();

  // First word after header stored as 32-bit pointer at 0h, final word (inclusive) at 4h
  // based on SH-1 memory mapped location of ROM (shift to rebase on zero)
  loopyChecksumStart = (((uint32_t)readWord_LOOPY(0x0) << 16) | (uint32_t)readWord_LOOPY(0x2)) - LOOPY_MAP_ROM_ZERO;
  loopyChecksumEnd = (((uint32_t)readWord_LOOPY(0x4) << 16) | (uint32_t)readWord_LOOPY(0x6)) - LOOPY_MAP_ROM_ZERO;
  // Full cart size DOES include the header, don't subtract it off :)
  cartSize = loopyChecksumEnd + 2;

  // SRAM first and last byte locations stored at 10h and 14h, based on SH-1 memory mapped location of SRAM
  uint32_t loopySramStart = (((uint32_t)readWord_LOOPY(0x10) << 16) | (uint32_t)readWord_LOOPY(0x12)) - LOOPY_MAP_SRAM_ZERO;
  uint32_t loopySramEnd = (((uint32_t)readWord_LOOPY(0x14) << 16) | (uint32_t)readWord_LOOPY(0x16)) - LOOPY_MAP_SRAM_ZERO;
  sramSize = loopySramEnd - loopySramStart + 1;
  // TODO sanity check these values

  // Get internal checksum from header
  loopyChecksum = ((uint32_t)readWord_LOOPY(0x8) << 16) | (uint32_t)readWord_LOOPY(0xA);
  sprintf(checksumStr, "%08lX", loopyChecksum);

  // Look up in database
  loopyCartRecognized = setRomName("loopy.txt", checksumStr);

  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Checksum: "));
  println_Msg(checksumStr);
  print_Msg(F("Size: "));
  print_Msg(cartSize * 8 / 1024 / 1024);
  println_Msg(F(" MBit"));
  print_Msg(F("Sram: "));
  print_Msg(sramSize * 8 / 1024);
  println_Msg(F(" KBit"));
  println_Msg(F(" "));

#if (defined(enable_OLED) || defined(enable_LCD))
  // Wait for user input
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
}

//******************************************
// READ CODE
//******************************************

void readROM_LOOPY() {
  dataIn_LOOPY();

  sprintf(fileName, "%s.bin", romName);

  EEPROM_readAnything(0, foldern);
  sprintf(folder, "LOOPY/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  draw_progressbar(0, cartSize);

  const size_t sdBufferSize = 512;
  uint32_t sum = 0;

  for (unsigned long ptr = 0; ptr < cartSize;) {
    word myWord = readWord_LOOPY(ptr);

    // aggregate checksum over 16-bit words, starting at 80h, this address is stored in header but never varies
    if (ptr >= loopyChecksumStart) {
      sum += myWord;
    }

    // Store in buffer
    sdBuffer[ptr++ % sdBufferSize] = (myWord >> 8) & 0xFF;
    sdBuffer[ptr++ % sdBufferSize] = myWord & 0xFF;

    // Flush when buffer full
    if (ptr % sdBufferSize == 0) {
      myFile.write(sdBuffer, sdBufferSize);
      blinkLED();
      // Only update progress bar every 64kb
      if (ptr % 0x10000 == 0) {
        draw_progressbar(ptr, cartSize);
      }
    }
  }
  // TODO this assumes size is divisible by 512
  myFile.close();

  // Instead of the CRC32, check the internal integrity based on the header checksum
  print_Msg(F("Header sum: "));
  println_Msg(checksumStr);
  print_Msg(F("Actual sum:  "));

  char calculatedChecksumStr[9];
  sprintf(calculatedChecksumStr, "%08lX", sum);
  println_Msg(calculatedChecksumStr);

  if (sum == loopyChecksum) {
    println_Msg(F("INTEGRITY OK :)"));
  } else {
    println_Msg(F("INTEGRITY FAIL! Bad dump"));
  }

  display_Update();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  //compareCRC("loopy.txt", 0, 1, 0x80);

#if (defined(enable_OLED) || defined(enable_LCD))
  // Wait for user input
  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
}

//******************************************
// SRAM
//******************************************

void writeSRAM_LOOPY() {
  dataOut_LOOPY();

  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currByte = 0; currByte < sramSize; currByte++) {
      writeByte_LOOPY(currByte, (myFile.read()));
    }
    myFile.close();
    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }
  dataIn_LOOPY();
}

void readSRAM_LOOPY() {
  dataIn_LOOPY();

  sprintf(fileName, "%s.sav", romName);

  EEPROM_readAnything(0, foldern);
  sprintf(folder, "LOOPY/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  const size_t sdBufferSize = 512;
  for (unsigned long ptr = 0; ptr < sramSize;) {
    sdBuffer[ptr++ % sdBufferSize] = readByte_LOOPY(ptr);
    if (ptr % sdBufferSize == 0) {
      myFile.write(sdBuffer, sdBufferSize);
    }
  }

  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));
  display_Update();
}

unsigned long verifySRAM_LOOPY() {
  dataIn_LOOPY();
  writeErrors = 0;

  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currBuffer = 0; currBuffer < sramSize; currBuffer += 512) {
      for (int currByte = 0; currByte < 512; currByte++) {
        byte myByte = readByte_LOOPY(currBuffer + currByte);
        sdBuffer[currByte] = myByte;
      }
      for (int i = 0; i < 512; i++) {
        if (myFile.read() != sdBuffer[i]) {
          writeErrors++;
        }
      }
    }
    myFile.close();
  } else {
    print_FatalError(sd_error_STR);
  }

  return writeErrors;
}
#endif
//******************************************
// End of File
//******************************************