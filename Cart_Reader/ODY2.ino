//******************************************
// MAGNAVOX ODYSSEY 2 MODULE
//******************************************
#if defined(ENABLE_ODY2)
// Magnavox Odyssey 2
// Philips Videopac/Videopac+
// Cartridge Pinout
// 30P 3.96mm pitch connector
//
//       FRONT            BACK
//        SIDE            SIDE
//              +------+
//          T0 -| 1  A |- /WR
//          D0 -| 2  B |- GND
//          D1 -| 3  C |- GND
//          D2 -| 4  D |- VCC (+5V)
//          D3 -| 5  E |- CS3
//          D4 -| 6  F |- /PSEN (/CE)
//          D5 -| 7  G |- A0
//          D6 -| 8  H |- A1
//          D7 -| 9  J |- A2
//   A10 (P22) -| 10 K |- A3
//  /CS1 (P14) -| 11 L |- A4
//         P11 -| 12 M |- A5
//         P10 -| 13 N |- A7
//   A11 (P23) -| 14 P |- A6
//    A9 (P21) -| 15 R |- A8 (P20)
//              +------+
//
// NOTE:  ADDRESS A7/A6 PIN ORDER ON PINS N & P.
// NOTE:  MOST CARTS DO NOT CONNECT A10 ON PIN 10.
//
//                           BACK
//      +---------------------------------------------+
//      | A  B  C  D  E  F  G  H  J  K  L  M  N  P  R |
// LEFT |                                             | RIGHT
//      | 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 |
//      +---------------------------------------------+
//                           FRONT
//
// CONTROL PINS:
// T0(PH0)    - SNES RESET
// CS3(PH3)   - SNES /CS
// /CS1(PH4)  - SNES /IRQ
// /WR(PH5)   - SNES /WR
// /PSEN(PH6) - SNES /RD

byte ODY2[] = { 2, 4, 8, 12, 16 };
byte ody2lo = 0;  // Lowest Entry
byte ody2hi = 4;  // Highest Entry

byte ody2mapper;
byte ody2size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptionsODY2[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_ODY2() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Odyssey 2 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //        T0(PH0)   ---(PH1)   CS3(PH3)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //        T0(PH0)   ---(PH1)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set CS3(PH3) to LOW
  PORTH &= ~(1 << 3);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_ODY2();
  strcpy(romName, "ODYSSEY2");

  mode = CORE_ODY2;
}

void ody2Menu() {
  convertPgm(menuOptionsODY2, 4);
  uint8_t mainMenu = question_box(F("ODYSSEY 2 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_ODY2();
      setup_ODY2();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_ODY2();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_ODY2();
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

uint8_t readData_ODY2(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13

  // Set /PSEN (/CE) to LOW
  PORTH &= ~(1 << 6);  // /PSEN LOW (ENABLE)
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Pull /PSEN (/CE) to HIGH
  PORTH |= (1 << 6);  // /PSEN HIGH (DISABLE)

  return ret;
}

void readSegment_ODY2(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_ODY2(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void bankSwitch_ODY2(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /CS1(PH4) to LOW
  PORTH &= ~(1 << 4);
  // Set /WR(PH5) to LOW
  PORTH &= ~(1 << 5);
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTC = data;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Set /CS1(PH4) to HIGH
  PORTH |= (1 << 4);

  DDRC = 0x00;  // Reset to Input
}

void readROM_ODY2() {
  createFolderAndOpenFile("ODY2", "ROM", romName, "bin");

  if (ody2mapper == 1) {  // A10 CONNECTED
    // Videopac 31:  Musician
    // Videopac 40:  4 in 1 Row/4 en 1 Ligne
    readSegment_ODY2(0x0000, 0x1000);  // 4K
  }
  // A10 NOT CONNECTED
  else if (ody2size > 2) {  // 12K/16K (BANKSWITCH)
    // Videopac+ 55:  Neutron Star         12K = 2K x 6 Banks
    // Videopac+ 58:  Norseman             12K = 2K x 6 Banks
    // Videopac+ 59:  Helicopter Rescue    16K = 2K x 8 Banks
    // Videopac+ 60:  Trans American Rally 16K = 2K x 8 Banks
    uint8_t ody2banks = (ody2size * 4) / 2;
    for (int x = (ody2banks - 1); x >= 0; x--) {
      bankSwitch_ODY2(0x80, ~x);
      readSegment_ODY2(0x0400, 0x0C00);  // 2K x 6/8 = 12K/16K
    }
  } else {                             // STANDARD SIZES
    readSegment_ODY2(0x0400, 0x0C00);  // 2K
    if (ody2size > 0) {
      readSegment_ODY2(0x1400, 0x1C00);  // +2K = 4K
      if (ody2size > 1) {
        readSegment_ODY2(0x2400, 0x2C00);  // +2K = 6K
        readSegment_ODY2(0x3400, 0x3C00);  // +2K = 8K
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
void printRomSize_ODY2(int index) {
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(ODY2[index]);
}
#endif

void setROMSize_ODY2() {
  byte newody2size;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (ody2lo == ody2hi)
    newody2size = ody2lo;
  else {
    newody2size = navigateMenu(ody2lo, ody2hi, &printRomSize_ODY2);

    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(ODY2[newody2size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (ody2lo == ody2hi)
    newody2size = ody2lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (ody2hi - ody2lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(ODY2[i + ody2lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newody2size = sizeROM.toInt() + ody2lo;
    if (newody2size > ody2hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(ODY2[newody2size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newody2size);
  ody2size = newody2size;
}

void checkStatus_ODY2() {
  EEPROM_readAnything(7, ody2mapper);
  EEPROM_readAnything(8, ody2size);
  if (ody2mapper > 1) {
    ody2mapper = 0;
    EEPROM_writeAnything(7, ody2mapper);
  }
  if (ody2size > 4) {
    ody2size = 0;
    EEPROM_writeAnything(8, ody2size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ODYSSEY 2 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(ody2mapper);
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(ODY2[ody2size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:   "));
  Serial.println(ody2mapper);
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(ODY2[ody2size]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_ODY2() {
  //go to root
  sd.chdir();

  struct database_entry_mapper_size entry;

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("ody2cart.txt", O_READ)) {
    seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineMapperSize, &entry)) {
      EEPROM_writeAnything(7, entry.gameMapper);
      EEPROM_writeAnything(8, entry.gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
//******************************************
// End of File
//******************************************