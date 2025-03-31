//******************************************
// TOMY PYUUTA MODULE
//******************************************
#ifdef ENABLE_PYUUTA
// Tomy Pyuuta
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//       FRONT              BACK
//        SIDE              SIDE
//              +--------+
//         GND -|  2   1 |- GND
//      /RESET -|  4   3 |- D7
//        J1-6 -|  6   5 |- D6
//  A15/CRUOUT -|  8   7 |- D5
//         A13 -| 10   9 |- D4
//         A12 -| 12  11 |- D3
//         A11 -| 14  13 |- D2
//         A10 -| 16  15 |- D1
//          A9 -| 18  17 |- D0
//          A8 -| 20  19 |- VCC
//          A7 -| 22  21 |- /CS1
//          A3 -| 24  23 |- A14
//          A6 -| 26  25 |- A2
//          A5 -| 28  27 |- A1
//          A4 -| 30  29 |- /DBIN
//  /WE/CPUCLK -| 32  31 |- A0
//   /INT4 /EC -| 34  33 |- SOUND
//       CRUIN -| 36  35 |- /CS0
//              +--------+
//
//                                           BACK
//        /CS0 SND  A0 /DB  A1  A2 A14 /CS1 VCC D0  D1  D2  D3  D4  D5  D6  D7  GND
//      +----------------------------------------------------------------------------+
//      |   35  33  31  29  27  25  23  21  19  17  15  13  11   9   7   5   3   1   |
// LEFT |                                                                            | RIGHT
//      |   36  34  32  30  28  26  24  22  20  18  16  14  12  10   8   6   4   2   |
//      +----------------------------------------------------------------------------+
//        CRIN /INT /WE A4  A5  A6  A3  A7  A8  A9 A10 A11 A12 A13 A15 J1-6 /RST GND
//                                           FRONT

// CONTROL PINS:
// /RESET(PH0) - SNES RESET
// /CS0(PH3)   - SNES /CS
// /DBIN(PH4)  - SNES /IRQ
// /CS1(PH6)   - SNES /RD

// NOTE: PYUUTA ADDRESS AND DATA BUS ARE BIG-ENDIAN
// LEAST SIGNIFICANT IS BIT 7 AND MOST SIGNIFICANT IS BIT 0
// PCB ADAPTER WIRED FOR DIFFERENCE

//******************************************
// VARIABLES
//******************************************
byte PYUUTA[] = {8,16,32};
byte pyuutalo = 0; // Lowest Entry
byte pyuutahi = 2; // Highest Entry
byte pyuutasize;
byte newpyuutasize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsPYUUTA[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void pyuutaMenu()
{
  convertPgm(menuOptionsPYUUTA, 4);
  uint8_t mainMenu = question_box(F("TOMY PYUUTA MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_PYUUTA();
      setup_PYUUTA();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_PYUUTA();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_PYUUTA();
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

void setup_PYUUTA()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // PYUUTA uses A0-A15 [A16-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //      /RST(PH0)   ---(PH1)  /CS0(PH3)  /DBIN(PH4)  ---(PH5)  /CS1(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //      /RST(PH0)   ---(PH1)  /CS0(PH3)  /DBIN(PH4)  ---(PH5)  /CS1(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_PYUUTA();
  strcpy(romName, "PYUUTA");

  mode = CORE_PYUUTA;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_PYUUTA(uint16_t addr)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  return ret;
}

void readSegment_PYUUTA(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_PYUUTA(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_PYUUTA() 
{
  createFolderAndOpenFile("PYUUTA", "ROM", romName, "bin");

  //8K $8000-$9FFF
  //16K $8000-$BFFF
  //32K $4000-$BFFF
  PORTH &= ~(1 << 4); // /DBIN(PH4) LOW
  if (pyuutasize > 1) { // 32K [3D CARTS]
    PORTH &= ~(1 << 6); // /CS1(PH6) LOW
    readSegment_PYUUTA(0x4000,0x8000); // +16K = 32K
    PORTH |= (1 << 6); // /CS1(PH6) HIGH
  }
  PORTH &= ~(1 << 3); // /CS0(PH3) LOW
  readSegment_PYUUTA(0x8000,0xA000); // 8K
  PORTH |= (1 << 3); // /CS0(PH3) HIGH
  if (pyuutasize > 0) { // 16K
    PORTH &= ~(1 << 3); // /CS0(PH3) LOW
    readSegment_PYUUTA(0xA000,0xC000); // +8K = 16K
    PORTH |= (1 << 3); // /CS0(PH3) HIGH
  }
  PORTH |= (1 << 4); // /DBIN(PH4) HIGH
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
void printRomSize_PYUUTA(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(PYUUTA[index]);
}
#endif

void setROMSize_PYUUTA()
{
  byte newpyuutasize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (pyuutalo == pyuutahi)
    newpyuutasize = pyuutalo;
  else {
    newpyuutasize = navigateMenu(pyuutalo, pyuutahi, &printRomSize_PYUUTA);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(PYUUTA[newpyuutasize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (pyuutalo == pyuutahi)
    newpyuutasize = pyuutalo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (pyuutahi - pyuutalo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(PYUUTA[i + pyuutalo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newpyuutasize = sizeROM.toInt() + pyuutalo;
    if (newpyuutasize > pyuutahi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(PYUUTA[newpyuutasize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newpyuutasize);
  pyuutasize = newpyuutasize;
}

void checkStatus_PYUUTA()
{
  EEPROM_readAnything(8, pyuutasize);
  if (pyuutasize > pyuutahi) {
    pyuutasize = 0; // default 8K
    EEPROM_writeAnything(8, pyuutasize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("TOMY PYUUTA"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(PYUUTA[pyuutasize]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(PYUUTA[pyuutasize]);
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_PYUUTA()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("pyuutacart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
