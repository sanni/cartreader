//******************************************
// COLECOVISION MODULE
//******************************************
#ifdef enable_COLV

// Coleco Colecovision
// Cartridge Pinout
// 30P 2.54mm pitch connector
//
//  FRONT SIDE             BACK SIDE
//       (EVEN)            (ODD)
//              +-------+
//       /C000 -| 2   1 |- D2
//          D3 -| 4   3 |- D1
//          D4 -| 6   5 |- D0
//          D5 -| 8   7 |- A0
//          D6 -| 10  9 |- A1
//          D7 -| 12 11 |- A2
//         A11 -| 14 13 |- GND (SHLD)
//         A10 -| 16 15 |- A3
//       /8000 -| 18 17 |- A4
//         A14 -| 20 19 |- A13
//       /A000 -| 22 21 |- A5
//         A12 -| 24 23 |- A6
//          A9 -| 26 25 |- A7
//          A8 -| 28 27 |- /E000
//    VCC(+5V) -| 30 29 |- GND
//              +-------+

// CONTROL PINS:
// CHIP SELECT PINS
// /8000(PH3) - CHIP 0 - SNES /CS
// /A000(PH4) - CHIP 1 - SNES /IRQ
// /C000(PH5) - CHIP 2 - SNES /WR
// /E000(PH6) - CHIP 3 - SNES /RD

const byte COL[] PROGMEM = { 8, 12, 16, 20, 24, 32 };
byte collo = 0;  // Lowest Entry
byte colhi = 5;  // Highest Entry

byte colsize;
byte newcolsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char colMenuItem1[] PROGMEM = "Select Cart";
static const char colMenuItem2[] PROGMEM = "Read ROM";
static const char colMenuItem3[] PROGMEM = "Set Size";
//static const char colMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsCOL[] PROGMEM = { colMenuItem1, colMenuItem2, colMenuItem3, string_reset2 };

void setup_COL() {
  // Set Address Pins to Output
  // Colecovision uses A0-A14 [A15-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)  /8000(PH3) /A000(PH4) /C000(PH5) /E000(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /8000(PH3) /A000(PH4) /C000(PH5) /E000(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_COL();
  strcpy(romName, "COLECO");

  mode = mode_COL;
}

void colMenu() {
  vselect(false);
  convertPgm(menuOptionsCOL, 4);
  uint8_t mainMenu = question_box(F("COLECOVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_COL();
      setup_COL();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_COL();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_COL();
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
// CHIP SELECT CONTROL PINS
// /8000(PH3) - CHIP 0
// /A000(PH4) - CHIP 1
// /C000(PH5) - CHIP 2
// /E000(PH6) - CHIP 3

uint8_t readData_COL(uint32_t addr) {
  // SELECT ROM CHIP - PULL /CE LOW
  uint8_t chipdecode = ((addr >> 13) & 0x3);
  if (chipdecode == 3)       // CHIP 3
    PORTH &= ~(1 << 6);      // /E000 LOW (ENABLE)
  else if (chipdecode == 2)  // CHIP 2
    PORTH &= ~(1 << 5);      // /C000 LOW (ENABLE)
  else if (chipdecode == 1)  // CHIP 1
    PORTH &= ~(1 << 4);      // /A000 LOW (ENABLE)
  else                       // CHIP 0
    PORTH &= ~(1 << 3);      // /8000 LOW (ENABLE)

  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A15

  // LATCH ADDRESS - PULL /CE HIGH
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);  // ALL /CE HIGH (DISABLE)

  uint8_t ret = PINC;
  return ret;
}

void readSegment_COL(uint32_t startaddr, uint32_t endaddr) {
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_COL(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readROM_COL() {
  strcpy(fileName, romName);
  strcat(fileName, ".col");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  //  sprintf(folder, "COL/ROM/%s/%d", romName, foldern);
  sprintf(folder, "COL/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(create_file_STR);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  // RESET ALL CS PINS HIGH (DISABLE)
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  readSegment_COL(0x8000, 0xA000);  // 8K
  if (colsize > 0) {
    readSegment_COL(0xA000, 0xB000);  // +4K = 12K
    if (colsize > 1) {
      readSegment_COL(0xB000, 0xC000);  // +4K = 16K
      if (colsize > 2) {
        readSegment_COL(0xC000, 0xD000);  // +4K = 20K
        if (colsize > 3) {
          readSegment_COL(0xD000, 0xE000);  // +4K = 24K
          if (colsize > 4) {
            readSegment_COL(0xE000, 0x10000);  // +8K = 32K
          }
        }
      }
    }
  }
  myFile.close();

  // RESET ALL CS PINS HIGH (DISABLE)
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Compare CRC32 to database and rename ROM if found
  compareCRC("colv.txt", 0, 1, 0);

  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_COL() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (collo == colhi)
    newcolsize = collo;
  else {
    int b = 0;
    int i = collo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(pgm_read_byte(&(COL[i])));
    println_Msg(F(""));
#if defined(enable_OLED)
    print_STR(press_to_change_STR, 1);
    print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
    print_STR(rotate_to_change_STR, 1);
    print_STR(press_to_select_STR, 1);
#endif
    display_Update();

    while (1) {
      b = checkButton();
      if (b == 2) {  // Previous (doubleclick)
        if (i == collo)
          i = colhi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(pgm_read_byte(&(COL[i])));
        println_Msg(F(""));
#if defined(enable_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 1) {  // Next (press)
        if (i == colhi)
          i = collo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(pgm_read_byte(&(COL[i])));
        println_Msg(F(""));
#if defined(enable_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 3) {  // Long Press - Execute (hold)
        newcolsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(pgm_read_byte(&(COL[newcolsize])));
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (collo == colhi)
    newcolsize = collo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (colhi - collo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(pgm_read_byte(&(COL[i + collo])));
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newcolsize = sizeROM.toInt() + collo;
    if (newcolsize > colhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(pgm_read_byte(&(COL[newcolsize])));
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newcolsize);
  colsize = newcolsize;
}

void checkStatus_COL() {
  EEPROM_readAnything(8, colsize);
  if (colsize > 5) {
    colsize = 0;
    EEPROM_writeAnything(8, colsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("COLECOVISION READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("ROM SIZE: "));
  print_Msg(pgm_read_byte(&(COL[colsize])));
  println_Msg(F("K"));
  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(pgm_read_byte(&(COL[colsize])));
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_COL() {
  char gamename[100];
  char tempStr2[2];
  char crc_search[9];

  //go to root
  sd.chdir();

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("colv.txt", O_READ)) {
    // Skip ahead to selected starting letter
    if ((myLetter > 0) && (myLetter <= 26)) {
      while (myFile.available()) {
        // Read current name
        get_line(gamename, &myFile, 96);

        // Compare selected letter with first letter of current name until match
        while (gamename[0] != 64 + myLetter) {
          skip_line(&myFile);
          skip_line(&myFile);
          get_line(gamename, &myFile, 96);
        }
        break;
      }

      // Rewind one line
      rewind_line(myFile);
    }

    // Display database
    while (myFile.available()) {
      display_Clear();

      // Read game name
      get_line(gamename, &myFile, 96);

      // Read CRC32 checksum
      sprintf(checksumStr, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(checksumStr, tempStr2);
      }

      // Skip over semicolon
      myFile.seekCur(1);

      // Read CRC32 of first 512 bytes
      sprintf(crc_search, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(crc_search, tempStr2);
      }

      // Skip over semicolon
      myFile.seekCur(1);

      // Read rom size
      // Read the next ascii character and subtract 48 to convert to decimal
      cartSize = myFile.read() - 48;

      // Remove leading 0 for single digit cart sizes
      if (cartSize != 0) {
        cartSize = cartSize * 10 + myFile.read() - 48;
      } else {
        cartSize = myFile.read() - 48;
      }

      // Skip rest of line
      myFile.seekCur(2);

      // Skip every 3rd line
      skip_line(&myFile);

      println_Msg(F("Select your cartridge"));
      println_Msg(F(""));
      println_Msg(gamename);
      print_Msg(F("Size: "));
      print_Msg(cartSize);
      println_Msg(F("KB"));
      println_Msg(F(""));
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#elif defined(SERIAL_MONITOR)
      println_Msg(F("U/D to Change"));
      println_Msg(F("Space to Select"));
#endif
      display_Update();

      int b = 0;
      while (1) {
        // Check button input
        b = checkButton();

        // Next
        if (b == 1) {
          break;
        }

        // Previous
        else if (b == 2) {
          rewind_line(myFile, 6);
          break;
        }

        // Selection
        else if (b == 3) {
          //byte COL[] = {8, 12, 16, 20, 24, 32};
          switch (cartSize) {
            case 8:
              colsize = 0;
              break;

            case 12:
              colsize = 1;
              break;

            case 16:
              colsize = 2;
              break;

            case 20:
              colsize = 3;
              break;

            case 24:
              colsize = 4;
              break;

            case 32:
              colsize = 5;
              break;

            default:
              colsize = 0;
              break;
          }
          EEPROM_writeAnything(8, colsize);
          myFile.close();
          break;
        }
      }
    }
  } else {
    print_FatalError(F("Database file not found"));
  }
}
#endif
//******************************************
// End of File
//******************************************