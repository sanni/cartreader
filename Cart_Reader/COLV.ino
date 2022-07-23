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

byte COL[] = {8, 12, 16, 20, 24, 32};
byte collo = 0; // Lowest Entry
byte colhi = 5; // Highest Entry

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
static const char colMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsCOL[] PROGMEM = {colMenuItem1, colMenuItem2, colMenuItem3, colMenuItem4};

void setup_COL()
{
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
  DDRH |=  (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |=  (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)  /8000(PH3) /A000(PH4) /C000(PH5) /E000(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF; // A16-A23
  PORTJ |= (1 << 0); // TIME(PJ0)

  checkStatus_COL();
  strcpy(romName, "COLECO");

  mode = mode_COL;
}

void colMenu()
{
  convertPgm(menuOptionsCOL, 4);
  uint8_t mainMenu = question_box(F("COLECOVISION MENU"), menuOptions, 4, 0);

  switch (mainMenu)
  {
    case 0:
      // Select Cart
      setCart_COL();
      wait();
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

uint8_t readData_COL(uint32_t addr)
{
  // SELECT ROM CHIP - PULL /CE LOW
  uint8_t chipdecode = ((addr >> 13) & 0x3);
  if (chipdecode == 3) // CHIP 3
    PORTH &= ~(1 << 6); // /E000 LOW (ENABLE)
  else if (chipdecode == 2) // CHIP 2
    PORTH &= ~(1 << 5); // /C000 LOW (ENABLE)
  else if (chipdecode == 1) // CHIP 1
    PORTH &= ~(1 << 4); // /A000 LOW (ENABLE)
  else // CHIP 0
    PORTH &= ~(1 << 3); // /8000 LOW (ENABLE)

  PORTF = addr & 0xFF;        // A0-A7
  PORTK = (addr >> 8) & 0xFF; // A8-A15

  // LATCH ADDRESS - PULL /CE HIGH
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6); // ALL /CE HIGH (DISABLE)

  uint8_t ret = PINC;
  return ret;
}

void readSegment_COL(uint32_t startaddr, uint32_t endaddr)
{
  for (uint32_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_COL(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readROM_COL()
{
  strcpy(fileName, romName);
  strcat(fileName, ".col");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  //  sprintf(folder, "COL/ROM/%s/%d", romName, foldern);
  sprintf(folder, "COL/ROM/%d", foldern);
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

  // RESET ALL CS PINS HIGH (DISABLE)
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  readSegment_COL(0x8000, 0xA000); // 8K
  if (colsize > 0) {
    readSegment_COL(0xA000, 0xB000); // +4K = 12K
    if (colsize > 1) {
      readSegment_COL(0xB000, 0xC000); // +4K = 16K
      if (colsize > 2) {
        readSegment_COL(0xC000, 0xD000); // +4K = 20K
        if (colsize > 3) {
          readSegment_COL(0xD000, 0xE000); // +4K = 24K
          if (colsize > 4) {
            readSegment_COL(0xE000, 0x10000); // +8K = 32K
          }
        }
      }
    }
  }
  myFile.close();

  // RESET ALL CS PINS HIGH (DISABLE)
  PORTH |= (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Compare CRC32 to database and rename ROM if found
  compareCRC("colv.txt", 0, 0);

  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_COL()
{
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (collo == colhi)
    newcolsize = collo;
  else {
    int b = 0;
    int i = collo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(COL[i]);
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
        if (i == collo)
          i = colhi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(COL[i]);
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
        if (i == colhi)
          i = collo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(COL[i]);
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
        newcolsize = i;
        break;
      }
    }
    display.setCursor(0, 56); // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(COL[newcolsize]);
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
      Serial.print(COL[i + collo]);
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
  Serial.print(COL[newcolsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newcolsize);
  colsize = newcolsize;
}

void checkStatus_COL()
{
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
  print_Msg(COL[colsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(COL[colsize]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile colcsvFile;
char colgame[36]; // title
char colrr[3];    // romsize
char colll[4];    // linelength (previous line)
unsigned long colcsvpos; // CSV File Position
char colcartCSV[] = "colcart.txt"; // CSV List
char colcsvEND[] = "EOF";    // CSV End Marker for scrolling

bool readLine_COL(FsFile &f, char* line, size_t maxLen)
{
  for (size_t n = 0; n < maxLen; n++) {
    int c = f.read();
    if ( c < 0 && n == 0) return false;  // EOF
    if (c < 0 || c == '\n') {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false; // line too long
}

bool readVals_COL(char* colgame, char* colrr, char* colll)
{
  char line[42];
  colcsvpos = colcsvFile.position();
  if (!readLine_COL(colcsvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(colgame, comma);
    else if (x == 1)
      strcpy(colrr, comma);
    else if (x == 2)
      strcpy(colll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_COL()
{
  bool buttonreleased = 0;
  bool cartselected = 0;
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F(" HOLD TO FAST CYCLE"));
  display_Update();
#else
  Serial.println(F("HOLD BUTTON TO FAST CYCLE"));
#endif
  delay(2000);
#if defined(enable_OLED)
  buttonVal1 = (PIND & (1 << 7)); // PD7
#elif defined(enable_LCD)
  boolean buttonVal1 = (PING & (1 << 2)); // PG2
#endif
  if (buttonVal1 == LOW) { // Button Held - Fast Cycle
    while (1) { // Scroll Game List
      while (readVals_COL(colgame, colrr, colll)) {
        if (strcmp(colcsvEND, colgame) == 0) {
          colcsvFile.seek(0); // Restart
        }
        else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(colgame);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(colgame);
#endif
#if defined(enable_OLED)
          buttonVal1 = (PIND & (1 << 7)); // PD7
#elif defined(enable_LCD)
          boolean buttonVal1 = (PING & (1 << 2)); // PG2
#endif
          if (buttonVal1 == HIGH) { // Button Released
            buttonreleased = 1;
            break;
          }
          if (buttonreleased) {
            buttonreleased = 0; // Reset Flag
            break;
          }
        }
      }
#if defined(enable_OLED)
      buttonVal1 = (PIND & (1 << 7)); // PD7
#elif defined(enable_LCD)
      boolean buttonVal1 = (PING & (1 << 2)); // PG2
#endif
      if (buttonVal1 == HIGH) // Button Released
        break;
    }
  }
#if (defined(enable_OLED) || defined(enable_LCD))
  display.setCursor(0, 56);
  println_Msg(F("FAST CYCLE OFF"));
  display_Update();
#else
  Serial.println(F(""));
  Serial.println(F("FAST CYCLE OFF"));
  Serial.println(F("PRESS BUTTON TO STEP FORWARD"));
  Serial.println(F("DOUBLE CLICK TO STEP BACK"));
  Serial.println(F("HOLD TO SELECT"));
  Serial.println(F(""));
#endif
  while (readVals_COL(colgame, colrr, colll)) {
    if (strcmp(colcsvEND, colgame) == 0) {
      colcsvFile.seek(0); // Restart
    }
    else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(colgame);
      display.setCursor(0, 48);
#if defined(enable_OLED)
      println_Msg(F("Press left to Change"));
      println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
      println_Msg(F("Rotate to Change"));
      println_Msg(F("Press to Select"));
#endif
      display_Update();
#else
      Serial.print(F("CART TITLE:"));
      Serial.println(colgame);
#endif
      while (1) { // Single Step
        int b = checkButton();
        if (b == 1) { // Continue (press)
          break;
        }
        if (b == 2) { // Reset to Start of List (doubleclick)
          byte prevline = strtol(colll, NULL, 10);
          colcsvpos -= prevline;
          colcsvFile.seek(colcsvpos);
          break;
        }
        if (b == 3) { // Long Press - Select Cart (hold)
          newcolsize = strtol(colrr, NULL, 10);
          EEPROM_writeAnything(8, newcolsize);
          cartselected = 1; // SELECTION MADE
#if (defined(enable_OLED) || defined(enable_LCD))
          println_Msg(F("SELECTION MADE"));
          display_Update();
#else
          Serial.println(F("SELECTION MADE"));
#endif
          break;
        }
      }
      if (cartselected) {
        cartselected = 0; // Reset Flag
        return true;
      }
    }
  }
#if (defined(enable_OLED) || defined(enable_LCD))
  println_Msg(F(""));
  println_Msg(F("END OF FILE"));
  display_Update();
#else
  Serial.println(F("END OF FILE"));
#endif

  return false;
}

void checkCSV_COL()
{
  if (getCartListInfo_COL()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(colgame);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: R"));
    println_Msg(newcolsize);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(colgame);
    // Display Settings
    Serial.print(F("CODE: R"));
    Serial.println(newcolsize);
    Serial.println(F(""));
#endif
  }
  else {
#if (defined(enable_OLED) || defined(enable_LCD))
    display.setCursor(0, 56);
    println_Msg(F("NO SELECTION"));
    display_Update();
#else
    Serial.println(F("NO SELECTION"));
#endif
  }
}

void setCart_COL()
{
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(colcartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "COL/CSV");
  sd.chdir(folder); // Switch Folder
  colcsvFile = sd.open(colcartCSV, O_READ);
  if (!colcsvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_COL();
    }
  }
  checkCSV_COL();

  colcsvFile.close();
}
#endif
