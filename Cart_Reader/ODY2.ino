//******************************************
// MAGNAVOX ODYSSEY 2 MODULE
//******************************************
#if defined(enable_ODY2)
// Magnavox Odyssey 2
// Philips Videopac/Videopac+
// Cartridge Pinout
// 30P 3.96mm pitch connector
//
//       FRONT            BACK
//        SIDE            SIDE
//              +------+
//          T0 -| 1  A |- /WR
//          D0 -| 2  B |- GND
//          D1 -| 3  C |- GND
//          D2 -| 4  D |- VCC (+5V)
//          D3 -| 5  E |- CS3
//          D4 -| 6  F |- /PSEN (/CE)
//          D5 -| 7  G |- A0
//          D6 -| 8  H |- A1
//          D7 -| 9  J |- A2
//   A10 (P22) -| 10 K |- A3
//  /CS1 (P14) -| 11 L |- A4
//         P11 -| 12 M |- A5
//         P10 -| 13 N |- A7
//   A11 (P23) -| 14 P |- A6
//    A9 (P21) -| 15 R |- A8 (P20)
//              +------+
//
// NOTE:  ADDRESS A7/A6 PIN ORDER ON PINS N & P.
// NOTE:  MOST CARTS DO NOT CONNECT A10 ON PIN 10.
//
//                           BACK
//      +---------------------------------------------+
//      | A  B  C  D  E  F  G  H  J  K  L  M  N  P  R |
// LEFT |                                             | RIGHT
//      | 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 |
//      +---------------------------------------------+
//                           FRONT
//
// CONTROL PINS:
// T0(PH0)    - SNES RESET
// CS3(PH3)   - SNES /CS
// /CS1(PH4)  - SNES /IRQ
// /WR(PH5)   - SNES /WR
// /PSEN(PH6) - SNES /RD

byte ODY2[] = { 2, 4, 8, 12, 16 };
byte ody2lo = 0;  // Lowest Entry
byte ody2hi = 4;  // Highest Entry

byte ody2mapper;
byte newody2mapper;
byte ody2size;
byte newody2size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char ody2MenuItem1[] PROGMEM = "Select Cart";
static const char ody2MenuItem2[] PROGMEM = "Read ROM";
static const char ody2MenuItem3[] PROGMEM = "Set Size";
static const char ody2MenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsODY2[] PROGMEM = { ody2MenuItem1, ody2MenuItem2, ody2MenuItem3, ody2MenuItem4 };

void setup_ODY2() {
  // Set Address Pins to Output
  // Odyssey 2 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //        T0(PH0)   ---(PH1)   CS3(PH3)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //        T0(PH0)   ---(PH1)   /CS1(PH4)  /WR(PH5)   /RD(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set CS3(PH3) to LOW
  PORTH &= ~(1 << 3);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_ODY2();
  strcpy(romName, "ODYSSEY2");

  mode = mode_ODY2;
}

void ody2Menu() {
  convertPgm(menuOptionsODY2, 4);
  uint8_t mainMenu = question_box(F("ODYSSEY 2 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_ODY2();
      wait();
      setup_ODY2();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_ODY2();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_ODY2();
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

uint8_t readData_ODY2(uint16_t addr) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13

  // Set /PSEN (/CE) to LOW
  PORTH &= ~(1 << 6);  // /PSEN LOW (ENABLE)
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // Pull /PSEN (/CE) to HIGH
  PORTH |= (1 << 6);  // /PSEN HIGH (DISABLE)

  return ret;
}

void readSegment_ODY2(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_ODY2(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

void bankSwitch_ODY2(uint16_t addr, uint8_t data) {
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /CS1(PH4) to LOW
  PORTH &= ~(1 << 4);
  // Set /WR(PH5) to LOW
  PORTH &= ~(1 << 5);
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  DDRC = 0xFF;  // Set to Output
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  PORTC = data;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Set /WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Set /CS1(PH4) to HIGH
  PORTH |= (1 << 4);

  DDRC = 0x00;  // Reset to Input
}

void readROM_ODY2() {
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ODY2/ROM/%d", foldern);
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

  if (ody2mapper == 1) {  // A10 CONNECTED
    // Videopac 31:  Musician
    // Videopac 40:  4 in 1 Row/4 en 1 Ligne
    readSegment_ODY2(0x0000, 0x1000);  // 4K
  }
  // A10 NOT CONNECTED
  else if (ody2size > 2) {  // 12K/16K (BANKSWITCH)
    // Videopac+ 55:  Neutron Star         12K = 2K x 6 Banks
    // Videopac+ 58:  Norseman             12K = 2K x 6 Banks
    // Videopac+ 59:  Helicopter Rescue    16K = 2K x 8 Banks
    // Videopac+ 60:  Trans American Rally 16K = 2K x 8 Banks
    uint8_t ody2banks = (ody2size * 4) / 2;
    for (int x = (ody2banks - 1); x >= 0; x--) {
      bankSwitch_ODY2(0x80, ~x);
      readSegment_ODY2(0x0400, 0x0C00);  // 2K x 6/8 = 12K/16K
    }
  } else {                             // STANDARD SIZES
    readSegment_ODY2(0x0400, 0x0C00);  // 2K
    if (ody2size > 0) {
      readSegment_ODY2(0x1400, 0x1C00);  // +2K = 4K
      if (ody2size > 1) {
        readSegment_ODY2(0x2400, 0x2C00);  // +2K = 6K
        readSegment_ODY2(0x3400, 0x3C00);  // +2K = 8K
      }
    }
  }
  myFile.close();

  unsigned long crcsize = ODY2[ody2size] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_ODY2() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (ody2lo == ody2hi)
    newody2size = ody2lo;
  else {
    int b = 0;
    int i = ody2lo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(ODY2[i]);
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
        if (i == ody2lo)
          i = ody2hi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(ODY2[i]);
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
        if (i == ody2hi)
          i = ody2lo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(ODY2[i]);
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
        newody2size = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(ODY2[newody2size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (ody2lo == ody2hi)
    newody2size = ody2lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (ody2hi - ody2lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(ODY2[i + ody2lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newody2size = sizeROM.toInt() + ody2lo;
    if (newody2size > ody2hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(ODY2[newody2size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newody2size);
  ody2size = newody2size;
}

void checkStatus_ODY2() {
  EEPROM_readAnything(7, ody2mapper);
  EEPROM_readAnything(8, ody2size);
  if (ody2mapper > 1) {
    ody2mapper = 0;
    EEPROM_writeAnything(7, ody2mapper);
  }
  if (ody2size > 4) {
    ody2size = 0;
    EEPROM_writeAnything(8, ody2size);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("ODYSSEY 2 READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("MAPPER:   "));
  println_Msg(ody2mapper);
  print_Msg(F("ROM SIZE: "));
  print_Msg(ODY2[ody2size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT MAPPER:   "));
  Serial.println(ody2mapper);
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(ODY2[ody2size]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile ody2csvFile;
char ody2game[51];                    // title
char ody2mm[3];                       // mapper (A10)
char ody2rr[3];                       // romsize
char ody2ll[4];                       // linelength (previous line)
unsigned long ody2csvpos;             // CSV File Position
char ody2cartCSV[] = "ody2cart.txt";  // CSV List
char ody2csvEND[] = "EOF";            // CSV End Marker for scrolling

bool readLine_ODY2(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_ODY2(char* ody2game, char* ody2mm, char* ody2rr, char* ody2ll) {
  char line[59];
  ody2csvpos = ody2csvFile.position();
  if (!readLine_ODY2(ody2csvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(ody2game, comma);
    else if (x == 1)
      strcpy(ody2mm, comma);
    else if (x == 2)
      strcpy(ody2rr, comma);
    else if (x == 3)
      strcpy(ody2ll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_ODY2() {
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
      while (readVals_ODY2(ody2game, ody2mm, ody2rr, ody2ll)) {
        if (strcmp(ody2csvEND, ody2game) == 0) {
          ody2csvFile.seek(0);  // Restart
        } else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(ody2game);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(ody2game);
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
  while (readVals_ODY2(ody2game, ody2mm, ody2rr, ody2ll)) {
    if (strcmp(ody2csvEND, ody2game) == 0) {
      ody2csvFile.seek(0);  // Restart
    } else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(ody2game);
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
      Serial.println(ody2game);
#endif
      while (1) {  // Single Step
        int b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(ody2ll, NULL, 10);
          ody2csvpos -= prevline;
          ody2csvFile.seek(ody2csvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newody2mapper = strtol(ody2mm, NULL, 10);
          newody2size = strtol(ody2rr, NULL, 10);
          EEPROM_writeAnything(7, newody2mapper);
          EEPROM_writeAnything(8, newody2size);
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

void checkCSV_ODY2() {
  if (getCartListInfo_ODY2()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(ody2game);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: M"));
    print_Msg(newody2mapper);
    print_Msg(F("/R"));
    println_Msg(newody2size);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(ody2game);
    // Display Settings
    Serial.print(F("CODE: M"));
    Serial.print(newody2mapper);
    Serial.print(F("/R"));
    Serial.println(newody2size);
    Serial.println(F(""));
#endif
  } else {
#if (defined(enable_OLED) || defined(enable_LCD))
    display.setCursor(0, 56);
    println_Msg(F("NO SELECTION"));
    display_Update();
#else
    Serial.println(F("NO SELECTION"));
#endif
  }
}

void setCart_ODY2() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(ody2cartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "ODY2/CSV");
  sd.chdir(folder);  // Switch Folder
  ody2csvFile = sd.open(ody2cartCSV, O_READ);
  if (!ody2csvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_ODY2();
    }
  }
  checkCSV_ODY2();

  ody2csvFile.close();
}
#endif
//******************************************
// End of File
//******************************************