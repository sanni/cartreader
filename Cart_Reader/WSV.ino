//******************************************
// WSV MODULE
//******************************************
#ifdef ENABLE_WSV
// Watara Supervision
// Cartridge Pinout
// 40P 2.5mm pitch connector
//
//       BACK            LABEL
//       SIDE            SIDE
//            +-------+
//       /RD -| 1  40 |- +5V
//        A0 -| 2  39 |- nc
//        A1 -| 3  38 |- nc
//        A2 -| 4  37 |- nc
//        A3 -| 5  36 |- nc
//        A4 -| 6  35 |- /WR
//        A5 -| 7  34 |- D0
//        A6 -| 8  33 |- D1
//        A7 -| 9  32 |- D2
//        A8 -| 10 31 |- D3
//        A9 -| 11 30 |- D4
//       A10 -| 12 29 |- D5
//       A11 -| 13 28 |- D6
//       A12 -| 14 27 |- D7
//       A13 -| 15 26 |- nc
//       A14 -| 16 25 |- nc
//       A15 -| 17 24 |- L1
//       A16 -| 18 23 |- L2
//        L3 -| 19 22 |- GND
//        L0 -| 20 21 |- PWR GND
//            +-------+
//
// L3 - L0 are the Link Port's I/O - only the 'MAGNUM' variant
// routed these to the cartridge slot as additional banking bits.
//
// CONTROL PINS:
// /WR - (PH5)
// /RD - (PH6)

const word WSV[] PROGMEM = { 32, 64, 512 };
byte wsvlo = 0;  // Lowest Entry
byte wsvhi = 2;  // Highest Entry

byte wsvsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// SETUP
//******************************************

void setup_WSV() {
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7 (BA5-BA7 UNUSED)
  DDRL = 0xFF;
  //PA0-PA7 (UNUSED)
  DDRA = 0xFF;

  // Set Data Pins (D0-D7) to Input
  // D0 - D7
  DDRC = 0x00;

  // Set Control Pins to Output
  //       WR(PH5)    RD(PH6)
  //  DDRH |= (1 << 5) | (1 << 6);
  //      ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)   /RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Switch WR(PH5) to HIGH
  //  PORTH |= (1 << 5);
  // Switch RD(PH6) to LOW
  //  PORTH &= ~(1 << 6);
  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Switch RD(PH6) to LOW
  PORTH &= ~(1 << 6);

  // Set Unused Pins HIGH
  PORTL = 0xE0;
  PORTA = 0xFF;
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_WSV();
  strcpy(romName, "SUPERVISION");

  mode = CORE_WSV;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char* const menuOptionsSV[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void wsvMenu() {
  convertPgm(menuOptionsSV, 4);
  uint8_t mainMenu = question_box(F("SUPERVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_WSV();
      setup_WSV();
      break;

    case 1:
      // Read Rom
      sd.chdir("/");
      readROM_WSV();
      sd.chdir("/");
      break;

    case 2:
      setROMSize_WSV();
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

// WRITE
void controlOut_WSV() {
  // Switch RD(PH6) to HIGH
  PORTH |= (1 << 6);
  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);
}

// READ
void controlIn_WSV() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch RD(PH6) to LOW
  PORTH &= ~(1 << 6);
}

void dataIn_WSV() {
  DDRC = 0x00;
}

void dataOut_WSV() {
  DDRC = 0xFF;
}

uint8_t readByte_WSV(uint32_t addr) {
  PORTF = addr & 0xFF;
  PORTK = (addr >> 8) & 0xFF;
  PORTL = (addr >> 16) & 0xFF;

  // Wait for data bus
  // 6 x 62.5ns = 375ns
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;
  NOP;

  return ret;
}

//******************************************
// READ CODE
//******************************************

void readROM_WSV() {
  createFolderAndOpenFile("WSV", "ROM", romName, "sv");

  // start reading rom
  dataIn_WSV();
  controlIn_WSV();

  romSize = pgm_read_word(&(WSV[wsvsize]));

  uint32_t romStart = 0;
  if (romSize < 64)
    romStart = 0x8000;
  uint32_t romEnd = (uint32_t)romSize * 0x400;
  for (uint32_t addr = 0; addr < romEnd; addr += 512) {
    for (uint16_t w = 0; w < 512; w++)
      sdBuffer[w] = readByte_WSV(romStart + addr + w);
    myFile.write(sdBuffer, 512);
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("wsv.txt", 0, 1, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_WSV(int index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(pgm_read_word(&(WSV[index])));
}
#endif

void setROMSize_WSV() {
  byte newwsvsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (wsvlo == wsvhi)
    newwsvsize = wsvlo;
  else {
    newwsvsize = navigateMenu(wsvlo, wsvhi, &printRomSize_WSV);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(pgm_read_word(&(WSV[newwsvsize])));
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (wsvlo == wsvhi)
    newwsvsize = wsvlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (wsvhi - wsvlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_word(&(WSV[i + wsvlo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newwsvsize = sizeROM.toInt() + wsvlo;
    if (newwsvsize > wsvhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(pgm_read_word(&(WSV[newwsvsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newwsvsize);
  wsvsize = newwsvsize;
}

void checkStatus_WSV() {
  EEPROM_readAnything(8, wsvsize);
  if (wsvsize > 2) {
    wsvsize = 1;  // default 64K
    EEPROM_writeAnything(8, wsvsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("WATARA SUPERVISION"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(pgm_read_word(&(WSV[wsvsize])));
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(pgm_read_word(&(WSV[wsvsize])));
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
struct database_entry_WSV {
  char crc_search[9];
  byte gameSize;
};

void readDataLine_WSV(FsFile& database, void* entry) {
  struct database_entry_WSV* castEntry = (database_entry_WSV*)entry;
  // Read CRC32 checksum
  for (byte i = 0; i < 8; i++) {
    checksumStr[i] = char(database.read());
  }

  // Skip over semicolon
  database.seekCur(1);

  // Read CRC32 of first 512 bytes
  for (byte i = 0; i < 8; i++) {
    castEntry->crc_search[i] = char(database.read());
  }

  // Skip over semicolon
  database.seekCur(1);

  // Read rom size
  // Read the next ascii character and subtract 48 to convert to decimal
  castEntry->gameSize = ((database.read() - 48) * 10) + (database.read() - 48);

  // Skip rest of line
  database.seekCur(2);
}

void printDataLine_WSV(void* entry) {
  struct database_entry_WSV* castEntry = (database_entry_WSV*)entry;
  print_Msg(FS(FSTRING_SIZE));
  if (castEntry->gameSize == 51)
    print_Msg(F("512"));
  else
    print_Msg(castEntry->gameSize);
  println_Msg(F("KB"));
}

void setCart_WSV() {
  //go to root
  sd.chdir();

  struct database_entry_WSV entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("wsv.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLine_WSV, &entry, &printDataLine_WSV)) {
      //word WSV[] = {32,64,512};
      switch (entry.gameSize) {
        case 32:
          wsvsize = 0;
          break;

        case 64:
          wsvsize = 1;
          break;

        case 51:
          wsvsize = 2;
          break;
        }
        EEPROM_writeAnything(8, wsvsize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
//******************************************
// End of File
//******************************************
