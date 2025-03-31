//******************************************
// BALLY ASTROCADE MODULE
//******************************************
#ifdef ENABLE_BALLY
// Bally Astrocade
// Cartridge Pinout
// 26P 2.54mm pitch connector
//
//    BOTTOM
//            +-------+
//       GND -| 1     |
//        A7 -| 2     |
//        A6 -| 3     |
//        A5 -| 4     |
//        A4 -| 5     |
//        A3 -| 6     |
//        A2 -| 7     |
//        A1 -| 8     |
//        A0 -| 9     |
//        D0 -| 10    |
//        D1 -| 11    |
//        D2 -| 12    |
//       GND -| 13    |
//        D3 -| 14    |
//        D4 -| 15    |
//        D5 -| 16    |
//        D6 -| 17    |
//        D7 -| 18    |
//       A11 -| 19    |
//       A10 -| 20    |
//   /ENABLE -| 21    |
//       A12 -| 22    |
//        A9 -| 23    |
//        A8 -| 24    |
//  VCC(+5V) -| 25    |
//       GND -| 26    |
//            +-------+
//
//                                                         TOP SIDE
//       +-----------------------------------------------------------------------------------------------------------+
// LEFT  |                                                                                                           | RIGHT
//       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18   19   20  21  22  23  24  25  26 |
//       +-----------------------------------------------------------------------------------------------------------+
//         GND  A7  A6  A5  A4  A3  A2  A1  A0  D0  D1  D2 GND  D3  D4  D5  D6  D7  A11  A10 /EN A12  A9  A8 +5V GND  
//
//                                                        BOTTOM SIDE

// CONTROL PINS:
// /ENABLE(PH3) - SNES /CS

//******************************************
// VARIABLES
//******************************************

byte BALLY[] = {2,4,8};
byte ballylo = 0; // Lowest Entry
byte ballyhi = 2; // Highest Entry

byte ballysize;
byte newballysize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsBALLY[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void ballyMenu()
{
  convertPgm(menuOptionsBALLY, 4);
  uint8_t mainMenu = question_box(F("BALLY ASTROCADE MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_BALLY();
      setup_BALLY();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_BALLY();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_BALLY();
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

void setup_BALLY()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Bally Astrocade uses A0-A12 [A13-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //       ---(PH0)   ---(PH1) /ENABLE(PH3) ---(PH4)   ---(PH5)   ---(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1) /ENABLE(PH3) ---(PH4)   ---(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_BALLY();
  strcpy(romName, "BALLY");

  mode = CORE_BALLY;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_BALLY(uint16_t addr)
{
  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15

  PORTC = 0xFF; // Input Pullup
  PORTH &= ~(1 << 3); // /ENABLE LOW
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;
  PORTH |= (1 << 3); // /ENABLE HIGH

  return ret;
}

void readSegment_BALLY(uint16_t startaddr, uint16_t endaddr)
{
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_BALLY(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_BALLY() 
{
  createFolderAndOpenFile("BALLY", "ROM", romName, "bin");

  readSegment_BALLY(0x0000,0x0800); // 2K
  if (ballysize > 0) {
    readSegment_BALLY(0x0800,0x1000); // +2K = 4K
    if (ballysize > 1) {
      readSegment_BALLY(0x1000,0x2000); // +4K = 8K
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
void printRomSize_BALLY(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(BALLY[index]);
}
#endif

void setROMSize_BALLY()
{
  byte newballysize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (ballylo == ballyhi)
    newballysize = ballylo;
  else {
    newballysize = navigateMenu(ballylo, ballyhi, &printRomSize_BALLY);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(BALLY[newballysize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (ballylo == ballyhi)
    newballysize = ballylo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (ballyhi - ballylo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(BALLY[i + ballylo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newballysize = sizeROM.toInt() + ballylo;
    if (newballysize > ballyhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(BALLY[newballysize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newballysize);
  ballysize = newballysize;
}

void checkStatus_BALLY()
{
  EEPROM_readAnything(8, ballysize);
  if (ballysize > ballyhi) {
    ballysize = 0; // default 2K
    EEPROM_writeAnything(8, ballysize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("BALLY ASTROCADE"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(BALLY[ballysize]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(BALLY[ballysize]);
  Serial.println(F("KB"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_BALLY()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("ballycart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif
