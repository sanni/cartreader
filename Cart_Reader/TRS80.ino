//******************************************
// TRS-80 COLOR COMPUTER MODULE
//******************************************
#ifdef ENABLE_TRS80
// TRS-80
// Color Computer
// Cartridge Pinout
// 40P 2.54mm pitch connector
//
//      TOP            BOTTOM
//     SIDE            SIDE
//          +-------+
//      NC -|  1 2  |- NC
//   /HALT -|  3 4  |- /NMI
//  /RESET -|  5 6  |- E
//       Q -|  7 8  |- /CART
//     +5V -|  9 10 |- D0
//      D1 -| 11 12 |- D2
//      D3 -| 13 14 |- D4
//      D5 -| 15 16 |- D6
//      D7 -| 17 18 |- R/W
//      A0 -| 19 20 |- A1
//      A2 -| 21 22 |- A3
//      A4 -| 23 24 |- A5
//      A6 -| 25 26 |- A7
//      A8 -| 27 28 |- A9
//     A10 -| 29 30 |- A11
//     A12 -| 31 32 |- /CTS
//     GND -| 33 34 |- GND
//     SND -| 35 36 |- /SCS
//     A13 -| 37 38 |- A14
//     A15 -| 39 40 |- /SLENB
//          +-------+
//
//                                                       TOP
//       A15  A13  SND  GND  A12  A10   A8   A6   A4   A2   A0   D7   D5   D3   D1  +5V   Q /RST /HLT NC 
//      +-----------------------------------------------------------------------------------------------+
//      | 39   37   35   33   31   29   27   25   23   21   19   17   15   13   11    9   7   5   3   1 |
// LEFT |                                                                                               | RIGHT
//      | 40   38   36   34   32   30   28   26   24   22   20   18   16   14   12   10   8   6   4   2 |
//      +-----------------------------------------------------------------------------------------------+
//       /SLB A14 /SCS  GND /CTS  A11   A9   A7   A5   A3   A1   RW   D6   D4   D2   D0 /CRT  E  /NMI NC
//                                                      BOTTOM

// CONTROL PINS:
// /RESET(PH0) - SNES RESET
// E(PH1)      - SNES CPUCLK
// /CTS(PH3)   - SNES /CS
// /SCS(PH4)   - SNES /IRQ
// R/W(PH5)    - SNES /WR
// NOTE:  CARTS CONNECT /CART TO Q

//******************************************
// VARIABLES
//******************************************
byte TRS80[] = {2,4,8,10,16,32,64,128};
byte trs80lo = 0; // Lowest Entry
byte trs80hi = 7; // Highest Entry

byte trs80size;
byte newtrs80size;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsTRS80[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void trs80Menu()
{
  convertPgm(menuOptionsTRS80, 4);
  uint8_t mainMenu = question_box(F("TRS-80 MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_TRS80();
      setup_TRS80();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_TRS80();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_TRS80();
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

void setup_TRS80()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // TRS-80 uses A0-A15 [A16-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //      /RST(PH0)    E(PH1)   /CTS(PH3)  /SCS(PH4)   R/W(PH5)   ---(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //      /RST(PH0)    E(PH1)   /CTS(PH3)  /SCS(PH4)   R/W(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_TRS80();
  strcpy(romName, "TRS80");

  mode = CORE_TRS80;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_TRS80(uint16_t addr)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15

  // Set /CTS to LOW
  PORTH &= ~(1 << 3); // /CTS LOW
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Pull /CTS to HIGH
  PORTH |= (1 << 3); // /CTS HIGH

  return ret;
}

void readSegment_TRS80(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_TRS80(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// BANKSWITCH
//******************************************

// Bankswitch is a combination of IC1 74LS175 (Quad Latch) and IC3 74LS10 (NAND Gate)
// IC1 latches D0-D3 using /RESET and control (CP) from IC3
// IC3 controls latch into IC1 using CP output based on R/W, E and /SCS
// IC3 CP LOW only when R/W LOW, /SCS LOW, and E HIGH
void bankSwitch_TRS80(uint16_t addr, uint8_t data)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF; // Set to Output
  PORTC = data;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // SET CP (LATCH INPUT INTO 74LS175) TO LOW
  PORTH &= ~(1 << 4); // /SCS(PH4) LOW
  PORTH &= ~(1 << 5); // R/W(PH5) LOW = WRITE
  // Pulse E to Latch Data
  PORTH &= ~(1 << 1); // E(PH1) LOW
  NOP;
  PORTH |= (1 << 1); // E(PH1) HIGH
  NOP;

  // SET CP TO HIGH
  PORTH |= (1 << 4); // /SCS(PH5) HIGH
  PORTH |= (1 << 5); // R/W(PH5) HIGH = READ

  DDRC = 0x00; // Reset to Input
}

//******************************************
// READ ROM
//******************************************

void readROM_TRS80() 
{
  createFolderAndOpenFile("TRS80", "ROM", romName, "ccc");

  // Set /RESET to LOW
  PORTH &= ~(1 << 0); // /RESET LOW
  delay(100);
  // Set /RESET to HIGH
  PORTH |= (1 << 0); // /RESET HIGH
  // Set R/W to READ
  PORTH |= (1 << 5); // R/W HIGH  

  if (trs80size > 5) { // Bankswitch Carts - Predator 64K/Robocop 128K
    // Predator 64K = (2^6) / 16 = 4
    // Robocop 128K = (2^7) / 16 = 8
    int banks = (int_pow(2, trs80size)) / 16;
    for (int x = 0; x < banks; x++) {
      bankSwitch_TRS80(0xFF40, x); // Bankswitch
      readSegment_TRS80(0xC000, 0x10000); // 16K * 8 = 128K
    }
  }
  else { // Normal Carts 2K/4K/8K/10K/16K/32K
    readSegment_TRS80(0xC000,0xC800); // 2K
    if (trs80size > 0) {
      readSegment_TRS80(0xC800,0xD000); // +2K = 4K
      if (trs80size > 1) {
        readSegment_TRS80(0xD000,0xE000); // +4K = 8K
        if (trs80size > 2) {
          readSegment_TRS80(0xE000,0xE800); // +2K = 10K
          if (trs80size > 3) {
            readSegment_TRS80(0xE800,0x10000); // +6K = 16K
            if (trs80size == 5) { // 32K
              // Second Chip Select - Switch to Upper 16K (Mind-Roll)
              PORTH &= ~(1 << 4); // /SCS LOW
              NOP; NOP; NOP; NOP; NOP;
              PORTH |= (1 << 4); // /SCS HIGH
              readSegment_TRS80(0x8000,0xC000); // +16K = 32K
            }
          }
        }
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
void printRomSize_TRS80(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(TRS80[index]);
}
#endif

void setROMSize_TRS80()
{
  byte newtrs80size;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (trs80lo == trs80hi)
    newtrs80size = trs80lo;
  else {
    newtrs80size = navigateMenu(trs80lo, trs80hi, &printRomSize_TRS80);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(TRS80[newtrs80size]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (trs80lo == trs80hi)
    newtrs80size = trs80lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (trs80hi - trs80lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(TRS80[i + trs80lo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newtrs80size = sizeROM.toInt() + trs80lo;
    if (newtrs80size > trs80hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(TRS80[newtrs80size]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newtrs80size);
  trs80size = newtrs80size;
}

void checkStatus_TRS80()
{
  EEPROM_readAnything(8, trs80size);
  if (trs80size > trs80hi) {
    trs80size = 5; // default 32K
    EEPROM_writeAnything(8, trs80size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("TRS-80 COLOR COMPUTER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(TRS80[trs80size]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(TRS80[trs80size];
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_TRS80()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("trs80cart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
