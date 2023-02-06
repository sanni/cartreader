//******************************************
// FAIRCHILD CHANNEL F MODULE
//******************************************
#if defined(enable_FAIRCHILD)
// Fairchild Channel F
// Cartridge Pinout
// 22P (27P Width) 2.54mm pitch connector
//
//   TOP            BOTTOM
//  SIDE            SIDE
//        +-------+
//        |    == |
//        |     1 |- GND
//        |     2 |- GND
//        |     3 |- D0
//        |     4 |- D1
//        |     5 |- /INTREQ
//        |     6 |- ROMC0
//        |     7 |- ROMC1
//        |     8 |- ROMC2
//        |     9 |- D2
//        |    10 |- ROMC3
//        |    11 |- D3
//        |    == |
//        |    == |
//        |    == |
//        |    12 |- ROMC4
//        |    13 |- PHI
//        |    14 |- D4
//        |    15 |- WRITE
//        |    16 |- D5
//        |    17 |- D6
//        |    18 |- D7
//        |    19 |- VDD(+5V)
//        |    20 |- VDD(+5V)
//        |    21 |- NC
//        |    22 |- VGG(+12V)
//        |    == |
//        +-------+
//
//                                               TOP
//       +----------------------------------------------------------------------------------+
//       |                                                                                  |
// LEFT  |                                                                                  | RIGHT
//       | == 22 21 20 19 18 17 16 15 14 13 12 == == == 11 10  9  8  7  6  5  4  3  2  1 == |
//       +----------------------------------------------------------------------------------+
//                                              BOTTOM
//

// CONTROL PINS:
// PHI(PH3)     - SNES /CS
// /INTREQ(PH4) - SNES /IRQ
// WRITE(PH5)   - SNES /WR
// ROMC0(PF0)   - SNES A0
// ROMC1(PF1)   - SNES A1
// ROMC2(PF2)   - SNES A2
// ROMC3(PF3)   - SNES A3
// ROMC4(PF4)   - SNES A4

/******************************************
  Defines
 *****************************************/
#define PHI_HI PORTH |= (1 << 3)
#define PHI_LOW PORTH &= ~(1 << 3)
#define WRITE_HI PORTH |= (1 << 5)
#define WRITE_LOW PORTH &= ~(1 << 5)

byte FAIRCHILD[] = { 2, 3, 4, 6 };
byte fairchildlo = 0;  // Lowest Entry
byte fairchildhi = 3;  // Highest Entry

byte fairchildsize;
byte newfairchildsize;

// EEPROM MAPPING
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char fairchildMenuItem1[] PROGMEM = "Select Cart";
static const char fairchildMenuItem2[] PROGMEM = "Read ROM";
static const char fairchildMenuItem3[] PROGMEM = "Set Size";
static const char fairchildMenuItem4[] PROGMEM = "Read 16K";
static const char fairchildMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsFAIRCHILD[] PROGMEM = { fairchildMenuItem1, fairchildMenuItem2, fairchildMenuItem3, fairchildMenuItem4, fairchildMenuItem5 };

void setup_FAIRCHILD() {
  // Set Address Pins to Output
  // Channel F uses A0-A4 [A5-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)   PHI(PH3) /INTREQ(PH4) WRITE(PH5) ---(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Unused Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   PHI(PH3) /INTREQ(PH4) WRITE(PH5)   ---(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTK = 0xFF;       // A8-A15
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_FAIRCHILD();
  strcpy(romName, "FAIRCHILD");

  mode = mode_FAIRCHILD;
}

void fairchildMenu() {
  convertPgm(menuOptionsFAIRCHILD, 5);
  uint8_t mainMenu = question_box(F("CHANNEL F MENU"), menuOptions, 5, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_FAIRCHILD();
      wait();
      setup_FAIRCHILD();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_FAIRCHILD();
      sd.chdir("/");
      break;

    case 2:
      // Set Size
      setROMSize_FAIRCHILD();
      break;

    case 3:
      // Read 16K
      sd.chdir("/");
      read16K_FAIRCHILD();
      sd.chdir("/");
      break;

    case 4:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
// READ CODE
//******************************************

// Sean Riddle Dumper Routine
// clear PC0 with ROMC state 8
// loop 256 times
// fetch 16 bytes into buffer with ROMC state 0
// dump buffer to serial port
// clear PC0

// Clear PC0
void clearRegister_FAIRCHILD() {
  PHI_LOW;
  WRITE_LOW;
  PORTF = 0;  // ROMC3 LOW

  delay(2000);

  PHI_HI;
  WRITE_HI;
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  WRITE_LOW;
  PHI_HI;
  PORTF = 0;  // ROMC3 LOW
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  PORTF = 0;  // ROMC3 LOW
  PHI_HI;
  PORTF = 0x8;  // this puts us in ROMC state 8 - clear PC0
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  PORTF = 0x08;  // ROMC3 HIGH
  PHI_HI;
  PORTF = 0x08;  // ROMC3 HIGH
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  PHI_HI;
  WRITE_HI;
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  WRITE_LOW;
  PHI_HI;
  PORTF = 0;  // ROMC3 LOW
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  WRITE_LOW;
  PHI_HI;
  PORTF = 0;  // ROMC3 LOW
  NOP;
  NOP;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
  NOP;

  WRITE_LOW;
}

void setROMC_FAIRCHILD(uint8_t command) {
  PHI_LOW;
  WRITE_LOW;
  NOP;

  WRITE_HI;
  PHI_HI;
  NOP;

  PHI_LOW;
  NOP;
  NOP;

  WRITE_LOW;
  PHI_HI;
  NOP;
  NOP;

  // PWs = 4 PHI Cycles
  // PWl = 6 PHI Cycles
  for (int x = 0; x < 2; x++) {  // 2 PHI
    PHI_LOW;
    NOP;
    NOP;
    PHI_HI;
    NOP;
    NOP;
  }
  PORTF = command;  // ROMC3 = command

  for (int x = 0; x < 3; x++) {  // 4 PHI
    PHI_LOW;
    NOP;
    NOP;
    PHI_HI;
    NOP;
    NOP;
  }

  PHI_LOW;
  NOP;
  NOP;

  PHI_HI;
  WRITE_HI;
  NOP;

  PHI_LOW;
  NOP;
  NOP;

  PHI_HI;
  WRITE_LOW;
  NOP;

  PHI_LOW;
  NOP;
  NOP;
}

void setREAD_FAIRCHILD() {
  PHI_LOW;
  WRITE_LOW;
  NOP;

  WRITE_HI;
  PHI_HI;
  NOP;

  PHI_LOW;
  NOP;
  NOP;

  WRITE_LOW;
  PHI_HI;
  NOP;
  NOP;

  // PWs = 4 PHI Cycles
  // PWl = 6 PHI Cycles
  for (int x = 0; x < 2; x++) {  // 2 PHI
    PHI_LOW;
    NOP;
    NOP;
    PHI_HI;
    NOP;
    NOP;
  }
  PORTF = 0;  // ROMC3 = 0 = Fetch Data
}

uint8_t readData_FAIRCHILD() {
  for (int x = 0; x < 3; x++) {  // 4 PHI
    PHI_LOW;
    NOP;
    NOP;
    PHI_HI;
    NOP;
    NOP;
  }

  PHI_LOW;
  NOP;
  NOP;

  PHI_HI;
  WRITE_HI;
  NOP;

  PHI_LOW;
  NOP;
  NOP;

  PHI_HI;
  WRITE_LOW;
  NOP;

  uint8_t ret = PINC;  // read databus into buffer

  PHI_LOW;
  NOP;
  NOP;

  return ret;
}

void readROM_FAIRCHILD() {
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "FAIRCHILD/ROM/%d", foldern);
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

  unsigned long cartsize = FAIRCHILD[fairchildsize] * 0x400;
  uint8_t blocks = cartsize / 0x200;
  setROMC_FAIRCHILD(0x8);  // Clear PC0
  setREAD_FAIRCHILD();

  // ROM Start Bytes
  // 0x55,0x08 - desert fox, muehle, space war, tic-tac-toe (all 2K)
  // 0x55,0x2B - most carts
  // 0x55,0xAA - alien invasion (4K)
  // 0x55,0xBB - video whizball (3K)
  for (int y = 0; y < 0x4800; y++) {
    uint8_t startbyte = readData_FAIRCHILD();
    if (startbyte == 0x55) {  // Start Byte
      sdBuffer[0] = startbyte;
      startbyte = readData_FAIRCHILD();
      if ((startbyte == 0x08) || (startbyte == 0x2B) || (startbyte == 0xAA) || (startbyte == 0xBB)) {
        sdBuffer[1] = startbyte;
        for (int w = 2; w < 512; w++) {
          startbyte = readData_FAIRCHILD();
          sdBuffer[w] = startbyte;
        }
        myFile.write(sdBuffer, 512);
        delay(1);  // Added delay
        for (int z = 1; z < blocks; z++) {
          // Skip BIOS/Blocks Code for 4K Cart
          if (cartsize == 0x1000) {  // Pro Football 4K
            setROMC_FAIRCHILD(0x8);  // Clear PC0
            setREAD_FAIRCHILD();
            uint16_t offset = z * 0x200;
            for (int x = 0; x < 0x800 + offset; x++) {  // Skip BIOS/Previous Blocks
              readData_FAIRCHILD();
            }
          }
          for (int w = 0; w < 512; w++) {
            uint8_t temp = readData_FAIRCHILD();
            sdBuffer[w] = temp;
          }
          myFile.write(sdBuffer, 512);
          delay(1);  // Added delay
        }
        break;
      }
    }
  }
  myFile.close();

  calcCRC(fileName, cartsize, NULL, 0);

  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void read16K_FAIRCHILD()  // Read 16K Bytes
{
  strcpy(fileName, romName);
  strcat(fileName, ".bin");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "FAIRCHILD/ROM/%d", foldern);
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

  unsigned long cartsize = FAIRCHILD[fairchildsize] * 0x400;
  for (int y = 0; y < 0x20; y++) {
    // Skip BIOS/Blocks Code for 4K Cart
    // 3K CARTS MAY NEED THE SKIP BIOS/BLOCKS ROUTINE USED FOR THE 4K CART
    // TEST 3K CARTS BY SETTING ROM SIZE TO 2K/4K AND COMPARE 16K DUMPS
    if (cartsize == 0x1000) {  // Pro Football 4K
      setROMC_FAIRCHILD(0x8);  // Clear PC0
      setREAD_FAIRCHILD();
      uint16_t offset = y * 0x200;
      for (int x = 0; x < 0x800 + offset; x++) {  // Skip BIOS/Previous Blocks
        readData_FAIRCHILD();
      }
    }
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_FAIRCHILD();
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
    delay(1);  // Added delay
  }
  myFile.close();

  calcCRC(fileName, 0x4000, NULL, 0);

  println_Msg(F(""));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void setROMSize_FAIRCHILD() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  if (fairchildlo == fairchildhi)
    newfairchildsize = fairchildlo;
  else {
    int b = 0;
    int i = fairchildlo;
    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(FAIRCHILD[i]);
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
        if (i == fairchildlo)
          i = fairchildhi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(FAIRCHILD[i]);
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
        if (i == fairchildhi)
          i = fairchildlo;
        else
          i++;
        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(FAIRCHILD[i]);
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
        newfairchildsize = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(FAIRCHILD[newfairchildsize]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (fairchildlo == fairchildhi)
    newfairchildsize = fairchildlo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (fairchildhi - fairchildlo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(FAIRCHILD[i + fairchildlo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    newfairchildsize = sizeROM.toInt() + fairchildlo;
    if (newfairchildsize > fairchildhi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(F(""));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(FAIRCHILD[newfairchildsize]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, newfairchildsize);
  fairchildsize = newfairchildsize;
}

void checkStatus_FAIRCHILD() {
  EEPROM_readAnything(8, fairchildsize);
  if (fairchildsize > 3) {
    fairchildsize = 0;
    EEPROM_writeAnything(8, fairchildsize);
  }

#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(F("CHANNEL F READER"));
  println_Msg(F("CURRENT SETTINGS"));
  println_Msg(F(""));
  print_Msg(F("ROM SIZE: "));
  print_Msg(FAIRCHILD[fairchildsize]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("CURRENT ROM SIZE: "));
  Serial.print(FAIRCHILD[fairchildsize]);
  Serial.println(F("K"));
  Serial.println(F(""));
#endif
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile fairchildcsvFile;
char fairchildgame[33];                         // title
char fairchildrr[3];                            // romsize
char fairchildll[4];                            // linelength (previous line)
unsigned long fairchildcsvpos;                  // CSV File Position
char fairchildcartCSV[] = "fairchildcart.txt";  // CSV List
char fairchildcsvEND[] = "EOF";                 // CSV End Marker for scrolling

bool readLine_FAIRCHILD(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_FAIRCHILD(char* fairchildgame, char* fairchildrr, char* fairchildll) {
  char line[39];
  fairchildcsvpos = fairchildcsvFile.position();
  if (!readLine_FAIRCHILD(fairchildcsvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(fairchildgame, comma);
    else if (x == 1)
      strcpy(fairchildrr, comma);
    else if (x == 2)
      strcpy(fairchildll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_FAIRCHILD() {
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
      while (readVals_FAIRCHILD(fairchildgame, fairchildrr, fairchildll)) {
        if (strcmp(fairchildcsvEND, fairchildgame) == 0) {
          fairchildcsvFile.seek(0);  // Restart
        } else {
#if (defined(enable_OLED) || defined(enable_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(F(""));
          println_Msg(fairchildgame);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(fairchildgame);
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
  while (readVals_FAIRCHILD(fairchildgame, fairchildrr, fairchildll)) {
    if (strcmp(fairchildcsvEND, fairchildgame) == 0) {
      fairchildcsvFile.seek(0);  // Restart
    } else {
#if (defined(enable_OLED) || defined(enable_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(F(""));
      println_Msg(fairchildgame);
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
      Serial.println(fairchildgame);
#endif
      while (1) {  // Single Step
        int b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(fairchildll, NULL, 10);
          fairchildcsvpos -= prevline;
          fairchildcsvFile.seek(fairchildcsvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          newfairchildsize = strtol(fairchildrr, NULL, 10);
          EEPROM_writeAnything(8, newfairchildsize);
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

void checkCSV_FAIRCHILD() {
  if (getCartListInfo_FAIRCHILD()) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CART SELECTED"));
    println_Msg(F(""));
    println_Msg(fairchildgame);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: R"));
    println_Msg(newfairchildsize);
    display_Update();
#else
    Serial.println(F(""));
    Serial.println(F("CART SELECTED"));
    Serial.println(fairchildgame);
    // Display Settings
    Serial.print(F("CODE: R"));
    Serial.println(newfairchildsize);
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

void setCart_FAIRCHILD() {
#if (defined(enable_OLED) || defined(enable_LCD))
  display_Clear();
  println_Msg(fairchildcartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "FAIRCHILD/CSV");
  sd.chdir(folder);  // Switch Folder
  fairchildcsvFile = sd.open(fairchildcartCSV, O_READ);
  if (!fairchildcsvFile) {
#if (defined(enable_OLED) || defined(enable_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_FAIRCHILD();
    }
  }
  checkCSV_FAIRCHILD();

  fairchildcsvFile.close();
}
#endif
//******************************************
// End of File
//******************************************