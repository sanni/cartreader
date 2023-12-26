//******************************************
// VECTREX MODULE
//******************************************
#ifdef enable_VECTREX
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
//  Defines
//******************************************
#define CLK_ENABLE PORTH |= (1 << 1)    // /E HIGH
#define CLK_DISABLE PORTH &= ~(1 << 1)  // /E LOW
#define PB6_ENABLE PORTH |= (1 << 5)    // PB6 HIGH
#define PB6_DISABLE PORTH &= ~(1 << 5)  // PB6 LOW

byte VECTREX[] = { 4, 8, 12, 16, 32, 64 };
byte vectrexlo = 0;  // Lowest Entry
byte vectrexhi = 5;  // Highest Entry
byte vectrexsize;
byte newvectrexsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char vectrexMenuItem1[] PROGMEM = "Select Cart";
static const char vectrexMenuItem2[] PROGMEM = "Read ROM";
static const char vectrexMenuItem3[] PROGMEM = "Set Size";
static const char* const menuOptionsVECTREX[] PROGMEM = { vectrexMenuItem1, vectrexMenuItem2, vectrexMenuItem3, string_reset2 };

void setup_VECTREX() {
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

  mode = mode_VECTREX;
}

void vectrexMenu() {
  convertPgm(menuOptionsVECTREX, 4);
  uint8_t mainMenu = question_box(F("VECTREX MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_VECTREX();
      wait();
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
// READ CODE
//******************************************

uint8_t readData_VECTREX(uint16_t addr)  // Add Input Pullup
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

void readSegment_VECTREX(uint16_t startaddr, uint16_t endaddr) {
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

void readROM_VECTREX() {
  strcpy(fileName, romName);
  strcat(fileName, ".vec");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "VECTREX/ROM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(sd_error_STR);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

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

  unsigned long crcsize = VECTREX[vectrexsize] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_VECTREX() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (vectrexlo == vectrexhi)
    newvectrexsize = vectrexlo;
  else {
    int b = 0;
    int i = vectrexlo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(VECTREX[i]);
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
        if (i == vectrexlo)
          i = vectrexhi;
        else
          i--;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(VECTREX[i]);
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
        if (i == vectrexhi)
          i = vectrexlo;
        else
          i++;

        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(VECTREX[i]);
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
        newvectrexsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
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
      Serial.println(F(""));
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

void checkStatus_VECTREX() {
  EEPROM_readAnything(8, vectrexsize);
  if (vectrexsize > 2) {
    vectrexsize = 0;  // default 4KB
    EEPROM_writeAnything(8, vectrexsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("VECTREX READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("ROM SIZE: "));
  print_Msg(VECTREX[vectrexsize]);
  println_Msg(F("KB"));
  display_Update();
  wait();
#else
  Serial.print(F("ROM SIZE: "));
  Serial.print(VECTREX[vectrexsize]);
  Serial.println(F("KB"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile vectrexcsvFile;
char vectrexgame[25];                       // title
char vectrexrr[4];                          // romsize
char vectrexll[4];                          // linelength (previous line)
unsigned long vectrexcsvpos;                // CSV File Position
char vectrexcartCSV[] = "vectrexcart.txt";  // CSV List
char vectrexcsvEND[] = "EOF";               // CSV End Marker for scrolling

bool readLine_VECTREX(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_VECTREX(char* vectrexgame, char* vectrexrr, char* vectrexll) {
  char line[31];
  vectrexcsvpos = vectrexcsvFile.position();
  if (!readLine_VECTREX(vectrexcsvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(vectrexgame, comma);
    else if (x == 1)
      strcpy(vectrexrr, comma);
    else if (x == 2)
      strcpy(vectrexll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_VECTREX() {
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
  if (buttonVal1 == LOW) {         // Button Held - Fast Cycle
    while (1) {                    // Scroll Game List
      while (readVals_VECTREX(vectrexgame, vectrexrr, vectrexll)) {
        if (strcmp(vectrexcsvEND, vectrexgame) == 0) {
          vectrexcsvFile.seek(0);  // Restart
        } else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(vectrexgame);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(vectrexgame);
#endif
#if defined(enable_OLED)
          buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(enable_LCD)
          boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
          if (buttonVal1 == HIGH) {        // Button Released
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
      boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
      if (buttonVal1 == HIGH)          // Button Released
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
  while (readVals_VECTREX(vectrexgame, vectrexrr, vectrexll)) {
    if (strcmp(vectrexcsvEND, vectrexgame) == 0) {
      vectrexcsvFile.seek(0);  // Restart
    } else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(vectrexgame);
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
      Serial.println(vectrexgame);
#endif
      while (1) {  // Single Step
        int b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(vectrexll, NULL, 10);
          vectrexcsvpos -= prevline;
          vectrexcsvFile.seek(vectrexcsvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newvectrexsize = strtol(vectrexrr, NULL, 10);
          EEPROM_writeAnything(8, newvectrexsize);
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

void checkCSV_VECTREX() {
  if (getCartListInfo_VECTREX()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(vectrexgame);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: R"));
    println_Msg(newvectrexsize);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(vectrexgame);
    // Display Settings
    Serial.print(F("CODE: R"));
    Serial.println(newvectrexsize);
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

void setCart_VECTREX() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(vectrexcartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "VECTREX/CSV");
  sd.chdir(folder);  // Switch Folder
  vectrexcsvFile = sd.open(vectrexcartCSV, O_READ);
  if (!vectrexcsvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_VECTREX();
    }
  }
  checkCSV_VECTREX();
  vectrexcsvFile.close();
}
#endif
