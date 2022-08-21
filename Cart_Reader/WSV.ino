//******************************************
// WSV MODULE
//******************************************
#ifdef enable_WSV
// Watara Supervision
// Cartridge Pinout
// 40P 2.5mm pitch connector
//
//       BACK            LABEL
//       SIDE            SIDE
//            +-------+
//       /RD -| 1  40 |- +5V
//        A0 -| 2  39 |- nc
//        A1 -| 3  38 |- nc
//        A2 -| 4  37 |- nc
//        A3 -| 5  36 |- nc
//        A4 -| 6  35 |- /WR
//        A5 -| 7  34 |- D0
//        A6 -| 8  33 |- D1
//        A7 -| 9  32 |- D2
//        A8 -| 10 31 |- D3
//        A9 -| 11 30 |- D4
//       A10 -| 12 29 |- D5
//       A11 -| 13 28 |- D6
//       A12 -| 14 27 |- D7
//       A13 -| 15 26 |- nc
//       A14 -| 16 25 |- nc
//       A15 -| 17 24 |- L1
//       A16 -| 18 23 |- L2
//        L3 -| 19 22 |- GND
//        L0 -| 20 21 |- PWR GND
//            +-------+
//
// L3 - L0 are the Link Port's I/O - only the 'MAGNUM' variant
// routed these to the cartridge slot as additional banking bits.
//
// CONTROL PINS:
// /WR - (PH5)
// /RD - (PH6)

word WSV[] = {32, 64, 512};
byte wsvlo = 0; // Lowest Entry
byte wsvhi = 2; // Highest Entry

byte wsvsize;
byte newwsvsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
// SETUP
//******************************************

void setup_WSV()
{
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7 (BA5-BA7 UNUSED)
  DDRL = 0xFF;
  //PA0-PA7 (UNUSED)
  DDRA = 0xFF;

  // Set Data Pins (D0-D7) to Input
  // D0 - D7
  DDRC = 0x00;

  // Set Control Pins to Output
  //       WR(PH5)    RD(PH6)
  //  DDRH |= (1 << 5) | (1 << 6);
  //      ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)   /RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Switch WR(PH5) to HIGH
  //  PORTH |= (1 << 5);
  // Switch RD(PH6) to LOW
  //  PORTH &= ~(1 << 6);
  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   /WR(PH5)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Switch RD(PH6) to LOW
  PORTH &= ~(1 << 6);

  // Set Unused Pins HIGH
  PORTL = 0xE0;
  PORTA = 0xFF;
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_WSV();
  strcpy(romName, "SUPERVISION");

  mode = mode_WSV;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char wsvMenuItem1[] PROGMEM = "Select Cart";
static const char wsvMenuItem2[] PROGMEM = "Read ROM";
static const char wsvMenuItem3[] PROGMEM = "Set Size";
static const char wsvMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsSV[] PROGMEM = {wsvMenuItem1, wsvMenuItem2, wsvMenuItem3, wsvMenuItem4};

void wsvMenu()
{
  convertPgm(menuOptionsSV, 4);
  uint8_t mainMenu = question_box(F("SUPERVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_WSV();
      setup_WSV();
      break;

    case 1:
      // Read Rom
      sd.chdir("/");
      readROM_WSV();
      sd.chdir("/");
      break;

    case 2:
      setROMSize_WSV();
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  LOW LEVEL FUNCTIONS
//******************************************

// WRITE
void controlOut_WSV() {
  // Switch RD(PH6) to HIGH
  PORTH |= (1 << 6);
  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);
}

// READ
void controlIn_WSV() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch RD(PH6) to LOW
  PORTH &= ~(1 << 6);
}

void dataIn_WSV()
{
  DDRC = 0x00;
}

void dataOut_WSV()
{
  DDRC = 0xFF;
}

uint8_t readByte_WSV(uint32_t addr)
{
  PORTF = addr & 0xFF;
  PORTK = (addr >> 8) & 0xFF;
  PORTL = (addr >> 16) & 0xFF;

  // Wait for data bus
  // 6 x 62.5ns = 375ns
  NOP; NOP; NOP; NOP; NOP; NOP;

  uint8_t ret = PINC;
  NOP;

  return ret;
}

//******************************************
// READ CODE
//******************************************

void readROM_WSV()
{
  strcpy(fileName, romName);
  strcat(fileName, ".sv");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  //sprintf(folder, "WSV/ROM/%s/%d", romName, foldern);
  sprintf(folder, "WSV/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_Error(F("Can't create file on SD"), true);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  // start reading rom
  dataIn_WSV();
  controlIn_WSV();

  romSize = WSV[wsvsize];

  uint32_t romStart = 0;
  if (romSize < 64)
    romStart = 0x8000;
  uint32_t romEnd = (uint32_t)romSize * 0x400;
  for (uint32_t addr = 0; addr < romEnd; addr += 512)
  {
    for (uint16_t w = 0; w < 512; w++)
      sdBuffer[w] = readByte_WSV(romStart + addr + w);
    myFile.write(sdBuffer, 512);
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("wsv.txt", 0, 1, 0);

  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_WSV()
{
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (wsvlo == wsvhi)
    newwsvsize = wsvlo;
  else {
    int b = 0;
    int i = wsvlo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(WSV[i]);
    println_Msg(F(""));
#if defined(enable_OLED)
    println_Msg(F("Press left to Change"));
    println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
    println_Msg(F("Rotate to Change"));
    println_Msg(F("Press to Select"));
#endif
    display_Update();

    while (1) {
      b = checkButton();
      if (b == 2) { // Previous (doubleclick)
        if (i == wsvlo)
          i = wsvhi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(WSV[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to Change"));
        println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to Change"));
        println_Msg(F("Press to Select"));
#endif
        display_Update();
      }
      if (b == 1) { // Next (press)
        if (i == wsvhi)
          i = wsvlo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(WSV[i]);
        println_Msg(F(""));
#if defined(enable_OLED)
        println_Msg(F("Press left to Change"));
        println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
        println_Msg(F("Rotate to Change"));
        println_Msg(F("Press to Select"));
#endif
        display_Update();
      }
      if (b == 3) { // Long Press - Execute (hold)
        newwsvsize = i;
        break;
      }
    }
    display.setCursor(0, 56); // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(WSV[newwsvsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (wsvlo == wsvhi)
    newwsvsize = wsvlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (wsvhi - wsvlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(WSV[i + wsvlo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newwsvsize = sizeROM.toInt() + wsvlo;
    if (newwsvsize > wsvhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(WSV[newwsvsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newwsvsize);
  wsvsize = newwsvsize;
}

void checkStatus_WSV()
{
  EEPROM_readAnything(8, wsvsize);
  if (wsvsize > 2) {
    wsvsize = 1; // default 64K
    EEPROM_writeAnything(8, wsvsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("WATARA SUPERVISION"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("ROM SIZE: "));
  print_Msg(WSV[wsvsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(WSV[wsvsize]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************
void setCart_WSV() {
  char gamename[100];
  char tempStr2[2];
  char crc_search[9];

  //go to root
  sd.chdir();

  // Select starting letter
  byte myLetter = starting_letter();

  // Open database
  if (myFile.open("wsv.txt", O_READ)) {
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
      for (byte count_newline = 0; count_newline < 2; count_newline++) {
        while (1) {
          if (myFile.curPosition() == 0) {
            break;
          }
          else if (myFile.peek() == '\n') {
            myFile.seekSet(myFile.curPosition() - 1);
            break;
          }
          else {
            myFile.seekSet(myFile.curPosition() - 1);
          }
        }
      }
      if (myFile.curPosition() != 0)
        myFile.seekSet(myFile.curPosition() + 2);
    }

    // Display database
    while (myFile.available()) {
      display_Clear();

      // Read game name
#if defined(enable_OLED)
      get_line(gamename, &myFile, 42);
#else
      get_line(gamename, &myFile, 96);
#endif

      // Read CRC32 checksum
      sprintf(checksumStr, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(checksumStr, tempStr2);
      }

      // Skip over semicolon
      myFile.seekSet(myFile.curPosition() + 1);

      // Read CRC32 of first 512 bytes
      sprintf(crc_search, "%c", myFile.read());
      for (byte i = 0; i < 7; i++) {
        sprintf(tempStr2, "%c", myFile.read());
        strcat(crc_search, tempStr2);
      }

      // Skip over semicolon
      myFile.seekSet(myFile.curPosition() + 1);

      // Read rom size
      // Read the next ascii character and subtract 48 to convert to decimal
      cartSize = myFile.read() - 48;

      // Remove leading 0 for single digit cart sizes
      if (cartSize != 0) {
        cartSize = cartSize * 10 +  myFile.read() - 48;
      }
      else {
        cartSize = myFile.read() - 48;
      }

      // Skip rest of line
      myFile.seekSet(myFile.curPosition() + 2);

      // Skip every 3rd line
      skip_line(&myFile);

      println_Msg(F("Select your cartridge"));
      println_Msg(F(""));
      println_Msg(gamename);
      print_Msg(F("Size: "));
      if (cartSize == 51)
        print_Msg(F("512"));
      else
        print_Msg(cartSize);
      println_Msg(F("KB"));
      println_Msg(F(""));
#if defined(enable_OLED)
      println_Msg(F("Press left to Change"));
      println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
      println_Msg(F("Rotate to Change"));
      println_Msg(F("Press to Select"));
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
          for (byte count_newline = 0; count_newline < 7; count_newline++) {
            while (1) {
              if (myFile.curPosition() == 0) {
                break;
              }
              else if (myFile.peek() == '\n') {
                myFile.seekSet(myFile.curPosition() - 1);
                break;
              }
              else {
                myFile.seekSet(myFile.curPosition() - 1);
              }
            }
          }
          if (myFile.curPosition() != 0)
            myFile.seekSet(myFile.curPosition() + 2);
          break;
        }

        // Selection
        else if (b == 3) {
          //word WSV[] = {32,64,512};
          switch (cartSize) {
            case 32:
              wsvsize = 0;
              break;

            case 64:
              wsvsize = 1;
              break;

            case 51:
              wsvsize = 2;
              break;
          }
          EEPROM_writeAnything(8, wsvsize);
          myFile.close();
          break;
        }
      }
    }
  }
  else {
    print_Error(F("Database file not found"), true);
  }
}
#endif
