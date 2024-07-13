//******************************************
// CASIO LOOPY MODULE
//******************************************
// Loopy
// Cartridge Pinout
// 90P 2.1mm pitch connector
//
//         +--------+
//    +5V -|  1  90 |- D11
//        -|  2  89 |- +5V
//        -|  3  88 |- D9
//        -|  4  87 |-
//        -|  5  86 |- D7
//        -|  6  85 |- D5
//        -|  7  84 |- D15
//        -|  8  83 |- D13
// RAMCS1 -|  9  82 |- D12
//        -| 10  81 |- D1
//  RAMWE -| 11  80 |-
//        -| 12  79 |-
//    GND -| 13  78 |- A2
//    +5V -| 14  77 |- A4
//        -| 15  76 |- A19
//        -| 16  75 |- GND
//        -| 17  74 |- A18
//        -| 18  73 |- A16
//        -| 19  72 |- A17
//        -| 20  71 |- A14
//        -| 21  70 |- A5
//    A12 -| 22  69 |- A7
//    A10 -| 23  68 |- A9
//     A8 -| 24  67 |- A11
//     A6 -| 25  66 |-
//    A13 -| 26  65 |-
//    A15 -| 27  64 |-
//    A20 -| 28  63 |-
//    +5V -| 29  62 |-
//     A3 -| 30  61 |-
//    A21 -| 31  60 |- CLK
//     A1 -| 32  59 |-
//     A0 -| 33  58 |-
//        -| 34  57 |- +5V
//     D0 -| 35  56 |- OE
//  RESET -| 36  55 |-
//     D2 -| 37  54 |- ROMCE
//     D3 -| 38  53 |-
//    D14 -| 39  52 |-
//     D4 -| 40  51 |-
//     D6 -| 41  50 |-
//     D8 -| 42  49 |-
//        -| 43  48 |-
//    D10 -| 44  47 |- GND
//    GND -| 45  46 |-
//         +--------+
//
// * Blank pins have various uses depending on cartridge but are not necessary for dumping.
// IMPORTANT: All data are stored as BIG-ENDIAN. Many ROM dumps online are little endian.
// See https://github.com/kasamikona/Loopy-Tools/blob/master/Documentation/ROM%20Structure.md
//
// By @partlyhuman
// Special thanks to @kasamikona
#ifdef ENABLE_LOOPY

// SH-1 memory map locations, ROM starts here
const uint32_t LOOPY_MAP_ROM_ZERO = 0x0E000000;
const uint32_t LOOPY_MAP_SRAM_ZERO = 0x02000000;

// Control pins
const int LOOPY_ROMCE = 42;
const int LOOPY_OE = 43;
const int LOOPY_RAMWE = 6;
const int LOOPY_RAMCS1 = 7;
const int LOOPY_RESET = A7;

// The internal checksum read from the cart header at 08h, will be checked against an actual sum
uint32_t loopyChecksum;
uint32_t loopyChecksumStart;
uint32_t loopyChecksumEnd;

char loopyRomNameLong[64];

//******************************************
// SETUP
//******************************************

void setup_LOOPY() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // PK1-PK7, PA1-PA7, PC0-PC3, PL0-PL3
  // Take whole port and unset the exceptions later
  DDRK = DDRA = DDRC = DDRL = 0xFF;

  // Control pins, all active low
  pinMode(LOOPY_ROMCE, OUTPUT);
  pinMode(LOOPY_OE, OUTPUT);
  pinMode(LOOPY_RAMWE, OUTPUT);
  pinMode(LOOPY_RAMCS1, OUTPUT);
  pinMode(LOOPY_RESET, OUTPUT);
  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_OE, HIGH);
  digitalWrite(LOOPY_RAMWE, HIGH);
  digitalWrite(LOOPY_RAMCS1, HIGH);
  digitalWrite(LOOPY_RESET, HIGH);

  // Set Pins (D0-D15) to Input
  dataIn_LOOPY();

  getCartInfo_LOOPY();

  mode = CORE_LOOPY;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char loopyMenuItem4[] PROGMEM = "Format SRAM";
static const char* const menuOptionsLOOPY[] PROGMEM = { FSTRING_REFRESH_CART, FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, loopyMenuItem4, FSTRING_RESET };

void loopyMenu() {
  convertPgm(menuOptionsLOOPY, 5);
  uint8_t mainMenu = question_box(F("CASIO LOOPY MENU"), menuOptions, 5, 0);
  display_Clear();
  display_Update();
  bool waitForInput = false;

  switch (mainMenu) {
    case 0:
      setup_LOOPY();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_LOOPY();
      sd.chdir("/");
      waitForInput = true;
      break;

    case 2:
      // Read SRAM
      sd.chdir("/");
      println_Msg(F("Reading SRAM..."));
      display_Update();
      readSRAM_LOOPY();
      sd.chdir("/");
      waitForInput = true;
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
      waitForInput = true;
      break;

    case 4:
      // Format SRAM
      println_Msg(F("Formatting SRAM..."));
      display_Update();
      formatSRAM_LOOPY();
      waitForInput = true;
      break;

    case 5:
      // reset
      resetArduino();
      break;
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  if (waitForInput) {
    // Wait for user input
    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
#endif
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
  // CAUTION PORTL is shared, writing to PORTL indiscriminately will mess with CE/OE
  // D42	PL7	CE
  // D43	PL6	OE
  // D44	PL5
  // D45	PL4
  // D46	PL3	A11
  // D47	PL2	A9
  // D48	PL1	A7
  // D49	PL0	A5
  digitalWrite(46, bitRead(A, 11));
  digitalWrite(47, bitRead(A, 9));
  digitalWrite(48, bitRead(A, 7));
  digitalWrite(49, bitRead(A, 5));
  // PORTL = (bitRead(A, 5))
  //         | (bitRead(A, 7) << 1)
  //         | (bitRead(A, 9) << 2)
  //         | (bitRead(A, 11) << 3);
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
}

void setByte_LOOPY(uint8_t D) {
  // Since D lines are spread among so many ports, this is far more legible, and only used for SRAM
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

  // 100ns MAX
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return getByte_LOOPY();
}

void writeByte_LOOPY(unsigned long myAddress, byte myData) {
  setAddress_LOOPY(myAddress);

  digitalWrite(LOOPY_RAMWE, LOW);

  // tWHZ 35
  NOP;
  NOP;
  dataOut_LOOPY();

  setByte_LOOPY(myData);

  // tWP 60
  NOP;
  NOP;
  NOP;
  NOP;

  digitalWrite(LOOPY_RAMWE, HIGH);
  dataIn_LOOPY();
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

  // Only bothering to change lower bits since we never write words just bytes
  pinMode(A8, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(41, OUTPUT);
  // pinMode(A1, OUTPUT);
  // pinMode(3, OUTPUT);
  // pinMode(A0, OUTPUT);
  // pinMode(2, OUTPUT);
  // pinMode(14, OUTPUT);
  // pinMode(15, OUTPUT);
  // pinMode(A4, OUTPUT);
  // pinMode(4, OUTPUT);
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

// A little different than many games, loopy DB stores the checksum present in the header, so we can determine the rom name before saving, without renaming
bool setRomName_LOOPY(const char* database, char* crcStr, int stripExtensionChars = 4) {
  char gamename[96];
  char crc_search[9];
  bool found;

  sd.chdir();
  if (!myFile.open(database, O_READ)) {
    return false;
  }
  while (myFile.available()) {
    get_line(gamename, &myFile, sizeof(gamename));
    get_line(crc_search, &myFile, sizeof(crc_search));
    skip_line(&myFile);  //Skip every 3rd line
    if (strcmp(crc_search, crcStr) == 0) {
      found = true;
      strlcpy(loopyRomNameLong, gamename, strlen(gamename) - stripExtensionChars + 1);
      strcpy(romName, loopyRomNameLong);
      break;
    }
  }
  myFile.close();
  return found;
}

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
  strcpy(loopyRomNameLong, "LOOPY");
  strcpy(romName, loopyRomNameLong);
  setRomName_LOOPY("loopy.txt", checksumStr);

  println_Msg(F("Cart Info"));
  println_Msg(FS(FSTRING_SPACE));
  print_Msg(FS(FSTRING_NAME));
  println_Msg(loopyRomNameLong);
  print_Msg(FS(FSTRING_CHECKSUM));
  println_Msg(checksumStr);
  print_Msg(FS(FSTRING_SIZE));
  print_Msg(cartSize * 8 / 1024 / 1024);
  println_Msg(F(" MBit"));
  print_Msg(F("Sram: "));
  print_Msg(sramSize * 8 / 1024);
  println_Msg(F(" KBit"));
  println_Msg(FS(FSTRING_SPACE));

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
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

  createFolderAndOpenFile("LOOPY", "ROM", romName, "bin");

  draw_progressbar(0, cartSize);

  const size_t sdBufferSize = 512;
  uint32_t sum = 0;

  digitalWrite(LOOPY_ROMCE, LOW);

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

  digitalWrite(LOOPY_ROMCE, HIGH);

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
}

//******************************************
// SRAM
//******************************************

void writeSRAM_LOOPY() {
  // Being nice to the SRAM and not touching the data bus except when WE is LOW
  dataIn_LOOPY();

  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Restoring SRAM from"));
  println_Msg(filePath);
  display_Update();

  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_RESET, HIGH);
  digitalWrite(LOOPY_OE, LOW);

  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currByte = 0; currByte < sramSize; currByte++) {
      writeByte_LOOPY(currByte, myFile.read());
      if (currByte % 512 == 0) {
        blinkLED();
      }
    }
    myFile.close();

    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }

  digitalWrite(LOOPY_RAMCS1, HIGH);
  digitalWrite(LOOPY_OE, HIGH);

  dataIn_LOOPY();
}

void formatSRAM_LOOPY() {
  dataIn_LOOPY();
  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_RESET, HIGH);
  digitalWrite(LOOPY_OE, LOW);

  for (unsigned long currByte = 0; currByte < sramSize; currByte++) {
    writeByte_LOOPY(currByte, 0);
    if (currByte % 512 == 0) {
      blinkLED();
    }
  }

  digitalWrite(LOOPY_RAMCS1, HIGH);
  digitalWrite(LOOPY_OE, HIGH);
  dataIn_LOOPY();
  print_STR(done_STR, 1);
  display_Update();
}

void readSRAM_LOOPY() {
  dataIn_LOOPY();

  createFolder("LOOPY", "SAVE", romName, "sav");

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_OE, LOW);

  const size_t sdBufferSize = 512;
  for (unsigned long ptr = 0; ptr < sramSize;) {
    uint8_t b = readByte_LOOPY(ptr);
    sdBuffer[ptr++ % sdBufferSize] = b;
    if (ptr % sdBufferSize == 0) {
      myFile.write(sdBuffer, sdBufferSize);
      blinkLED();
    }
  }

  digitalWrite(LOOPY_OE, HIGH);
  digitalWrite(LOOPY_RAMCS1, HIGH);

  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  display_Update();
}

unsigned long verifySRAM_LOOPY() {
  dataIn_LOOPY();
  writeErrors = 0;

  digitalWrite(LOOPY_ROMCE, HIGH);
  digitalWrite(LOOPY_RAMCS1, LOW);
  digitalWrite(LOOPY_OE, LOW);

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

  digitalWrite(LOOPY_OE, HIGH);
  digitalWrite(LOOPY_RAMCS1, HIGH);

  return writeErrors;
}
#endif
