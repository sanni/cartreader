//******************************************
// EMERSON ARCADIA 2001 MODULE
//******************************************
#if defined(ENABLE_ARC)
// Emerson Arcadia 2001
// Cartridge Pinout
// 30P 2.54mm pitch connector
//
//       FRONT             BACK
//        SIDE             SIDE
//              +-------+
//         GND -| 2   1 |- A13 (A12)
//   VCC (+5V) -| 4   3 |- D3
//          A0 -| 6   5 |- D4
//          A1 -| 8   7 |- D5
//          A2 -| 10  9 |- D6
//          A3 -| 12 11 |- D7
//          A4 -| 14 13 |- D0
//          A5 -| 16 15 |- D2
//          A6 -| 18 17 |- D1
//          A7 -| 20 19 |- NC
//          A8 -| 22 21 |- NC
//          A9 -| 24 23 |- NC
//         A10 -| 26 25 |- GND
//         A11 -| 28 27 |- GND
//   A12 (/EN) -| 30 29 |- NC
//              +-------+
//
//                                    BACK
//       +------------------------------------------------------------+
//       | 1   3   5   7   9   11  13  15  17  19  21  23  25  27  29 |
// LEFT  |                                                            | RIGHT
//       | 2   4   6   8   10  12  14  16  18  20  22  24  26  28  30 |
//       +------------------------------------------------------------+
//                                    FRONT
//

byte ARC[] = { 2, 4, 6, 8 };
byte arclo = 0;  // Lowest Entry
byte archi = 3;  // Highest Entry

byte arcsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptionsARC[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_ARC() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Arcadia 2001 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Unused Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_ARC();
  strcpy(romName, "ARCADIA");

  mode = CORE_ARC;
}

void arcMenu() {
  convertPgm(menuOptionsARC, 4);
  uint8_t mainMenu = question_box(F("ARCADIA 2001 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_ARC();
      setup_ARC();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_ARC();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_ARC();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// READ CODE
//******************************************

uint8_t readData_ARC(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  return ret;
}

void readSegment_ARC(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_ARC(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readROM_ARC() {
  createFolderAndOpenFile("ARC", "ROM", romName, "bin");

  readSegment_ARC(0x0000, 0x0800);  // 2K
  if (arcsize > 0) {
    readSegment_ARC(0x0800, 0x1000);  // +2K = 4K
    if (arcsize > 1) {
      readSegment_ARC(0x2000, 0x2800);  // +2K = 6K
      if (arcsize > 2) {
        readSegment_ARC(0x2800, 0x3000);  // +2K = 8K
      }
    }
  }
  myFile.close();

  printCRC(fileName, NULL, 0);

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void printRomSize_ARC(int index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(ARC[index]);
}
#endif

void setROMSize_ARC() {
  byte newarcsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (arclo == archi)
    newarcsize = arclo;
  else {
    newarcsize = navigateMenu(arclo, archi, &printRomSize_ARC);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(ARC[newarcsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (arclo == archi)
    newarcsize = arclo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (archi - arclo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(ARC[i + arclo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newarcsize = sizeROM.toInt() + arclo;
    if (newarcsize > archi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(ARC[newarcsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newarcsize);
  arcsize = newarcsize;
}

void checkStatus_ARC() {
  EEPROM_readAnything(8, arcsize);
  if (arcsize > 3) {
    arcsize = 0;
    EEPROM_writeAnything(8, arcsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ARCADIA 2001 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(ARC[arcsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(ARC[arcsize]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_ARC() {
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("arccart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
//******************************************
// End of File
//******************************************
