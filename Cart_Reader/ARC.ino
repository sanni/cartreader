//******************************************
// EMERSON ARCADIA 2001 MODULE
//******************************************
#if defined(enable_ARC)
// Emerson Arcadia 2001
// Cartridge Pinout
// 30P 2.54mm pitch connector
//
//       FRONT             BACK
//        SIDE             SIDE
//              +-------+
//         GND -| 2   1 |- A13 (A12)
//   VCC (+5V) -| 4   3 |- D3
//          A0 -| 6   5 |- D4
//          A1 -| 8   7 |- D5
//          A2 -| 10  9 |- D6
//          A3 -| 12 11 |- D7
//          A4 -| 14 13 |- D0
//          A5 -| 16 15 |- D2
//          A6 -| 18 17 |- D1
//          A7 -| 20 19 |- NC
//          A8 -| 22 21 |- NC
//          A9 -| 24 23 |- NC
//         A10 -| 26 25 |- GND
//         A11 -| 28 27 |- GND
//   A12 (/EN) -| 30 29 |- NC
//              +-------+
//
//                                    BACK
//       +------------------------------------------------------------+
//       | 1   3   5   7   9   11  13  15  17  19  21  23  25  27  29 |
// LEFT  |                                                            | RIGHT
//       | 2   4   6   8   10  12  14  16  18  20  22  24  26  28  30 |
//       +------------------------------------------------------------+
//                                    FRONT
//

byte ARC[] = { 2, 4, 6, 8 };
byte arclo = 0;  // Lowest Entry
byte archi = 3;  // Highest Entry

byte arcsize;
byte newarcsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char arcMenuItem1[] PROGMEM = "Select Cart";
static const char arcMenuItem2[] PROGMEM = "Read ROM";
static const char arcMenuItem3[] PROGMEM = "Set Size";
static const char arcMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsARC[] PROGMEM = { arcMenuItem1, arcMenuItem2, arcMenuItem3, arcMenuItem4 };

void setup_ARC() {
  // Set Address Pins to Output
  // Arcadia 2001 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Unused Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)   ---(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_ARC();
  strcpy(romName, "ARCADIA");

  mode = mode_ARC;
}

void arcMenu() {
  convertPgm(menuOptionsARC, 4);
  uint8_t mainMenu = question_box(F("ARCADIA 2001 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_ARC();
      wait();
      setup_ARC();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_ARC();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_ARC();
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

uint8_t readData_ARC(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  return ret;
}

void readSegment_ARC(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_ARC(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void readROM_ARC() {
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ARC/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }
  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  readSegment_ARC(0x0000, 0x0800);  // 2K
  if (arcsize > 0) {
    readSegment_ARC(0x0800, 0x1000);  // +2K = 4K
    if (arcsize > 1) {
      readSegment_ARC(0x2000, 0x2800);  // +2K = 6K
      if (arcsize > 2) {
        readSegment_ARC(0x2800, 0x3000);  // +2K = 8K
      }
    }
  }
  myFile.close();

  unsigned long crcsize = ARC[arcsize] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_ARC() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (arclo == archi)
    newarcsize = arclo;
  else {
    int b = 0;
    int i = arclo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(ARC[i]);
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
        if (i == arclo)
          i = archi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(ARC[i]);
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
        if (i == archi)
          i = arclo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(ARC[i]);
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
        newarcsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(ARC[newarcsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (arclo == archi)
    newarcsize = arclo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (archi - arclo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(ARC[i + arclo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newarcsize = sizeROM.toInt() + arclo;
    if (newarcsize > archi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(ARC[newarcsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newarcsize);
  arcsize = newarcsize;
}

void checkStatus_ARC() {
  EEPROM_readAnything(8, arcsize);
  if (arcsize > 3) {
    arcsize = 0;
    EEPROM_writeAnything(8, arcsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("ARCADIA 2001 READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("ROM SIZE: "));
  print_Msg(ARC[arcsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(ARC[arcsize]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile arccsvFile;
char arcgame[20];                   // title
char arcrr[3];                      // romsize
char arcll[4];                      // linelength (previous line)
unsigned long arccsvpos;            // CSV File Position
char arccartCSV[] = "arccart.txt";  // CSV List
char arccsvEND[] = "EOF";           // CSV End Marker for scrolling

bool readLine_ARC(FsFile& f, char* line, size_t maxLen) {
  for (size_t n = 0; n < maxLen; n++) {
    int c = f.read();
    if (c < 0 && n == 0) return false;  // EOF
    if (c < 0 || c == '\n') {
      line[n] = 0;
      return true;
    }
    line[n] = c;
  }
  return false;  // line too long
}

bool readVals_ARC(char* arcgame, char* arcrr, char* arcll) {
  char line[26];
  arccsvpos = arccsvFile.position();
  if (!readLine_ARC(arccsvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(arcgame, comma);
    else if (x == 1)
      strcpy(arcrr, comma);
    else if (x == 2)
      strcpy(arcll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_ARC() {
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
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {  // Button Held - Fast Cycle
    while (1) {             // Scroll Game List
      while (readVals_ARC(arcgame, arcrr, arcll)) {
        if (strcmp(arccsvEND, arcgame) == 0) {
          arccsvFile.seek(0);  // Restart
        } else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(arcgame);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(arcgame);
#endif
#if defined(enable_OLED)
          buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
          buttonVal1 = (PING & (1 << 2));  //PG2
#endif
          if (buttonVal1 == HIGH) {  // Button Released
            buttonreleased = 1;
            break;
          }
          if (buttonreleased) {
            buttonreleased = 0;  // Reset Flag
            break;
          }
        }
      }
#if defined(enable_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
      buttonVal1 = (PING & (1 << 2));      //PG2
#endif
      if (buttonVal1 == HIGH)  // Button Released
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
  while (readVals_ARC(arcgame, arcrr, arcll)) {
    if (strcmp(arccsvEND, arcgame) == 0) {
      arccsvFile.seek(0);  // Restart
    } else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(arcgame);
      display.setCursor(0, 48);
#if defined(enable_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(enable_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
#else
      Serial.print(F("CART TITLE:"));
      Serial.println(arcgame);
#endif
      while (1) {  // Single Step
        int b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(arcll, NULL, 10);
          arccsvpos -= prevline;
          arccsvFile.seek(arccsvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newarcsize = strtol(arcrr, NULL, 10);
          EEPROM_writeAnything(8, newarcsize);
          cartselected = 1;  // SELECTION MADE
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
        cartselected = 0;  // Reset Flag
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

void checkCSV_ARC() {
  if (getCartListInfo_ARC()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(arcgame);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: R"));
    println_Msg(newarcsize);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(arcgame);
    // Display Settings
    Serial.print(F("CODE: R"));
    Serial.println(newarcsize);
    Serial.println(F(""));
#endif
  } else {
#ifdef enable_OLED
    display.setCursor(0, 56);
    println_Msg(F("NO SELECTION"));
    display_Update();
#else
    Serial.println(F("NO SELECTION"));
#endif
  }
}

void setCart_ARC() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(arccartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "ARC/CSV");
  sd.chdir(folder);  // Switch Folder
  arccsvFile = sd.open(arccartCSV, O_READ);
  if (!arccsvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_ARC();
    }
  }
  checkCSV_ARC();

  arccsvFile.close();
}
#endif
//******************************************
// End of File
//******************************************