//******************************************
// LITTLE JAMMER MODULE
//******************************************
#ifdef ENABLE_LJ
// Little Jammer
// Cartridge Pinout
// 48P 1.25mm pitch connector
// 
// FORM FACTOR IS SAME AS BANDAI WONDERSWAN/BENESSE POCKET CHALLENGE V2/LITTLE JAMMER PRO
// WIRING IS COMPLETELY DIFFERENT!
//
// LEFT SIDE
// 1 VSS (GND)
// 2 A-1
// 3 A0
// 4 A1
// 5 A2
// 6 A3
// 7 A4
// 8 A5
// 9 A6
// 10 A7
// 11 A8
// 12 A9
// 13 A10
// 14 A11
// 15 A12
// 16 A13
// 17 A14
// 18 A15
// 19 A16
// 20 A17
// 21 A18
// 22 A19
// 23 A20
// 24 VCC (+5V)
// 25 VCC (+5V)
// 26 D0
// 27 D1
// 28 D2
// 29 D3
// 30 D4
// 31 D5
// 32 D6
// 33 D7
// 34 /WE
// 35 /RESET
// 36 /CE
// 37 /OE
// 38 VSS (GND)
// 39 NC
// 40 NC
// 41 NC
// 42 NC
// 43 NC
// 44 NC
// 45 NC
// 46 NC
// 47 VSS (GND)
// 48 VSS (GND)
// RIGHT SIDE

// CONTROL PINS:
// /RESET(PH0) - SNES RESET
// /CE(PH3)    - SNES /CS
// /WE(PH5)    - SNES /WR
// /OE(PH6)    - SNES /RD

// LITTLE JAMMER DIRECT ADDRESSING
// 1 1111 1111 1111 1111 1111
// 1 F    F    F    F    F = 0x1FFFFF
// Size = 0x200000 = 2MB
//
// A20 connection on Pin 23 = 0x400000 = 4MB
// 11 1111 1111 1111 1111 1111
// 3 F    F    F    F    F = 0x3FFFFF
// Size = 0x400000 = 4MB

//******************************************
// VARIABLES
//******************************************
byte LJ[] = {1,2,4};
byte ljlo = 0; // Lowest Entry
byte ljhi = 2; // Highest Entry
byte ljsize;
byte newljsize;

boolean ljflashfound = false;
byte ljbytecheck;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsLJ[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void ljMenu()
{
  convertPgm(menuOptionsLJ, 4);
  uint8_t mainMenu = question_box(F("LITTLE JAMMER MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    // Select Cart
    case 0:
      setCart_LJ();
      setup_LJ();
      break;

    // Read ROM
    case 1:
      sd.chdir("/");
      readROM_LJ();
      sd.chdir("/");
      break;

    // Set Size
    case 2:
      setROMSize_LJ();
      break;
    
    // Reset
    case 3:
      resetArduino();
      break;
  }
}

//******************************************
// SETUP
//******************************************

void setup_LJ()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // LITTLE JAMMER uses A(-1)-A19 wired to A0-A20 [A21-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //      /RST(PH0)   ---(PH1)   /CE(PH3)   ---(PH4)   /WR(PH5)   /OE(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //      /RST(PH0)   ---(PH1)   /CE(PH3)   ---(PH4)   /WR(PH5)   /OE(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_LJ();
  strcpy(romName, "LJ");

  mode = CORE_LJ;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_LJ(uint32_t addr)
{
  PORTF = addr  & 0xFF;        // A(-1)-A6
  PORTK = (addr >> 8) & 0xFF;  // A7-A14
  PORTL = (addr >> 16) & 0xFF; // A15-A20
  NOP;
  NOP;

  // switch /CE(PH3) to LOW
  PORTH &= ~(1 << 3);
  // switch /OE(PH6) to LOW
  PORTH &= ~(1 << 6);
  NOP;
  NOP;

  uint8_t ret = PINC;

  // switch /CE(PH3) and /OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  return ret;
}

void readSegment_LJ(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_LJ(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_LJ()
{
  createFolderAndOpenFile("LJ", "ROM", romName, "bin");

  // Maximum Direct Address Size is 4MB
  readSegment_LJ(0x000000,0x100000); // 1MB
  if (ljsize > 0) // 2MB/4MB
  {
    readSegment_LJ(0x100000,0x200000); // +1MB = 2MB
    if (ljsize > 1) // 4MB
    {
      readSegment_LJ(0x200000,0x400000); // +2MB = 4MB
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
void printRomSize_LJ(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(LJ[index]);
}
#endif

void setROMSize_LJ()
{
  byte newljsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (ljlo == ljhi)
    newljsize = ljlo;
  else {
    newljsize = navigateMenu(ljlo, ljhi, &printRomSize_LJ);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LJ[newljsize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (ljlo == ljhi)
    newljsize = ljlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (ljhi - ljlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(LJ[i + ljlo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newljsize = sizeROM.toInt() + ljlo;
    if (newljsize > ljhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(LJ[newljsize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newljsize);
  ljsize = newljsize;
}

void checkStatus_LJ()
{
  EEPROM_readAnything(8, ljsize);
  if (ljsize > ljhi) {
    ljsize = 1; // default 2M
    EEPROM_writeAnything(8, ljsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("LITTLE JAMMER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(LJ[ljsize]);
  println_Msg(F("MB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(LJ[ljsize]);
  Serial.println(F("MB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_LJ()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("ljcart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
