//******************************************
// RCA STUDIO II MODULE
//******************************************
#ifdef ENABLE_RCA
// RCA Studio II
// Cartridge Pinout
// 22P 3.96mm pitch connector
//
//        FRONT
//               +-------+
//           D7 -| 1     |
//           D6 -| 2     |
//           D5 -| 3     |
//           D4 -| 4     |
//           D3 -| 5     |
//  ROM_DISABLE -| 6     |
//          GND -| 7     |
//           D2 -| 8     |
//           D1 -| 9     |
//           D0 -| 10    |
//           A0 -| 11    |
//           A1 -| 12    |
//           A2 -| 13    |
//           A3 -| 14    |
//     VCC(+5V) -| 15    |
//           A4 -| 16    |
//           A5 -| 17    |
//           A6 -| 18    |
//          TPA -| 19    |
//           A7 -| 20    |
//         /MRD -| 21    |
//        ROMCS -| 22    |
//               +-------+
//
//                                                BACK SIDE
//       +-------------------------------------------------------------------------------------------+
// LEFT  |                                                                                           | RIGHT
//       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20   21  22  |
//       +-------------------------------------------------------------------------------------------+
//          D7  D6  D5  D4  D3 DIS GND  D2  D1  D0  A0  A1  A2  A3 +5V  A4  A5  A6 TPA  A7 /MRD  CS  
//
//                                                FRONT SIDE

// CONTROL PINS:
// /MRD(PH3) - SNES /CS
// TPA(PH6)  - SNES /RD

//******************************************
// VARIABLES
//******************************************
byte RCA[] = {1,2};
byte rcalo = 0; // Lowest Entry
byte rcahi = 1; // Highest Entry

byte rcasize;
byte newrcasize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// MENU
//******************************************
// Base Menu
static const char* const menuOptionsRCA[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void rcaMenu()
{
  convertPgm(menuOptionsRCA, 4);
  uint8_t mainMenu = question_box(F("RCA STUDIO II MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_RCA();
      setup_RCA();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_RCA();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_RCA();
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

void setup_RCA()
{
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // RCA Studio II uses A0-A7 [A8-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output 
  //       ---(PH0)   ---(PH1)  /MRD(PH3)   ---(PH4)   ---(PH5)   TPA(PH6)
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /MRD(PH3)   ---(PH4)   ---(PH5)   TPA(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTK = 0xFF; // A8-A15
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_RCA();
  strcpy(romName, "RCA");

  mode = CORE_RCA;
}

//******************************************
// READ FUNCTIONS
//******************************************

uint8_t readData_RCA(uint16_t addr)
{
  // Setup TPA + /MRD
  PORTH &= ~(1 << 6); // TPA LOW
  delayMicroseconds(4);
  PORTH |= (1 << 3); // /MRD HIGH;
  delayMicroseconds(4);

  // Set HIGH Address
  PORTF = (addr >> 8) & 0xFF;
  delayMicroseconds(4);

  // Latch HIGH Address
  PORTH |= (1 << 6); // TPA HIGH
  delayMicroseconds(4);
  PORTH &= ~(1 << 3); // /MRD LOW
  delayMicroseconds(4);

  // Switch TPA LOW
  PORTH &= ~(1 << 6); // TPA LOW
  delayMicroseconds(4);

  // Set LOW Address
  PORTF = addr & 0xFF;
  delayMicroseconds(4);
  uint8_t ret = PINC;
  // Reset /MRD
  PORTH |= (1 << 3); // /MRD HIGH;

  return ret;
}

void readSegment_RCA(uint16_t startaddr, uint16_t endaddr)
{
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_RCA(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_RCA() 
{
  createFolderAndOpenFile("RCA", "ROM", romName, "bin");

  readSegment_RCA(0x0400,0x0600); // 512B
  if (rcasize > 0)
    readSegment_RCA(0x0600,0x0800); // +512B = 1K
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
void printRomSize_RCA(int index)
{
    display_Clear();
    print_Msg(FS(FSTRING_ROM_SIZE));
    println_Msg(RCA[index]);
}
#endif

void setROMSize_RCA()
{
  byte newrcasize;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (rcalo == rcahi)
    newrcasize = rcalo;
  else {
    newrcasize = navigateMenu(rcalo, rcahi, &printRomSize_RCA);
    
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(RCA[newrcasize]);
  println_Msg(F("KB"));
  display_Update();
  delay(1000);
#else
  if (rcalo == rcahi)
    newrcasize = rcalo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (rcahi - rcalo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(RCA[i + rcalo]);
      Serial.println(F("KB"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newrcasize = sizeROM.toInt() + rcalo;
    if (newrcasize > rcahi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(RCA[newrcasize]);
  Serial.println(F("KB"));
#endif
  EEPROM_writeAnything(8, newrcasize);
  rcasize = newrcasize;
}

void checkStatus_RCA()
{
  EEPROM_readAnything(8, rcasize);
  if (rcasize > rcahi)  {
    rcasize = 1; // default 1024B
    EEPROM_writeAnything(8, rcasize);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("RCA STUDIO II"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(RCA[rcasize] * 512);
  println_Msg(F("B"));
  display_Update();
  wait();
#else
  Serial.print(FS(FSTRING_ROM_SIZE));
  Serial.print(RCA[rcasize] * 512);
  Serial.println(F("B"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

void setCart_RCA()
{
  //go to root
  sd.chdir();

  byte gameSize;

  // Select starting letter
  //byte myLetter = starting_letter();

  // Open database
  if (myFile.open("rcacart.txt", O_READ)) {
    // seek_first_letter_in_database(myFile, myLetter);

    if(checkCartSelection(myFile, &readDataLineSingleDigit, &gameSize)) {
      EEPROM_writeAnything(8, gameSize);
    }
  } else {
    print_FatalError(FS(FSTRING_DATABASE_FILE_NOT_FOUND));
  }
}
#endif