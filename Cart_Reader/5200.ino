//******************************************
// ATARI 5200 MODULE
//******************************************
#ifdef ENABLE_5200
// Atari 5200
// Cartridge Pinout
// 36P 2.54mm pitch connector
//
//                        RIGHT
//                      +-------+
//           INTERLOCK -| 18 19 |- A0
//                  A2 -| 17 20 |- A1
//    L             A5 -| 16 21 |- A3
//    A             A6 -| 15 22 |- A4         B
//    B            GND -| 14 23 |- GND        O
//    E            GND -| 13 24 |- GND        T
//    L            GND -| 12 25 |- GND        T
//                 N/C -| 11 26 |- +5V        O
//   /ENABLE 4000-7FFF -| 10 27 |- A7         M
//   /ENABLE 8000-BFFF -|  9 28 |- N/C
//                  D7 -|  8 29 |- A8         S
//    S             D6 -|  7 30 |- AUD        I
//    I             D5 -|  6 31 |- A9         D
//    D             D4 -|  5 32 |- A13        E
//    E             D3 -|  4 33 |- A10
//                  D2 -|  3 34 |- A12
//                  D1 -|  2 35 |- A11
//                  D0 -|  1 36 |- INTERLOCK
//                      +-------+
//                        LEFT
//
//                                        LABEL SIDE
//
//                                         /EN /EN
//          D0  D1  D2  D3  D4  D5  D6  D7  80  40  -- GND GND GND  A6  A5  A2  INT
//       +--------------------------------------------------------------------------+
//       |   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  |
// LEFT  |                                                                          | RIGHT
//       |  36  35  34  33  32  31  30  29  28  27  26  25  24  23  22  21  20  19  |
//       +--------------------------------------------------------------------------+
//         INT A11 A12 A10 A13  A9 AUD  A8  --  A7 +5V GND GND GND  A4  A3  A1  A0
//
//                                        BOTTOM SIDE

// CONTROL PINS:
// /4000(PH5) - SNES /WR
// /8000(PH6) - SNES /RD

//******************************************
//  Defines
//******************************************
#define DISABLE_4000 PORTH |= (1 << 5)  // ROM SELECT 4000-7FFF
#define ENABLE_4000 PORTH &= ~(1 << 5)
#define DISABLE_8000 PORTH |= (1 << 6)  // ROM SELECT 8000-BFFF
#define ENABLE_8000 PORTH &= ~(1 << 6)

//******************************************
//  Supported Mappers
//******************************************
// Supported Mapper Array
// Format = {mapper,romsizelo,romsizehi}
static const byte PROGMEM a5200mapsize[] = {
  0, 0, 3,  // Standard 4K/8K/16K/32K
  1, 2, 2,  // Two Chip 16K
  2, 4, 4,  // Bounty Bob Strikes Back 40K [UNTESTED]
};

byte a5200mapcount = 3;  // (sizeof(a5200mapsize) / sizeof(a5200mapsize[0])) / 3;
byte a5200mapselect;
int a5200index;

byte a5200[] = { 4, 8, 16, 32, 40 };
byte a5200lo = 0;  // Lowest Entry
byte a5200hi = 4;  // Highest Entry
byte a5200mapper = 0;
byte new5200mapper;
byte a5200size;
byte new5200size;

// EEPROM MAPPING
// 07 MAPPER
// 08 ROM SIZE

//******************************************
//  Menu
//******************************************
// Base Menu
static const char* const menuOptions5200[] PROGMEM = { FSTRING_SELECT_CART, FSTRING_READ_ROM, FSTRING_SET_SIZE, FSTRING_RESET };

void setup_5200() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  // Atari 5200 uses A0-A13 [A14-A23 UNUSED]
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D7) to Input
  DDRC = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH0)   ---(PH1)   ---(PH3)   ---(PH4)  /4000(PH5) /8000(PH6)
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Unused Data Pins (PA0-PA7) to Output
  DDRA = 0xFF;

  // Set Unused Pins HIGH
  PORTA = 0xFF;
  PORTL = 0xFF;       // A16-A23
  PORTJ |= (1 << 0);  // TIME(PJ0)

  checkStatus_5200();
  strcpy(romName, "ATARI");

  mode = CORE_5200;
}

void a5200Menu() {
  convertPgm(menuOptions5200, 4);
  uint8_t mainMenu = question_box(F("ATARI 5200 MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Select Cart
      setCart_5200();
      wait();
      setup_5200();
      break;

    case 1:
      // Read ROM
      sd.chdir("/");
      readROM_5200();
      sd.chdir("/");
      break;

    case 2:
      // Set Mapper + Size
      setMapper_5200();
      checkMapperSize_5200();
      setROMSize_5200();
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

uint8_t readData_5200(uint16_t addr)  // Add Input Pullup
{
  PORTF = addr & 0xFF;         // A0-A7
  PORTK = (addr >> 8) & 0xFF;  // A8-A13
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // DDRC = 0x00; // Set to Input
  PORTC = 0xFF;  // Input Pullup
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  // Extended Delay for Vanguard
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return ret;
}

void readSegment_5200(uint16_t startaddr, uint16_t endaddr) {
  for (uint16_t addr = startaddr; addr < endaddr; addr += 512) {
    for (int w = 0; w < 512; w++) {
      uint8_t temp = readData_5200(addr + w);
      sdBuffer[w] = temp;
    }
    myFile.write(sdBuffer, 512);
  }
}

//******************************************
// READ ROM
//******************************************

void readROM_5200() {
  strcpy(fileName, romName);
  strcat(fileName, ".a52");

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "5200/ROM/%d", foldern);
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

  // 5200 A13-A0 = 10 0000 0000 0000

  switch (a5200mapper) {
    case 0:  // Standard 4KB/8KB/16KB/32KB
      // Lower Half of 32K is at 0x4000
      if (a5200size == 3) {  // 32K
        ENABLE_4000;
        readSegment_5200(0x4000, 0x8000);  // +16K = 32K
        DISABLE_4000;
      }
      // 4K/8K/16K + Upper Half of 32K
      ENABLE_8000;
      if (a5200size > 1)
        readSegment_5200(0x8000, 0xA000);  // +8K = 16K
      if (a5200size > 0)
        readSegment_5200(0xA000, 0xB000);  // +4K = 8K
      // Base 4K
      readSegment_5200(0xB000, 0xC000);  // 4K
      DISABLE_8000;
      break;

    case 1:  // Two Chip 16KB
      ENABLE_4000;
      readSegment_5200(0x4000, 0x6000);  // 8K
      DISABLE_4000;
      ENABLE_8000;
      readSegment_5200(0x8000, 0xA000);  // +8K = 16K
      DISABLE_8000;
      break;

    case 2:  // Bounty Bob Strikes Back 40KB [UNTESTED]
      ENABLE_4000;
      // First 16KB (4KB x 4)
      for (int w = 0; w < 4; w++) {
        readData_5200(0x4FF6 + w);
        readSegment_5200(0x4000, 0x4E00);
        // Split Read of Last 0x200 bytes
        for (int x = 0; x < 0x1F6; x++) {
          sdBuffer[x] = readData_5200(0x4E00 + x);
        }
        myFile.write(sdBuffer, 502);
        // Bank Registers 0x4FF6-0x4FF9
        for (int y = 0; y < 4; y++) {
          readData_5200(0x4FFF);  // Reset Bank
          sdBuffer[y] = readData_5200(0x4FF6 + y);
        }
        // End of Bank 0x4FFA-0x4FFF
        readData_5200(0x4FFF);      // Reset Bank
        readData_5200(0x4FF6 + w);  // Set Bank
        for (int z = 4; z < 10; z++) {
          sdBuffer[z] = readData_5200(0x4FF6 + z);  // 0x4FFA-0x4FFF
        }
        myFile.write(sdBuffer, 10);
      }
      readData_5200(0x4FFF);  // Reset Bank
      // Second 16KB (4KB x 4)
      for (int w = 0; w < 4; w++) {
        readData_5200(0x5FF6 + w);
        readSegment_5200(0x5000, 0x5E00);
        // Split Read of Last 0x200 bytes
        for (int x = 0; x < 0x1F6; x++) {
          sdBuffer[x] = readData_5200(0x5E00 + x);
        }
        myFile.write(sdBuffer, 502);
        // Bank Registers 0x5FF6-0x5FF9
        for (int y = 0; y < 4; y++) {
          readData_5200(0x5FFF);  // Reset Bank
          sdBuffer[y] = readData_5200(0x5FF6 + y);
        }
        // End of Bank 0x5FFA-0x5FFF
        readData_5200(0x5FFF);      // Reset Bank
        readData_5200(0x5FF6 + w);  // Set Bank
        for (int z = 4; z < 10; z++) {
          sdBuffer[z] = readData_5200(0x5FF6 + z);  // 0x5FFA-0x5FFF
        }
        myFile.write(sdBuffer, 10);
      }
      readData_5200(0x5FFF);  // Reset Bank
      DISABLE_4000;
      ENABLE_8000;
      readSegment_5200(0x8000, 0xA000);  // +8K = 40K
      DISABLE_8000;
      break;
  }
  myFile.close();

  unsigned long crcsize = a5200[a5200size] * 0x400;
  calcCRC(fileName, crcsize, NULL, 0);

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

//******************************************
// ROM SIZE
//******************************************

void println_Mapper5200(byte mapper) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  if (mapper == 0)
    println_Msg(F("STANDARD"));
  else if (mapper == 1)
    println_Msg(F("TWO CHIP"));
  else if (mapper == 2)
    println_Msg(F("BOUNTY BOB"));
#else
  if (mapper == 0)
    Serial.println(F("STANDARD"));
  else if (mapper == 1)
    Serial.println(F("TWO CHIP"));
  else if (mapper == 2)
    Serial.println(F("BOUNTY BOB"));
#endif
}

void checkMapperSize_5200() {
  for (int i = 0; i < a5200mapcount; i++) {
    a5200index = i * 3;
    byte mapcheck = pgm_read_byte(a5200mapsize + a5200index);
    if (mapcheck == a5200mapper) {
      a5200lo = pgm_read_byte(a5200mapsize + a5200index + 1);
      a5200hi = pgm_read_byte(a5200mapsize + a5200index + 2);
      break;
    }
  }
}

void setROMSize_5200() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  if (a5200lo == a5200hi)
    new5200size = a5200lo;
  else {
    uint8_t b = 0;
    int i = a5200lo;

    display_Clear();
    print_Msg(F("ROM Size: "));
    println_Msg(a5200[i]);
    println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
    print_STR(press_to_change_STR, 1);
    print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
    print_STR(rotate_to_change_STR, 1);
    print_STR(press_to_select_STR, 1);
#endif
    display_Update();

    while (1) {
      b = checkButton();
      if (b == 2) {  // Previous (doubleclick)
        if (i == a5200lo)
          i = a5200hi;
        else
          i--;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(a5200[i]);
        println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 1) {  // Next (press)
        if (i == a5200hi)
          i = a5200lo;
        else
          i++;

        // Only update display after input because of slow LCD library
        display_Clear();
        print_Msg(F("ROM Size: "));
        println_Msg(a5200[i]);
        println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
        print_STR(press_to_change_STR, 1);
        print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
        print_STR(rotate_to_change_STR, 1);
        print_STR(press_to_select_STR, 1);
#endif
        display_Update();
      }
      if (b == 3) {  // Long Press - Execute (hold)
        new5200size = i;
        break;
      }
    }
    display.setCursor(0, 56);  // Display selection at bottom
  }
  print_Msg(F("ROM SIZE "));
  print_Msg(a5200[new5200size]);
  println_Msg(F("K"));
  display_Update();
  delay(1000);
#else
  if (a5200lo == a5200hi)
    new5200size = a5200lo;
  else {
setrom:
    String sizeROM;
    for (int i = 0; i < (a5200hi - a5200lo + 1); i++) {
      Serial.print(F("Select ROM Size:  "));
      Serial.print(i);
      Serial.print(F(" = "));
      Serial.print(a5200[i + a5200lo]);
      Serial.println(F("K"));
    }
    Serial.print(F("Enter ROM Size: "));
    while (Serial.available() == 0) {}
    sizeROM = Serial.readStringUntil('\n');
    Serial.println(sizeROM);
    new5200size = sizeROM.toInt() + a5200lo;
    if (new5200size > a5200hi) {
      Serial.println(F("SIZE NOT SUPPORTED"));
      Serial.println(FS(FSTRING_EMPTY));
      goto setrom;
    }
  }
  Serial.print(F("ROM Size = "));
  Serial.print(a5200[new5200size]);
  Serial.println(F("K"));
#endif
  EEPROM_writeAnything(8, new5200size);
  a5200size = new5200size;
}

void checkStatus_5200() {
  EEPROM_readAnything(7, a5200mapper);
  EEPROM_readAnything(8, a5200size);
  if (a5200mapper > 2) {
    a5200mapper = 0;  // default STANDARD
    EEPROM_writeAnything(7, a5200mapper);
  }
  if (a5200size > 4) {
    a5200size = 1;  // default 8K
    EEPROM_writeAnything(8, a5200size);
  }

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F("ATARI 5200 READER"));
  println_Msg(FS(FSTRING_CURRENT_SETTINGS));
  println_Msg(FS(FSTRING_EMPTY));
  print_Msg(F("MAPPER:   "));
  println_Msg(a5200mapper);
  println_Mapper5200(a5200mapper);
  print_Msg(F("ROM SIZE: "));
  print_Msg(a5200[a5200size]);
  println_Msg(F("K"));
  display_Update();
  wait();
#else
  Serial.print(F("MAPPER:   "));
  Serial.println(a5200mapper);
  println_Mapper5200(a5200mapper);
  Serial.print(F("ROM SIZE: "));
  Serial.print(a5200[a5200size]);
  Serial.println(F("K"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
}

//******************************************
// SET MAPPER
//******************************************
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
void displayMapperSelect_5200(int index, boolean printInstructions) {
  display_Clear();
  print_Msg(F("Mapper: "));
  a5200index = index * 3;
  a5200mapselect = pgm_read_byte(a5200mapsize + a5200index);
  println_Msg(a5200mapselect);
  println_Mapper5200(a5200mapselect);

  if(printInstructions) {
    println_Msg(FS(FSTRING_EMPTY));
#if defined(ENABLE_OLED)
    print_STR(press_to_change_STR, 1);
    print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
    print_STR(rotate_to_change_STR, 1);
    print_STR(press_to_select_STR, 1);
#endif
  }
  display_Update();
}
#endif


void setMapper_5200() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  uint8_t b = 0;
  int i = 0;
  // Check Button Status
#if defined(ENABLE_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {             // Button Pressed
    while (1) {                        // Scroll Mapper List
#if defined(ENABLE_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
      buttonVal1 = (PING & (1 << 2));      //PG2
#endif
      if (buttonVal1 == HIGH) {        // Button Released
        // Correct Overshoot
        if (i == 0)
          i = a5200mapcount - 1;
        else
          i--;
        break;
      }
      displayMapperSelect_5200(i, false);
      if (i == (a5200mapcount - 1))
        i = 0;
      else
        i++;
      delay(250);
    }
  }

  displayMapperSelect_5200(i, true);
  
  while (1) {
    b = checkButton();
    if (b == 2) {  // Previous Mapper (doubleclick)
      if (i == 0)
        i = a5200mapcount - 1;
      else
        i--;

      // Only update display after input because of slow LCD library
      displayMapperSelect_5200(i, true);
    }
    if (b == 1) {  // Next Mapper (press)
      if (i == (a5200mapcount - 1))
        i = 0;
      else
        i++;

      // Only update display after input because of slow LCD library
      displayMapperSelect_5200(i, true);

    }
    if (b == 3) {  // Long Press - Execute (hold)
      new5200mapper = a5200mapselect;
      break;
    }
  }
  display.setCursor(0, 56);
  print_Msg(F("MAPPER "));
  print_Msg(new5200mapper);
  println_Msg(F(" SELECTED"));
  display_Update();
  delay(1000);
#else
setmapper:
  String newmap;
  Serial.println(F("SUPPORTED MAPPERS:"));
  Serial.println(F("0 = Standard 4K/8K/16K/32K"));
  Serial.println(F("1 = Two Chip 16K"));
  Serial.println(F("2 = Bounty Bob Strikes Back 40K"));
  Serial.print(F("Enter Mapper [0-2]: "));
  while (Serial.available() == 0) {}
  newmap = Serial.readStringUntil('\n');
  Serial.println(newmap);
  a5200index = newmap.toInt() * 3;
  new5200mapper = pgm_read_byte(a5200mapsize + a5200index);
#endif
  EEPROM_writeAnything(7, new5200mapper);
  a5200mapper = new5200mapper;
}

//******************************************
// CART SELECT CODE
//******************************************

FsFile a5200csvFile;
char a5200game[39];                    // title
char a5200mm[3];                       // mapper
char a5200rr[3];                       // romsize
char a5200ll[4];                       // linelength (previous line)
unsigned long a5200csvpos;             // CSV File Position
char a5200cartCSV[] = "5200.txt";      // CSV List
char a5200csvEND[] = "EOF";            // CSV End Marker for scrolling

bool readLine_5200(FsFile& f, char* line, size_t maxLen) {
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

bool readVals_5200(char* a5200game, char* a5200mm, char* a5200rr, char* a5200ll) {
  char line[44];
  a5200csvpos = a5200csvFile.position();
  if (!readLine_5200(a5200csvFile, line, sizeof(line))) {
    return false;  // EOF or too long
  }
  char* comma = strtok(line, ",");
  int x = 0;
  while (comma != NULL) {
    if (x == 0)
      strcpy(a5200game, comma);
    else if (x == 1)
      strcpy(a5200mm, comma);
    else if (x == 2)
      strcpy(a5200rr, comma);
    else if (x == 3)
      strcpy(a5200ll, comma);
    comma = strtok(NULL, ",");
    x += 1;
  }
  return true;
}

bool getCartListInfo_5200() {
  bool buttonreleased = 0;
  bool cartselected = 0;
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(F(" HOLD TO FAST CYCLE"));
  display_Update();
#else
  Serial.println(F("HOLD BUTTON TO FAST CYCLE"));
#endif
  delay(2000);
#if defined(ENABLE_OLED)
  buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
  boolean buttonVal1 = (PING & (1 << 2));  //PG2
#endif
  if (buttonVal1 == LOW) {         // Button Held - Fast Cycle
    while (1) {                    // Scroll Game List
      while (readVals_5200(a5200game, a5200mm, a5200rr, a5200ll)) {
        if (strcmp(a5200csvEND, a5200game) == 0) {
          a5200csvFile.seek(0);  // Restart
        } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
          display_Clear();
          println_Msg(F("CART TITLE:"));
          println_Msg(FS(FSTRING_EMPTY));
          println_Msg(a5200game);
          display_Update();
#else
          Serial.print(F("CART TITLE:"));
          Serial.println(a5200game);
#endif
#if defined(ENABLE_OLED)
          buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
          buttonVal1 = (PING & (1 << 2));  //PG2
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
#if defined(ENABLE_OLED)
      buttonVal1 = (PIND & (1 << 7));  // PD7
#elif defined(ENABLE_LCD)
      buttonVal1 = (PING & (1 << 2));      //PG2
#endif
      if (buttonVal1 == HIGH)          // Button Released
        break;
    }
  }
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display.setCursor(0, 56);
  println_Msg(F("FAST CYCLE OFF"));
  display_Update();
#else
  Serial.println(FS(FSTRING_EMPTY));
  Serial.println(F("FAST CYCLE OFF"));
  Serial.println(F("PRESS BUTTON TO STEP FORWARD"));
  Serial.println(F("DOUBLE CLICK TO STEP BACK"));
  Serial.println(F("HOLD TO SELECT"));
  Serial.println(FS(FSTRING_EMPTY));
#endif
  while (readVals_5200(a5200game, a5200mm, a5200rr, a5200ll)) {
    if (strcmp(a5200csvEND, a5200game) == 0) {
      a5200csvFile.seek(0);  // Restart
    } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
      display_Clear();
      println_Msg(F("CART TITLE:"));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(a5200game);
      display.setCursor(0, 48);
#if defined(ENABLE_OLED)
      print_STR(press_to_change_STR, 1);
      print_STR(right_to_select_STR, 1);
#elif defined(ENABLE_LCD)
      print_STR(rotate_to_change_STR, 1);
      print_STR(press_to_select_STR, 1);
#endif
      display_Update();
#else
      Serial.print(F("CART TITLE:"));
      Serial.println(a5200game);
#endif
      while (1) {  // Single Step
        uint8_t b = checkButton();
        if (b == 1) {  // Continue (press)
          break;
        }
        if (b == 2) {  // Reset to Start of List (doubleclick)
          byte prevline = strtol(a5200ll, NULL, 10);
          a5200csvpos -= prevline;
          a5200csvFile.seek(a5200csvpos);
          break;
        }
        if (b == 3) {  // Long Press - Select Cart (hold)
          new5200mapper = strtol(a5200mm, NULL, 10);
          new5200size = strtol(a5200rr, NULL, 10);
          EEPROM_writeAnything(7, new5200mapper);
          EEPROM_writeAnything(8, new5200size);
          cartselected = 1;  // SELECTION MADE
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
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
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(FS(FSTRING_END_OF_FILE));
  display_Update();
#else
  Serial.println(FS(FSTRING_END_OF_FILE));
#endif

  return false;
}

void checkCSV_5200() {
  if (getCartListInfo_5200()) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(FS(FSTRING_CART_SELECTED));
    println_Msg(FS(FSTRING_EMPTY));
    println_Msg(a5200game);
    display_Update();
    // Display Settings
    display.setCursor(0, 56);
    print_Msg(F("CODE: M"));
    print_Msg(new5200mapper);
    print_Msg(F("/R"));
    println_Msg(new5200size);
    display_Update();
#else
    Serial.println(FS(FSTRING_EMPTY));
    Serial.println(FS(FSTRING_CART_SELECTED));
    Serial.println(a5200game);
    // Display Settings
    Serial.print(F("CODE: M"));
    Serial.print(new5200mapper);
    Serial.print(F("/R"));
    Serial.println(new5200size);
    Serial.println(FS(FSTRING_EMPTY));
#endif
  } else {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display.setCursor(0, 56);
    println_Msg(FS(FSTRING_NO_SELECTION));
    display_Update();
#else
    Serial.println(FS(FSTRING_NO_SELECTION));
#endif
  }
}

void checkSize_5200() {
  EEPROM_readAnything(7, a5200mapper);
  for (int i = 0; i < a5200mapcount; i++) {
    a5200index = i * 3;
    if (a5200mapper == pgm_read_byte(a5200mapsize + a5200index)) {
      a5200size = pgm_read_byte(a5200mapsize + a5200index + 1);
      EEPROM_writeAnything(8, a5200size);
      break;
    }
  }
}

void setCart_5200() {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display_Clear();
  println_Msg(a5200cartCSV);
  display_Update();
#endif
  sd.chdir();
  sprintf(folder, "5200/CSV");
  sd.chdir(folder);  // Switch Folder
  a5200csvFile = sd.open(a5200cartCSV, O_READ);
  if (!a5200csvFile) {
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
    display_Clear();
    println_Msg(F("CSV FILE NOT FOUND!"));
    display_Update();
#else
    Serial.println(F("CSV FILE NOT FOUND!"));
#endif
    while (1) {
      if (checkButton() != 0)
        setup_5200();
    }
  }
  checkCSV_5200();
  a5200csvFile.close();
}
#endif
