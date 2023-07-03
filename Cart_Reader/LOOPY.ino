//******************************************
// CASIO LOOPY MODULE
//******************************************
#ifdef enable_LOOPY

// SH-1 memory map locations, ROM starts here
const uint32_t LOOPY_MAP_ROM_ZERO = 0x0E000000;
const uint32_t LOOPY_MAP_SRAM_ZERO = 0x02000000;
const uint32_t LOOPY_SRAM_SIZE = 0x2000;

//******************************************
// SETUP
//******************************************

void setup_LOOPY() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // PK1-PK7, PA1-PA7, PC0-PC3, PL0-PL3 // Take whole port and unset the exceptions later
  DDRK = DDRA = DDRC = DDRL = 0xFF;

  // Set Control Pins to Output
  //      /RAMWE(PH6)/RAMCS2(PH4)
  DDRH |= (1 << 6) | (1 << 4);
  //      /CE(PL7)   /OE(PL6)
  DDRL |= (1 << 7) | (1 << 6);

  // Set Pins (D0-D15) to Input
  dataIn_LOOPY();

  // Reset HIGH (PF7)
  DDRF |= (1 << 7);
  PORTF |= (1 << 7);

  strcpy(romName, "LOOPY");
  getCartInfo_LOOPY();

  mode = mode_LOOPY;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char loopyMenuItem1[] PROGMEM = "Read ROM";
static const char loopyMenuItem2[] PROGMEM = "Read SRAM";
static const char loopyMenuItem3[] PROGMEM = "Write SRAM";
//static const char loopyMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsLOOPY[] PROGMEM = { loopyMenuItem1, loopyMenuItem2, loopyMenuItem3, string_reset2 };

void loopyMenu() {
  convertPgm(menuOptionsLOOPY, 4);
  uint8_t mainMenu = question_box(F("CASIO LOOPY MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Read ROM
      sd.chdir("/");
      readROM_LOOPY();
      sd.chdir("/");
      break;

    case 1:
      // Read SRAM
      if (sramSize) {
        sd.chdir("/");
        display_Clear();
        println_Msg(F("Reading SRAM..."));
        display_Update();
        readSRAM_LOOPY();
        sd.chdir("/");
      } else {
        print_Error(F("Cart has no SRAM"));
      }
#if (defined(enable_OLED) || defined(enable_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 2:
      // Write SRAM
      if (sramSize) {
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
      } else {
        print_Error(F("Cart has no SRAM"));
      }
#if (defined(enable_OLED) || defined(enable_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  LOW LEVEL FUNCTIONS
//******************************************

void setAddress_LOOPY(unsigned long A) {
  // PK1	A0
  // PK2	A1
  // PK3	A21
  // PK4	A3
  // PK5	A20
  // PK6	A15
  // PK7	A13
  PORTK = (bitRead(A, 0) << 1)
          | (bitRead(A, 1) << 2)
          | (bitRead(A, 21) << 3)
          | (bitRead(A, 3) << 4)
          | (bitRead(A, 20) << 5)
          | (bitRead(A, 15) << 6)
          | (bitRead(A, 13) << 7);
  // PA1	A2
  // PA2	A4
  // PA3	A19
  // PA4	A18
  // PA5	A16
  // PA6	A17
  // PA7	A14
  PORTA = (bitRead(A, 2) << 1)
          | (bitRead(A, 4) << 2)
          | (bitRead(A, 19) << 3)
          | (bitRead(A, 18) << 4)
          | (bitRead(A, 16) << 5)
          | (bitRead(A, 17) << 6)
          | (bitRead(A, 14) << 7);
  // PC0	A6
  // PC1	A8
  // PC2	A10
  // PC3	A12
  PORTC = (bitRead(A, 6))
          | (bitRead(A, 8) << 1)
          | (bitRead(A, 10) << 2)
          | (bitRead(A, 12) << 3);
  // PL0	A5
  // PL1	A7
  // PL2	A9
  // PL3	A11
  PORTL = (bitRead(A, 5))
          | (bitRead(A, 7) << 1)
          | (bitRead(A, 9) << 2)
          | (bitRead(A, 11) << 3);
}

uint16_t getWord_LOOPY() {
  return digitalRead(A8)
         | (digitalRead(22) << 1)
         | (digitalRead(A6) << 2)
         | (digitalRead(A5) << 3)
         | (digitalRead(A3) << 4)
         | (digitalRead(40) << 5)
         | (digitalRead(A2) << 6)
         | (digitalRead(41) << 7)
         | (digitalRead(A1) << 8)
         | (digitalRead(3) << 9)
         | (digitalRead(A0) << 10)
         | (digitalRead(2) << 11)
         | (digitalRead(14) << 12)
         | (digitalRead(15) << 13)
         | (digitalRead(A4) << 14)
         | (digitalRead(4) << 15);
}

uint8_t getByte_LOOPY() {
  return digitalRead(A8)
         | (digitalRead(22) << 1)
         | (digitalRead(A6) << 2)
         | (digitalRead(A5) << 3)
         | (digitalRead(A3) << 4)
         | (digitalRead(40) << 5)
         | (digitalRead(A2) << 6)
         | (digitalRead(41) << 7);
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

void writeByte_LOOPY(unsigned long myAddress, byte myData) {
  setAddress_LOOPY(myAddress);

  __asm__("nop\n\t");

  PORTA = myData;

  // Set CS2(PH0), /CE(PH3), /OE(PH6) to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 6);
  // Set /CS1(PH4), /WE0(PH5) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 4) | (1 << 5);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

word readWord_LOOPY(unsigned long myAddress) {
  setAddress_LOOPY(myAddress);

  __asm__("nop\n\t");

  // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 4) | (1 << 5);
  // Set /CE(PH3), /OE(PH6) to LOW
  PORTH &= ~(1 << 3) & ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  // Set /CE(PH3), /OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);
  // Setting CS2(PH0) LOW
  PORTH &= ~(1 << 0);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempWord;
}

byte readByte_LOOPY(unsigned long myAddress) {  // SRAM BYTE
  setAddress_LOOPY(myAddress);

  __asm__("nop\n\t");

  // Set CS2(PH0), /CE(PH3), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5);
  // Set /CS1(PH4), /OE(PH6) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  byte tempByte = PINA;

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set /CS1(PH4), /OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);
  // Setting CS2(PH0) LOW
  PORTH &= ~(1 << 0);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
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
  // Set control
  dataIn_LOOPY();

  // Last word of ROM stored as 32-bit pointer at 000004h
  // TODO make sure you have endianness right when interpreting this
  uint32_t headerRomSize = ((uint32_t)readWord_LOOPY(0x4) << 16 | (uint32_t)readWord_LOOPY(0x6));
  cartSize = headerRomSize - LOOPY_MAP_ROM_ZERO + 2;

  // Get internal CRC32 from header
  uint32_t cartHeaderCrc = (uint32_t)readWord_LOOPY(0x8) << 16 | (uint32_t)readWord_LOOPY(0xA);
  snprintf(checksumStr, 8, "%08X", cartHeaderCrc);

  // Look up in database
  // compareCRC("loopy.txt", cartId, false, 0);

  // SRAM size can be calculated from subtracting 32bit pointers 000014h (last byte of sram, memory mapped) - 000010h (first byte of sram, memory mapped)
  // But this is fine
  sramSize = LOOPY_SRAM_SIZE;

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  // print_Msg(F("Name: "));
  // println_Msg(romName);
  print_Msg(F("CRC32: "));
  println_Msg(checksumStr);
  print_Msg(F("Size: "));
  print_Msg(cartSize * 8 / 1024 / 1024);
  println_Msg(F(" MBit"));
  print_Msg(F("Sram: "));
  if (sramSize > 0) {
    print_Msg(sramSize * 8 / 1024);
    println_Msg(F(" KBit"));
  } else
    println_Msg(F("None"));
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

  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  EEPROM_readAnything(0, foldern);
  sprintf(folder, "LOOPY/ROM/%s/%d", romName, foldern);
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

  word d = 0;
  uint32_t progress = 0;
  draw_progressbar(0, cartSize);

  for (unsigned long currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 256) {
    for (int currWord = 0; currWord < 256; currWord++) {
      word myWord = readWord_LOOPY(currBuffer + currWord);
      // Split word into two bytes
      sdBuffer[d] = ((myWord >> 8) & 0xFF);
      sdBuffer[d + 1] = (myWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
    progress += 512;
    draw_progressbar(progress, cartSize);
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("loopy.txt", 0, 1, 0);

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
      //        writeWord_LOOPY(currByte, ((myFile.read() << 8 ) & 0xFF));
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

  strcpy(fileName, romName);
  strcat(fileName, ".sav");

  EEPROM_readAnything(0, foldern);
  sprintf(folder, "LOOPY/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }
  for (unsigned long currBuffer = 0; currBuffer < sramSize; currBuffer += 512) {
    for (int currByte = 0; currByte < 512; currByte++) {
      byte myByte = readByte_LOOPY(currBuffer + currByte);
      sdBuffer[currByte] = myByte;
    }
    myFile.write(sdBuffer, 512);
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