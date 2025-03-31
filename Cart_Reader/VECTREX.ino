//******************************************
// VECTREX MODULE
//******************************************
#ifdef ENABLE_VECTREX
// Vectrex
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//                RIGHT
//              +-------+
//         +5V -|  2  1 |- /HALT
//         +5V -|  4  3 |- A7
//          A8 -|  6  5 |- A6
//          A9 -|  8  7 |- A5     B
//    L    A11 -| 10  9 |- A4     O
//    A    /OE -| 12 11 |- A3     T
//    B    A10 -| 14 13 |- A2     T
//    E    A15 -| 16 15 |- A1     O
//    L     D7 -| 18 17 |- A0     M
//          D6 -| 20 19 |- D0
//    S     D5 -| 22 21 |- D1     S
//    I     D4 -| 24 23 |- D2     I
//    D     D3 -| 26 25 |- GND    D
//    E    GND -| 28 27 |- GND    E
//         R/W -| 30 29 |- A12
//       /CART -| 32 31 |- A13
//        /NMI -| 34 33 |- A14
//        /IRQ -| 36 35 |- PB6
//              +-------+
//                LEFT
//
//                                                LABEL SIDE
//
//        /IRQ /NMI /CART R/W  GND   D3   D4   D5   D6   D7  A15  A10  /OE  A11   A9   A8  +5V  +5V
//       +-------------------------------------------------------------------------------------------+
//       |  36   34   32   30   28   26   24   22   20   18   16   14   12   10    8    6    4    2  |
// LEFT  |                                                                                           | RIGHT
//       |  35   33   31   29   27   25   23   21   19   17   15   13   11    9    7    5    3    1  |
//       +-------------------------------------------------------------------------------------------+
//         PB6  A14  A13  A12  GND  GND   D2   D1   D0   A0   A1   A2   A3   A4   A5   A6   A7  /HALT
//
//                                                BOTTOM SIDE

// CONTROL PINS:
// /OE(PH1)   - SNES CPUCLK
// /CART(PH3) - SNES /CS
// PB6(PH5)   - SNES /WR
// R/W(PH6)   - SNES /RD

//******************************************
// DEFINES
//******************************************
#define CLK_ENABLE PORTH |= (1 << 1)    // /E HIGH
#define CLK_DISABLE PORTH &= ~(1 << 1)  // /E LOW
#define PB6_ENABLE PORTH |= (1 << 5)    // PB6 HIGH
#define PB6_DISABLE PORTH &= ~(1 << 5)  // PB6 LOW

//******************************************
// VARIABLES
//******************************************
byte VECTREX[] = { 4, 8, 12, 16, 32, 64 };
byte vectrexlo = 0;  // Lowest Entry
byte vectrexhi = 5;  // Highest Entry
byte vectrexsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsVECTREX[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void vectrexMenu()
{
  convertPgm(menuOptionsVECTREX, 4);
  uint8_t mainMenu = question_box(F("VECTREX MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_VECTREX();
      setup_VECTREX();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_VECTREX();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_VECTREX();
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

void setup_VECTREX()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Vectrex uses A0-A15 [A16-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   /OE(PH1)  /CART(PH3)  ---(PH4)  PB6(PH5)   R/W(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   /OE(PH1)  /CART(PH3)  ---(PH4)  PB6(PH5)   R/W(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  // Set /CART LOW
  PORTH &= ~(1 << 3);  // Enable Cart
  // Set /OE LOW
  PORTH &= ~(1 << 1);  // Output Enable

  checkStatus_VECTREX();
  strcpy(romName, "VECTREX");

  mode = CORE_VECTREX;
}

//******************************************
// READ CODE
//******************************************

uint8_t readData_VECTREX(uint16_t addr) // Add Input Pullup
{
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTC = 0xFF;  // Input Pullup
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;  // Added delay for better read
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  return ret;
}

void readSegment_VECTREX(uint16_t startaddr, uint16_t endaddr)
{
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_VECTREX(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_VECTREX()
{
  createFolderAndOpenFile("VECTREX", "ROM", romName, "vec");

  PB6_DISABLE;  // PB6 LOW - Switch Bank
  
  // Standard Carts 4K/8K
  readSegment_VECTREX(0x0000, 0x1000);  // 4K
  if (vectrexsize > 0) {
    readSegment_VECTREX(0x1000, 0x2000);  // +4K = 8K
    // 12K (Dark Tower)
    if (vectrexsize > 1)
      readSegment_VECTREX(0x2000, 0x3000);  // +4K = 12K
    // 16K Carts
    if (vectrexsize > 2)
      readSegment_VECTREX(0x3000, 0x4000);  // +4K = 16K
    // Oversize 32K Carts
    if (vectrexsize > 3)
      readSegment_VECTREX(0x4000, 0x8000);  // +16K = 32K
    // Oversize 64K Carts
    if (vectrexsize > 4) {
      PB6_ENABLE;                           // PB6 HIGH - Switch Bank
      readSegment_VECTREX(0x0000, 0x8000);  // +32K = 64K
      PB6_DISABLE;                          // Reset PB6
    }
  }
  myFile.close();

  printCRC(fileName, NULL, 0);

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
void printRomSize_VECTREX(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(VECTREX[index]);
}
#endif

void setROMSize_VECTREX()
{
  byte newvectrexsize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (vectrexlo == vectrexhi)
    newvectrexsize = vectrexlo;
  else {
    newvectrexsize = navigateMenu(vectrexlo, vectrexhi, &printRomSize_VECTREX);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(VECTREX[newvectrexsize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (vectrexlo == vectrexhi)
    newvectrexsize = vectrexlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (vectrexhi - vectrexlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(VECTREX[i + vectrexlo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newvectrexsize = sizeROM.toInt() + vectrexlo;
    if (newvectrexsize > vectrexhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(VECTREX[newvectrexsize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newvectrexsize);
  vectrexsize = newvectrexsize;
}

void checkStatus_VECTREX()
{
  EEPROM_readAnything(8, vectrexsize);
  if (vectrexsize > vectrexhi) {
    vectrexsize = 0;  // default 4KB
    EEPROM_writeAnything(8, vectrexsize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("VECTREX READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(VECTREX[vectrexsize]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(VECTREX[vectrexsize]);
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_VECTREX()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("vectrexcart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
