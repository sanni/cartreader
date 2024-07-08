//******************************************
// CASIO PV-1000/PV-2000 MODULE
//******************************************
#ifdef ENABLE_PV1000
// Casio PV-1000/PV-2000
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//  FRONT               BACK
//   SIDE               SIDE
//         +---------+
//    VCC -| B01 A01 |- GND
//     NC -| B02 A02 |- NC
//    A14 -| B03 A03 |- A15
//    A12 -| B04 A04 |- A13
//    A10 -| B05 A05 |- A11
//     A8 -| B06 A06 |- A9
//     A6 -| B07 A07 |- A7
//     A4 -| B08 A08 |- A5
//     A2 -| B09 A09 |- A3
//     A0 -| B10 A10 |- A1
//     D6 -| B11 A11 |- D7
//     D4 -| B12 A12 |- D5
//     D2 -| B13 A13 |- D3
//     D0 -| B14 A14 |- D1
//   /CS1 -| B15 A15 |- /CS2
//    /WR -| B16 A16 |- /RD
//    CON -| B17 A17 |- /IORQ
//    VCC -| B18 A18 |- GND
//         +---------+
//
//                                         BACK
//        GND NC  A15 A13 A11 A9  A7  A5  A3  A1  D7  D5  D3  D1 /CS2 /RD /IO GND
//      +-------------------------------------------------------------------------+
//      | A01 A02 A03 A04 A05 A06 A07 A08 A09 A10 A11 A12 A13 A14 A15 A16 A17 A18 |
// LEFT |                                                                         | RIGHT
//      | B01 B02 B03 B04 B05 B06 B07 B08 B09 B10 B11 B12 B13 B14 B15 B16 B17 B18 |
//      +-------------------------------------------------------------------------+
//        VCC NC  A14 A12 A10 A8  A6  A4  A2  A0  D6  D4  D2  D0 /CS1 /WR CON VCC
//                                         FRONT

// CONTROL PINS:
// /CS2(PH3) - SNES /CS
// /CS1(PH4) - SNES /IRQ
// /WR(PH5) - SNES /WR
// /RD(PH6) - SNES /RD

//******************************************
// VARIABLES
//******************************************
byte PV1000[] = {8,16};
byte pv1000lo = 0; // Lowest Entry
byte pv1000hi = 1; // Highest Entry
byte pv1000size;
byte newpv1000size;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsPV1000[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void pv1000Menu()
{
  convertPgm(menuOptionsPV1000, 4);
  uint8_t mainMenu = question_box(F("PV-1000 MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_PV1000();
      setup_PV1000();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_PV1000();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_PV1000();
      break;
    
    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_PV1000()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // PV-1000 uses A0-A15 [A16-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //       ---(PH0)   ---(PH1)  /CS2(PH3)  /CS1(PH4)   /WR(PH5)   /RD(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /CS2(PH3)  /CS1(PH4)   /WR(PH5)   /RD(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_PV1000();
  strcpy(romName, "PV1000");

  mode = CORE_PV1000;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_PV1000(uint16_t addr)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /RD to LOW
  PORTH &= ~(1 << 6); // /RD LOW (ENABLE)
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Pull /RD to HIGH
  PORTH |= (1 << 6); // /RD HIGH (DISABLE)

  return ret;
}

void readSegment_PV1000(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_PV1000(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_PV1000() 
{
  createFolderAndOpenFile("PV1000", "ROM", romName, "bin");

  if (pv1000size == 0) { // 8K
    PORTH &= ~(1 << 4); // /CS1(PH4) LOW
    readSegment_PV1000(0x0000,0x2000); // 8K
    PORTH |= (1 << 4); // /CS1(PH4) HIGH
  }
  else { // 16K
    PORTH &= ~(1 << 3); // /CS2(PH3) LOW
    readSegment_PV1000(0x0000,0x4000); // 16K
    PORTH |= (1 << 3); // /CS2(PH3) HIGH
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
void printRomSize_PV1000(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(PV1000[index]);
}
#endif

void setROMSize_PV1000()
{
  byte newpv1000size;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (pv1000lo == pv1000hi)
    newpv1000size = pv1000lo;
  else {
    newpv1000size = navigateMenu(pv1000lo, pv1000hi, &printRomSize_PV1000);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(PV1000[newpv1000size]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (pv1000lo == pv1000hi)
    newpv1000size = pv1000lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (pv1000hi - pv1000lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(PV1000[i + pv1000lo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newpv1000size = sizeROM.toInt() + pv1000lo;
    if (newpv1000size > pv1000hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(PV1000[newpv1000size]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newpv1000size);
  pv1000size = newpv1000size;
}

void checkStatus_PV1000()
{
  EEPROM_readAnything(8, pv1000size);
  if (pv1000size > pv1000hi) {
    pv1000size = 0; // default 8K
    EEPROM_writeAnything(8, pv1000size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("CASIO PV-1000"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(PV1000[pv1000size]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(PV1000[pv1000size]);
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_PV1000()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("pv1000cart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
