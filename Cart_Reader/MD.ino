//******************************************
// SEGA MEGA DRIVE MODULE
//******************************************
// Writes to Sega CD Backup RAM Cart require an extra wire from MRES (B02) to VRES (B27)
#ifdef enable_MD

/******************************************
   Variables
 *****************************************/
unsigned long sramEnd;
word eepSize;
word addrhi;
word addrlo;
word chksum;
boolean is32x = 0;

//***********************************************
// EEPROM SAVE TYPES
// 1 = Acclaim Type 1    [24C02]
// 2 = Acclaim Type 2    [24C02/24C16/24C65]
// 3 = Capcom/SEGA       [24C01]
// 4 = EA                [24C01]
// 5 = Codemasters       [24C08/24C16/24C65]
//***********************************************
byte eepType;

//*********************************************************
// SERIAL EEPROM LOOKUP TABLE
// Format = {chksum, eepType | eepSize}
// chksum is located in ROM at 0x18E (0xC7)
// eepType and eepSize are combined to conserve memory
//*********************************************************
static const word PROGMEM eepid[] = {
  // ACCLAIM TYPE 1
  0x5B9F, 0x101,  // NBA Jam (J)
  0x694F, 0x101,  // NBA Jam (UE) (Rev 0)
  0xBFA9, 0x101,  // NBA Jam (UE) (Rev 1)
  // ACCLAIM TYPE 2
  0x16B2, 0x102,   // Blockbuster World Videogame Championship II (U)   [NO HEADER SAVE DATA]
  0xCC3F, 0x102,   // NBA Jam Tournament Edition (W) (Rev 0)            [NO HEADER SAVE DATA]
  0x8AE1, 0x102,   // NBA Jam Tournament Edition (W) (Rev 1)            [NO HEADER SAVE DATA]
  0xDB97, 0x102,   // NBA Jam Tournament Edition 32X (W)
  0x7651, 0x102,   // NFL Quarterback Club (W)
  0xDFE4, 0x102,   // NFL Quarterback Club 32X (W)
  0x3DE6, 0x802,   // NFL Quarterback Club '96 (UE)
  0xCB78, 0x2002,  // Frank Thomas Big Hurt Baseball (UE)
  0x6DD9, 0x2002,  // College Slam (U)
  // CAPCOM
  0xAD23, 0x83,  // Mega Man:  The Wily Wars (E)
  0xEA80, 0x83,  // Rockman Megaworld (J)
  // SEGA
  0x760F, 0x83,  // Evander "Real Deal" Holyfield Boxing (JU)
  0x95E7, 0x83,  // Greatest Heavyweights of the Ring (E)
  0x0000, 0x83,  // Greatest Heavyweights of the Ring (J)             [BLANK CHECKSUM 0000]
  0x7270, 0x83,  // Greatest Heavyweights of the Ring (U)
  0xBACC, 0x83,  // Honoo no Toukyuuji Dodge Danpei (J)
  0xB939, 0x83,  // MLBPA Sports Talk Baseball (U)                    [BAD HEADER SAVE DATA]
  0x487C, 0x83,  // Ninja Burai Densetsu (J)
  0x740D, 0x83,  // Wonder Boy in Monster World (B)
  0x0278, 0x83,  // Wonder Boy in Monster World (J)
  0x9D79, 0x83,  // Wonder Boy in Monster World (UE)
  // EA
  0x8512, 0x84,  // Bill Walsh College Football (UE)                  [BAD HEADER SAVE DATA]
  0xA107, 0x84,  // John Madden Football '93 (UE)                     [NO HEADER SAVE DATA]
  0x5807, 0x84,  // John Madden Football '93 Championship Edition (U) [NO HEADER SAVE DATA]
  0x2799, 0x84,  // NHLPA Hockey '93 (UE) (Rev 0)                     [NO HEADER SAVE DATA]
  0xFA57, 0x84,  // NHLPA Hockey '93 (UE) (Rev 1)                     [NO HEADER SAVE DATA]
  0x8B9F, 0x84,  // Rings of Power (UE)                               [NO HEADER SAVE DATA]
  // CODEMASTERS
  0x7E65, 0x405,   // Brian Lara Cricket (E)                            [NO HEADER SAVE DATA]
  0x9A5C, 0x2005,  // Brian Lara Cricket 96 (E) (Rev 1.0)               [NO HEADER SAVE DATA]
  0xC4EE, 0x2005,  // Brian Lara Cricket 96 (E) (Rev 1.1)               [NO HEADER SAVE DATA]
  0x7E50, 0x805,   // Micro Machines 2 (E) (J-Cart)                     [NO HEADER SAVE DATA]
  0x165E, 0x805,   // Micro Machines '96 (E) (J-Cart) (Rev 1.0/1.1)     [NO HEADER SAVE DATA]
  0x168B, 0x405,   // Micro Machines Military (E) (J-Cart)              [NO HEADER SAVE DATA]
  0x12C1, 0x2005,  // Shane Warne Cricket (E)                           [NO HEADER SAVE DATA]
};

byte eepcount = (sizeof(eepid) / sizeof(eepid[0])) / 2;
word eepdata;

// CD BACKUP RAM
unsigned long bramSize = 0;

// REALTEC MAPPER
boolean realtec = 0;

#define DEFAULT_VALUE_segaSram16bit 0
int segaSram16bit = DEFAULT_VALUE_segaSram16bit;

//*****************************************
// SONIC & KNUCKLES LOCK-ON MODE VARIABLES
// SnKmode :
// 0 = Not Sonic & Knuckles
// 1 = Only Sonic & Knucles
// 2 = Sonic & Knucles + Sonic1
// 3 = Sonic & Knucles + Sonic2
// 4 = Sonic & Knucles + Sonic3
// 5 = Sonic & Knucles + Other game
//*****************************************
static byte SnKmode = 0;
static unsigned long cartSizeLockon;
static unsigned long cartSizeSonic2 = 262144;
static word chksumLockon;
static word chksumSonic2 = 0x0635;

/******************************************
   Configuration
 *****************************************/
#ifdef use_md_conf
void mdLoadConf() {
  if (myFile.open("mdconf.txt", O_READ)) {
    char line[64];
    int n;
    int i;
    while ((n = myFile.fgets(line, sizeof(line) - 1)) > 0) {
      // preprocess
      for (i = 0; i < n; i++) {
        if (line[i] == ';') {
          // comments
          line[i] = '\0';
          n = i;
          break;
        } else if (line[i] == '\n' || line[i] == '\r') {
          // EOL
          line[n] = '\0';
          n = i;
          break;
        }
      }
      //print_Msg(F("read line: "));
      //println_Msg(line);
      if (line[0] == '[') {
        char* name;
        char* value;
        i = 1;
        name = line + i;
        for (; i < sizeof(line); i++) {
          if (line[i] == ']') {
            line[i] = '\0';
            i++;
            break;
          }
        }
        if (line[i] != '\0') {
          for (; i < sizeof(line); i++) {
            if (line[i] != ' ') {
              value = line + i;
              i++;
              break;
            }
          }
          for (; i < sizeof(line) && line[i] != '\0'; i++) {
            if (line[i] == ' ') {
              line[i] = '\0';
              break;
            }
          }
        }
        //print_Msg(F("read name: "));
        //println_Msg(name);
        //print_Msg(F("value: "));
        //println_Msg(value);
        if (!strcmp("segaSram16bit", name)) {
          // Retrode compatible setting
          // [segaSram16bit] 1   ; 0=no, 1=yes, 2=y+large
          // 0: Output each byte once. Not supported by most emulators.
          // 1: Duplicate each byte. Usable by Kega Fusion.
          // 2: Duplicate each byte. Pad with 0xFF so that the file size is 64KB.
          segaSram16bit = atoi(value);
          if (segaSram16bit != 0 && segaSram16bit != 1 && segaSram16bit != 2) {
            segaSram16bit = DEFAULT_VALUE_segaSram16bit;
          }
          print_Msg(F("segaSram16bit: "));
          println_Msg(segaSram16bit);
        }
      }
    }
    myFile.close();
  }
}
#endif

/******************************************
   Menu
 *****************************************/
// MD menu items
static const char MDMenuItem1[] PROGMEM = "Game Cartridge";
static const char MDMenuItem2[] PROGMEM = "SegaCD RamCart";
static const char MDMenuItem3[] PROGMEM = "Flash Repro";
//static const char MDMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsMD[] PROGMEM = { MDMenuItem1, MDMenuItem2, MDMenuItem3, string_reset2 };

// Cart menu items
static const char MDCartMenuItem1[] PROGMEM = "Read Rom";
static const char MDCartMenuItem2[] PROGMEM = "Read Sram";
static const char MDCartMenuItem3[] PROGMEM = "Write Sram";
static const char MDCartMenuItem4[] PROGMEM = "Read EEPROM";
static const char MDCartMenuItem5[] PROGMEM = "Write EEPROM";
static const char MDCartMenuItem6[] PROGMEM = "Cycle cart";
//static const char MDCartMenuItem7[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsMDCart[] PROGMEM = { MDCartMenuItem1, MDCartMenuItem2, MDCartMenuItem3, MDCartMenuItem4, MDCartMenuItem5, MDCartMenuItem6, string_reset2 };

// Sega CD Ram Backup Cartridge menu items
static const char SCDMenuItem1[] PROGMEM = "Read Backup RAM";
static const char SCDMenuItem2[] PROGMEM = "Write Backup RAM";
//static const char SCDMenuItem3[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsSCD[] PROGMEM = { SCDMenuItem1, SCDMenuItem2, string_reset2 };

// Sega start menu
void mdMenu() {
  vselect(false);
  // create menu with title and 4 options to choose from
  unsigned char mdDev;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsMD, 4);
  mdDev = question_box(F("Select MD device"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mdDev) {
    case 0:
      display_Clear();
      display_Update();
      setup_MD();
      mode = mode_MD_Cart;
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_MD();
      mode = mode_SEGA_CD;
      break;

#ifdef enable_FLASH
    case 2:
      display_Clear();
      display_Update();
      setup_MD();
      mode = mode_MD_Cart;
      // Change working dir to root
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser(F("Select file"));
      display_Clear();
      // Setting CS(PH3) LOW
      PORTH &= ~(1 << 3);

      // ID flash
      resetFlash_MD();
      idFlash_MD();
      resetFlash_MD();
      print_Msg(F("Flash ID: "));
      println_Msg(flashid_str);
      if (flashid == 0xC2F1) {
        println_Msg(F("MX29F1610 detected"));
        flashSize = 2097152;
      } else {
        print_FatalError(F("Error: Unknown flashrom"));
      }
      display_Update();

      eraseFlash_MD();
      resetFlash_MD();
      blankcheck_MD();
      write29F1610_MD();
      resetFlash_MD();
      delay(1000);
      resetFlash_MD();
      delay(1000);
      verifyFlash_MD();
      // Set CS(PH3) HIGH
      PORTH |= (1 << 3);
      println_Msg(F(""));
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
      break;
#endif

    case 3:
      resetArduino();
      break;
  }
}

void mdCartMenu() {
  // create menu with title and 6 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsMDCart, 7);
  mainMenu = question_box(F("MEGA DRIVE Reader"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();

      // common ROM read fail state: no cart inserted - tends to report impossibly large cartSize
      // largest known game so far is supposedly "Paprium" at 10MB, so cap sanity check at 16MB
      if (cartSize != 0 && cartSize <= 16777216) {
        // Change working dir to root
        sd.chdir("/");
        if (realtec) {
          readRealtec_MD();
        } else {
          readROM_MD();
          // Calculate and compare CRC32 with nointro
          if (is32x)
            //database, crcString, renamerom, offset
            compareCRC("32x.txt", 0, 1, 0);
          else
            compareCRC("md.txt", 0, 1, 0);
        }
      } else {
        print_Error(F("Cart has no ROM"));
      }
#ifdef global_log
      save_log();
#endif
      break;

    case 1:
      display_Clear();
      // Does cartridge have SRAM
      if ((saveType == 1) || (saveType == 2) || (saveType == 3)) {
        // Change working dir to root
        sd.chdir("/");
        println_Msg(F("Reading Sram..."));
        display_Update();
        enableSram_MD(1);
        readSram_MD();
        enableSram_MD(0);
      } else {
        print_Error(F("Cart has no Sram"));
      }
      break;

    case 2:
      display_Clear();
      // Does cartridge have SRAM
      if ((saveType == 1) || (saveType == 2) || (saveType == 3)) {
        // Change working dir to root
        sd.chdir("/");
        // Launch file browser
        fileBrowser(F("Select srm file"));
        display_Clear();
        enableSram_MD(1);
        writeSram_MD();
        writeErrors = verifySram_MD();
        enableSram_MD(0);
        if (writeErrors == 0) {
          println_Msg(F("Sram verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(writeErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else {
        print_Error(F("Cart has no Sram"));
      }
      break;

    case 3:
      display_Clear();
      if (saveType == 4)
        readEEP_MD();
      else {
        print_Error(F("Cart has no EEPROM"));
      }
      break;

    case 4:
      display_Clear();
      if (saveType == 4) {
        // Launch file browser
        fileBrowser(F("Select eep file"));
        display_Clear();
        writeEEP_MD();
      } else {
        print_Error(F("Cart has no EEPROM"));
      }
      break;

    case 5:
      // For multi-game carts
      // Set reset pin to output (PH0)
      DDRH |= (1 << 0);
      // Switch RST(PH0) to LOW
      PORTH &= ~(1 << 0);

      display_Clear();
      print_Msg(F("Resetting..."));
      display_Update();
      delay(3000);  // wait 3 secs to switch to next game
      resetArduino();
      break;

    case 6:
      // Reset
      resetArduino();
      break;
  }
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void segaCDMenu() {
  // create menu with title and 3 options to choose from
  unsigned char scdMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSCD, 3);
  scdMenu = question_box(F("SEGA CD RAM"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (scdMenu) {
    case 0:
      display_Clear();
      if (bramSize > 0)
        readBram_MD();
      else {
        print_Error(F("Not CD Backup RAM Cart"));
      }
      break;

    case 1:
      display_Clear();
      if (bramSize > 0) {
        // Launch file browser
        fileBrowser(F("Select brm file"));
        display_Clear();
        writeBram_MD();
      } else {
        print_Error(F("Not CD Backup RAM Cart"));
      }
      break;

    case 2:
      // Reset
      asm volatile("  jmp 0");
      break;
  }
  println_Msg(F(""));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_MD() {
#ifdef use_md_conf
  mdLoadConf();
#endif

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) CLK(PH1) CS(PH3) WRH(PH4) WRL(PH5) OE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output
  DDRJ |= (1 << 0);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;

  // Setting RST(PH0) CLK(PH1) CS(PH3) WRH(PH4) WRL(PH5) OE(PH6) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Setting TIME(PJ0) HIGH
  PORTJ |= (1 << 0);

  delay(200);

  // Print all the info
  getCartInfo_MD();
}

/******************************************
   I/O Functions
 *****************************************/

/******************************************
  Low level functions
*****************************************/
void writeWord_MD(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);
  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);

  // Leave WR low for at least 200ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

word readWord_MD(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  NOP;

  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);
  // Setting OE(PH6) LOW
  PORTH &= ~(1 << 6);

  // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // Read
  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);
  // Setting OE(PH6) HIGH
  PORTH |= (1 << 6);

  // these 6x nop delays have been here before, unknown what they mean
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  return tempWord;
}

void writeFlash_MD(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t");

  // Switch WE(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WE(PH5)to HIGH
  PORTH |= (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

word readFlash_MD(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting OE(PH6) LOW
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  __asm__("nop\n\t");

  // Setting OE(PH6) HIGH
  PORTH |= (1 << 6);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempWord;
}

// Switch data pins to write
void dataOut_MD() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn_MD() {
  DDRC = 0x00;
  DDRA = 0x00;
}

/******************************************
  MEGA DRIVE functions
*****************************************/
byte copyToRomName_MD(char* output, const byte* input, byte length) {
  byte myLength = 0;

  for (byte i = 0; i < 48; i++) {
    if (
      (
        (input[i] >= '0' && input[i] <= '9') || (input[i] >= 'A' && input[i] <= 'z'))
      && myLength < length) {
      output[myLength++] = input[i];
    }
  }

  return myLength;
}

void getCartInfo_MD() {
  // Set control
  dataIn_MD();

  // Get cart size
  cartSize = ((long(readWord_MD(0xD2)) << 16) | readWord_MD(0xD3)) + 1;

  // Check for 32x compatibility
  if ((readWord_MD(0x104 / 2) == 0x2033) && (readWord_MD(0x106 / 2) == 0x3258))
    is32x = 1;
  else
    is32x = 0;

  // Get cart checksum
  chksum = readWord_MD(0xC7);

  // Get cart ID
  char id[15];
  memset(id, 0, 15);
  for (byte c = 0; c < 14; c += 2) {
    // split word
    word myWord = readWord_MD((0x180 + c) / 2);
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    id[c] = hiByte;
    id[c + 1] = loByte;
  }

  // Zero Wing Check
  if (cartSize == 0x80000) {
    switch (chksum) {
      case 0xD07D:            //Zero Wing (J) 8Mbit
        cartSize = 0x100000;  //1MB instead of 512KB
        chksum = 0xF204;
        break;
      case 0x95C9:            //Zero Wing (E) 8Mbit
        cartSize = 0x100000;  //1MB instead of 512KB
        break;
    }
  }

  // Super Street Fighter 2 + Demons of Asteborg Check
  if (cartSize == 0x400000) {
    switch (chksum) {
      // Super Street Fighter 2
      case 0xCE25:  // Super Street Fighter 2 (J) 40Mbit
      case 0xE41D:  // Super Street Fighter 2 (E) 40Mbit
      case 0xE017:  // Super Street Fighter 2 (U) 40Mbit
        cartSize = 0x500000;
        break;

      // Demons of Asteborg
      case 0x0000:  // Demons of Asteborg v1.0 (W) 120Mbit
        cartSize = 0xEAF2F4;
        break;

      case 0xBCBF:  // Demons of Asteborg v1.1 (W) 120Mbit
      case 0x6E1E:  // Demons of Asteborg v1.11 (W) 120Mbit
        cartSize = 0xEA0000;
        break;
    }
  }

  // Beggar Prince rev.1 Check
  if (!strncmp("SF-001", id, 6) && (chksum == 0x3E08)) {
    cartSize = 0x400000;
  }

  // Legend of Wukong Check
  if (!strncmp("SF-002", id, 6) && (chksum == 0x12B0)) {
    chksum = 0x45C6;
  }

  // Sonic & Knuckles Check
  SnKmode = 0;
  if (chksum == 0xDFB3) {

    //Sonic & Knuckles ID:GM MK-1563 -00
    if (!strcmp("GM MK-1563 -00", id)) {
      char labelLockon[17];
      memset(labelLockon, 0, 17);

      // Get labelLockon
      for (byte c = 0; c < 16; c += 2) {
        // split word
        word myWord = readWord_MD((0x200100 + c) / 2);
        byte loByte = myWord & 0xFF;
        byte hiByte = myWord >> 8;

        // write to buffer
        labelLockon[c] = hiByte;
        labelLockon[c + 1] = loByte;
      }

      // check Lock-on game presence
      if (!(strcmp("SEGA MEGA DRIVE ", labelLockon) & strcmp("SEGA GENESIS    ", labelLockon))) {
        char idLockon[15];
        memset(idLockon, 0, 15);

        // Lock-on cart checksum
        chksumLockon = readWord_MD(0x1000C7);
        // Lock-on cart size
        cartSizeLockon = ((long(readWord_MD(0x1000D2)) << 16) | readWord_MD(0x1000D3)) + 1;

        // Get IdLockon
        for (byte c = 0; c < 14; c += 2) {
          // split word
          word myWord = readWord_MD((0x200180 + c) / 2);
          byte loByte = myWord & 0xFF;
          byte hiByte = myWord >> 8;

          // write to buffer
          idLockon[c] = hiByte;
          idLockon[c + 1] = loByte;
        }

        if (!strncmp("GM 00001009-0", idLockon, 13) || !strncmp("GM 00004049-0", idLockon, 13)) {
          //Sonic1 ID:GM 00001009-0? or GM 00004049-0?
          SnKmode = 2;
        } else if (!strcmp("GM 00001051-00", idLockon) || !strcmp("GM 00001051-01", idLockon) || !strcmp("GM 00001051-02", idLockon)) {
          //Sonic2 ID:GM 00001051-00 or GM 00001051-01 or GM 00001051-02
          SnKmode = 3;

          // Prepare Sonic2 Banks
          writeSSF2Map(0x509878, 1);  // 0xA130F1

        } else if (!strcmp("GM MK-1079 -00", idLockon)) {
          //Sonic3 ID:GM MK-1079 -00
          SnKmode = 4;
        } else {
          //Other game
          SnKmode = 5;
        }

      } else {
        SnKmode = 1;
      }
    }
  }

  // Serial EEPROM Check
  for (int i = 0; i < eepcount; i++) {
    int index = i * 2;
    word eepcheck = pgm_read_word(eepid + index);
    if (eepcheck == chksum) {
      eepdata = pgm_read_word(eepid + index + 1);
      eepType = eepdata & 0x7;
      eepSize = eepdata & 0xFFF8;
      saveType = 4;  // SERIAL EEPROM
      break;
    }
  }

  // Greatest Heavyweights of the Ring (J) has blank chksum 0x0000
  // Other non-save carts might have the same blank chksum
  // Check header for Serial EEPROM Data
  if (chksum == 0x0000) {
    if (readWord_MD(0xD9) != 0xE840) {  // NOT SERIAL EEPROM
      eepType = 0;
      eepSize = 0;
      saveType = 0;
    }
  }

  // Codemasters EEPROM Check
  // Codemasters used the same incorrect header across multiple carts
  // Carts with checksum 0x165E or 0x168B could be EEPROM or non-EEPROM
  // Check the first DWORD in ROM (0x444E4C44) to identify the EEPROM carts
  if ((chksum == 0x165E) || (chksum == 0x168B)) {
    if (readWord_MD(0x00) != 0x444E) {  // NOT SERIAL EEPROM
      eepType = 0;
      eepSize = 0;
      saveType = 0;
    }
  }

  // CD Backup RAM Cart Check
  // 4 = 128KB (2045 Blocks) Sega CD Backup RAM Cart
  // 6 = 512KB (8189 Blocks) Ultra CD Backup RAM Cart (Aftermarket)
  word bramCheck = readWord_MD(0x00);
  if ((((bramCheck & 0xFF) == 0x04) && ((chksum & 0xFF) == 0x04)) || (((bramCheck & 0xFF) == 0x06) && ((chksum & 0xFF) == 0x06))) {
    unsigned long p = 1 << (bramCheck & 0xFF);
    bramSize = p * 0x2000L;
  }
  if (saveType != 4) {  // NOT SERIAL EEPROM
    // Check if cart has sram
    saveType = 0;
    sramSize = 0;

    // FIXED CODE FOR SRAM/FRAM/PARALLEL EEPROM
    // 0x5241F820 SRAM (ODD BYTES/EVEN BYTES)
    // 0x5241F840 PARALLEL EEPROM - READ AS SRAM
    // 0x5241E020 SRAM (BOTH BYTES)
    if (readWord_MD(0xD8) == 0x5241) {
      word sramType = readWord_MD(0xD9);
      if ((sramType == 0xF820) || (sramType == 0xF840)) {  // SRAM/FRAM ODD/EVEN BYTES
        // Get sram start and end
        sramBase = ((long(readWord_MD(0xDA)) << 16) | readWord_MD(0xDB));
        sramEnd = ((long(readWord_MD(0xDC)) << 16) | readWord_MD(0xDD));
        if (sramBase == 0x20000020 && sramEnd == 0x00010020) {  // Fix for Psy-o-blade
          sramBase = 0x200001;
          sramEnd = 0x203fff;
        }
        // Check alignment of sram
        // 0x300001 for HARDBALL '95 (U)
        // 0x3C0001 for Legend of Wukong (Aftermarket)
        if ((sramBase == 0x200001) || (sramBase == 0x300001) || (sramBase == 0x3C0001)) {
          // low byte
          saveType = 1;  // ODD
          sramSize = (sramEnd - sramBase + 2) / 2;
          // Right shift sram base address so [A21] is set to high 0x200000 = 0b001[0]00000000000000000000
          sramBase = sramBase >> 1;
        } else if (sramBase == 0x200000) {
          // high byte
          saveType = 2;  // EVEN
          sramSize = (sramEnd - sramBase + 1) / 2;
          // Right shift sram base address so [A21] is set to high 0x200000 = 0b001[0]00000000000000000000
          sramBase = sramBase / 2;
        } else {
          print_Msg(("sramType: "));
          print_Msg_PaddedHex16(sramType);
          println_Msg(F(""));
          print_Msg(("sramBase: "));
          print_Msg_PaddedHex32(sramBase);
          println_Msg(F(""));
          print_Msg(("sramEnd: "));
          print_Msg_PaddedHex32(sramEnd);
          println_Msg(F(""));
          print_FatalError(F("Unknown Sram Base"));
        }
      } else if (sramType == 0xE020) {  // SRAM BOTH BYTES
        // Get sram start and end
        sramBase = ((long(readWord_MD(0xDA)) << 16) | readWord_MD(0xDB));
        sramEnd = ((long(readWord_MD(0xDC)) << 16) | readWord_MD(0xDD));

        if (sramBase == 0x200001) {
          saveType = 3;  // BOTH
          sramSize = sramEnd - sramBase + 2;
          sramBase = sramBase >> 1;
        } else if (sramBase == 0x200000) {
          saveType = 3;  // BOTH
          sramSize = sramEnd - sramBase + 1;
          sramBase = sramBase >> 1;
        } else {
          print_Msg(("sramType: "));
          print_Msg_PaddedHex16(sramType);
          println_Msg(F(""));
          print_Msg(("sramBase: "));
          print_Msg_PaddedHex32(sramBase);
          println_Msg(F(""));
          print_Msg(("sramEnd: "));
          print_Msg_PaddedHex32(sramEnd);
          println_Msg(F(""));
          print_FatalError(F("Unknown Sram Base"));
        }
      }
    } else {
      // SRAM CARTS WITH BAD/MISSING HEADER SAVE DATA
      switch (chksum) {
        case 0xC2DB:     // Winter Challenge (UE)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x200FFF;
          break;

        case 0xD7B6:     // Buck Rogers: Countdown to Doomsday (UE)
        case 0xFE3E:     // NBA Live '98 (U)
        case 0xFDAD:     // NFL '94 starring Joe Montana (U)
        case 0x632E:     // PGA Tour Golf (UE) (Rev 1)
        case 0xD2BA:     // PGA Tour Golf (UE) (Rev 2)
        case 0x44FE:     // Super Hydlide (J)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x203FFF;
          break;

        case 0xDB5E:     // Might & Magic: Gates to Another World (UE) (Rev 1)
        case 0x3428:     // Starflight (UE) (Rev 0)
        case 0x43EE:     // Starflight (UE) (Rev 1)
          saveType = 3;  // BOTH
          sramBase = 0x200001;
          sramEnd = 0x207FFF;
          break;

        case 0xBF72:     // College Football USA '96 (U)
        case 0x72EF:     // FIFA Soccer '97 (UE)
        case 0xD723:     // Hardball III (U)
        case 0x06C1:     // Madden NFL '98 (U)
        case 0xDB17:     // NHL '96 (UE)
        case 0x5B3A:     // NHL '98 (U)
        case 0x2CF2:     // NFL Sports Talk Football '93 starring Joe Montana (UE)
        case 0xE9B1:     // Summer Challenge (U)
        case 0xEEE8:     // Test Drive II: The Duel (U)
          saveType = 1;  // ODD
          sramBase = 0x200001;
          sramEnd = 0x20FFFF;
          break;
      }
      if (saveType == 1) {
        sramSize = (sramEnd - sramBase + 2) / 2;
        sramBase = sramBase >> 1;
      } else if (saveType == 3) {
        sramSize = sramEnd - sramBase + 2;
        sramBase = sramBase >> 1;
      }
    }
  }

  // Get name
  for (byte c = 0; c < 48; c += 2) {
    // split word
    word myWord = readWord_MD((0x150 + c) / 2);
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }
  romName[copyToRomName_MD(romName, sdBuffer, sizeof(romName) - 1)] = 0;

  //Get Lock-on cart name
  if (SnKmode >= 2) {
    char romNameLockon[12];

    //Change romName
    strcpy(romName, "SnK_");

    for (byte c = 0; c < 48; c += 2) {
      // split word
      word myWord = readWord_MD((0x200150 + c) / 2);
      byte loByte = myWord & 0xFF;
      byte hiByte = myWord >> 8;

      // write to buffer
      sdBuffer[c] = hiByte;
      sdBuffer[c + 1] = loByte;
    }
    romNameLockon[copyToRomName_MD(romNameLockon, sdBuffer, sizeof(romNameLockon) - 1)] = 0;

    switch (SnKmode) {
      case 2: strcat(romName, "SONIC1"); break;
      case 3: strcat(romName, "SONIC2"); break;
      case 4: strcat(romName, "SONIC3"); break;
      case 5: strcat(romName, romNameLockon); break;
    }
  }

  // Realtec Mapper Check
  word realtecCheck1 = readWord_MD(0x3F080);  // 0x7E100 "SEGA" (BootROM starts at 0x7E000)
  word realtecCheck2 = readWord_MD(0x3F081);
  if ((realtecCheck1 == 0x5345) && (realtecCheck2 == 0x4741)) {
    realtec = 1;
    strcpy(romName, "Realtec");
    cartSize = 0x80000;
  }

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  if (bramCheck != 0x00FF) {
    print_Msg(F("bramCheck: "));
    print_Msg_PaddedHex16(bramCheck);
    println_Msg(F(""));
  }
  if (bramSize > 0) {
    print_Msg(F("bramSize(KB): "));
    println_Msg(bramSize >> 10);
  }
  print_Msg(F("Size: "));
  print_Msg(cartSize * 8 / 1024 / 1024);
  switch (SnKmode) {
    case 2:
    case 4:
    case 5:
      print_Msg(F("+"));
      print_Msg(cartSizeLockon * 8 / 1024 / 1024);
      break;
    case 3:
      print_Msg(F("+"));
      print_Msg(cartSizeLockon * 8 / 1024 / 1024);
      print_Msg(F("+"));
      print_Msg(cartSizeSonic2 * 8 / 1024 / 1024);
      break;
  }
  println_Msg(F(" MBit"));
  print_Msg(F("ChkS: "));
  print_Msg_PaddedHexByte((chksum >> 8));
  print_Msg_PaddedHexByte((chksum & 0x00ff));
  switch (SnKmode) {
    case 2:
    case 4:
    case 5:
      print_Msg(F("+"));
      print_Msg_PaddedHexByte((chksumLockon >> 8));
      print_Msg_PaddedHexByte((chksumLockon & 0x00ff));
      break;
    case 3:
      print_Msg(F("+"));
      print_Msg_PaddedHexByte((chksumLockon >> 8));
      print_Msg_PaddedHexByte((chksumLockon & 0x00ff));
      print_Msg(F("+"));
      print_Msg_PaddedHexByte((chksumSonic2 >> 8));
      print_Msg_PaddedHexByte((chksumSonic2 & 0x00ff));
      break;
  }
  println_Msg(F(""));
  if (saveType == 4) {
    print_Msg(F("Serial EEPROM: "));
    print_Msg(eepSize * 8 / 1024);
    println_Msg(F(" KBit"));
  } else {
    print_Msg(F("Sram: "));
    if (sramSize > 0) {
      print_Msg(sramSize * 8 / 1024);
      println_Msg(F(" KBit"));
    } else
      println_Msg(F("None"));
  }
  println_Msg(F(" "));

  // Wait for user input
#if (defined(enable_LCD) || defined(enable_OLED))
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
}

void writeSSF2Map(unsigned long myAddress, word myData) {
  dataOut_MD();

  // Set TIME(PJ0) HIGH
  PORTJ |= (1 << 0);

  // 0x50987E * 2 = 0xA130FD  Bank 6 (0x300000-0x37FFFF)
  // 0x50987F * 2 = 0xA130FF  Bank 7 (0x380000-0x3FFFFF)
  PORTL = (myAddress >> 16) & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTF = myAddress & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t");

  // Strobe TIME(PJ0) LOW to latch the data
  PORTJ &= ~(1 << 0);
  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 200ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Set TIME(PJ0) HIGH
  PORTJ |= (1 << 0);

  dataIn_MD();
}

// Read rom and save to the SD card
void readROM_MD() {
  // Checksum
  uint16_t calcCKS = 0;
  uint16_t calcCKSLockon = 0;
  uint16_t calcCKSSonic2 = 0;

  // Set control
  dataIn_MD();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".BIN");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "MD/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  byte buffer[1024] = { 0 };

  // get current time
  // unsigned long startTime = millis();

  // Phantasy Star/Beyond Oasis with 74HC74 and 74HC139 switch ROM/SRAM at address 0x200000
  if (0x200000 < cartSize && cartSize < 0x400000) {
    enableSram_MD(0);
  }

  // Prepare SSF2 Banks
  if (cartSize > 0x400000) {
    writeSSF2Map(0x50987E, 6);  // 0xA130FD
    writeSSF2Map(0x50987F, 7);  // 0xA130FF
  }
  byte offsetSSF2Bank = 0;
  word d = 0;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize);
  if (SnKmode >= 2) totalProgressBar += (uint32_t)cartSizeLockon;
  if (SnKmode == 3) totalProgressBar += (uint32_t)cartSizeSonic2;
  draw_progressbar(0, totalProgressBar);

  for (unsigned long currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 512) {
    // Blink led
    if (currBuffer % 16384 == 0)
      blinkLED();

    if (currBuffer == 0x200000) {
      writeSSF2Map(0x50987E, 8);  // 0xA130FD
      writeSSF2Map(0x50987F, 9);  // 0xA130FF
      offsetSSF2Bank = 1;
    }

    // Demons of Asteborg Additional Banks
    else if (currBuffer == 0x280000) {
      writeSSF2Map(0x50987E, 10);  // 0xA130FD
      writeSSF2Map(0x50987F, 11);  // 0xA130FF
      offsetSSF2Bank = 2;
    } else if (currBuffer == 0x300000) {
      writeSSF2Map(0x50987E, 12);  // 0xA130FD
      writeSSF2Map(0x50987F, 13);  // 0xA130FF
      offsetSSF2Bank = 3;
    } else if (currBuffer == 0x380000) {
      writeSSF2Map(0x50987E, 14);  // 0xA130FD
      writeSSF2Map(0x50987F, 15);  // 0xA130FF
      offsetSSF2Bank = 4;
    } else if (currBuffer == 0x400000) {
      writeSSF2Map(0x50987E, 16);  // 0xA130FD
      writeSSF2Map(0x50987F, 17);  // 0xA130FF
      offsetSSF2Bank = 5;
    } else if (currBuffer == 0x480000) {
      writeSSF2Map(0x50987E, 18);  // 0xA130FD
      writeSSF2Map(0x50987F, 19);  // 0xA130FF
      offsetSSF2Bank = 6;
    } else if (currBuffer == 0x500000) {
      writeSSF2Map(0x50987E, 20);  // 0xA130FD
      writeSSF2Map(0x50987F, 21);  // 0xA130FF
      offsetSSF2Bank = 7;
    } else if (currBuffer == 0x580000) {
      writeSSF2Map(0x50987E, 22);  // 0xA130FD
      writeSSF2Map(0x50987F, 23);  // 0xA130FF
      offsetSSF2Bank = 8;
    } else if (currBuffer == 0x600000) {
      writeSSF2Map(0x50987E, 24);  // 0xA130FD
      writeSSF2Map(0x50987F, 25);  // 0xA130FF
      offsetSSF2Bank = 9;
    } else if (currBuffer == 0x680000) {
      writeSSF2Map(0x50987E, 26);  // 0xA130FD
      writeSSF2Map(0x50987F, 27);  // 0xA130FF
      offsetSSF2Bank = 10;
    } else if (currBuffer == 0x700000) {
      writeSSF2Map(0x50987E, 28);  // 0xA130FD
      writeSSF2Map(0x50987F, 29);  // 0xA130FF
      offsetSSF2Bank = 11;
    }

    d = 0;

    for (int currWord = 0; currWord < 512; currWord++) {
      unsigned long myAddress = currBuffer + currWord - (offsetSSF2Bank * 0x80000);
      PORTF = myAddress & 0xFF;
      PORTK = (myAddress >> 8) & 0xFF;
      PORTL = (myAddress >> 16) & 0xFF;

      // Arduino running at 16Mhz -> one nop = 62.5ns
      NOP;
      // Setting CS(PH3) LOW
      PORTH &= ~(1 << 3);
      // Setting OE(PH6) LOW
      PORTH &= ~(1 << 6);
      // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;
      NOP;

      // Read
      buffer[d] = PINA;
      buffer[d + 1] = PINC;

      // Setting CS(PH3) HIGH
      PORTH |= (1 << 3);
      // Setting OE(PH6) HIGH
      PORTH |= (1 << 6);

      // Skip first 256 words
      if (((currBuffer == 0) && (currWord >= 256)) || (currBuffer > 0)) {
        calcCKS += ((buffer[d] << 8) | buffer[d + 1]);
      }
      d += 2;
    }
    myFile.write(buffer, 1024);

    // update progress bar
    processedProgressBar += 1024;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }
  if (SnKmode >= 2) {
    for (unsigned long currBuffer = 0; currBuffer < cartSizeLockon / 2; currBuffer += 512) {
      // Blink led
      if (currBuffer % 16384 == 0)
        blinkLED();

      d = 0;

      for (int currWord = 0; currWord < 512; currWord++) {
        unsigned long myAddress = currBuffer + currWord + cartSize / 2;
        PORTF = myAddress & 0xFF;
        PORTK = (myAddress >> 8) & 0xFF;
        PORTL = (myAddress >> 16) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        NOP;
        // Setting CS(PH3) LOW
        PORTH &= ~(1 << 3);
        // Setting OE(PH6) LOW
        PORTH &= ~(1 << 6);
        // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        // Read
        buffer[d] = PINA;
        buffer[d + 1] = PINC;

        // Setting CS(PH3) HIGH
        PORTH |= (1 << 3);
        // Setting OE(PH6) HIGH
        PORTH |= (1 << 6);

        // Skip first 256 words
        if (((currBuffer == 0) && (currWord >= 256)) || (currBuffer > 0)) {
          calcCKSLockon += ((buffer[d] << 8) | buffer[d + 1]);
        }
        d += 2;
      }
      myFile.write(buffer, 1024);

      // update progress bar
      processedProgressBar += 1024;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
  }
  if (SnKmode == 3) {
    for (unsigned long currBuffer = 0; currBuffer < cartSizeSonic2 / 2; currBuffer += 512) {
      // Blink led
      if (currBuffer % 16384 == 0)
        blinkLED();

      d = 0;

      for (int currWord = 0; currWord < 512; currWord++) {
        unsigned long myAddress = currBuffer + currWord + (cartSize + cartSizeLockon) / 2;
        PORTF = myAddress & 0xFF;
        PORTK = (myAddress >> 8) & 0xFF;
        PORTL = (myAddress >> 16) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        NOP;
        // Setting CS(PH3) LOW
        PORTH &= ~(1 << 3);
        // Setting OE(PH6) LOW
        PORTH &= ~(1 << 6);
        // most MD ROMs are 200ns, comparable to SNES > use similar access delay of 6 x 62.5 = 375ns
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;
        NOP;

        // Read
        buffer[d] = PINA;
        buffer[d + 1] = PINC;

        // Setting CS(PH3) HIGH
        PORTH |= (1 << 3);
        // Setting OE(PH6) HIGH
        PORTH |= (1 << 6);

        calcCKSSonic2 += ((buffer[d] << 8) | buffer[d + 1]);
        d += 2;
      }
      myFile.write(buffer, 1024);

      // update progress bar
      processedProgressBar += 1024;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
  }
  // Close the file:
  myFile.close();

  // Reset SSF2 Banks
  if (cartSize > 0x400000) {
    writeSSF2Map(0x50987E, 6);  // 0xA130FD
    writeSSF2Map(0x50987F, 7);  // 0xA130FF
  }

  // print elapsed time
  //print_Msg(F("Time elapsed: "));
  //print_Msg((millis() - startTime) / 1000);
  //println_Msg(F("s"));
  //display_Update();

  // Calculate internal checksum
  print_Msg(F("Internal checksum..."));
  display_Update();
  if (chksum == calcCKS) {
    println_Msg(F("OK"));
    display_Update();
  } else {
    println_Msg(F("Error"));
    char calcsumStr[5];
    sprintf(calcsumStr, "%04X", calcCKS);
    println_Msg(calcsumStr);
    print_Error(F(""));
    display_Update();
  }

  // More checksums
  if (SnKmode >= 2) {
    print_Msg(F("Lock-on checksum..."));
    if (chksumLockon == calcCKSLockon) {
      println_Msg(F("OK"));
      display_Update();
    } else {
      print_Msg(F("Error"));
      char calcsumStr[5];
      sprintf(calcsumStr, "%04X", calcCKSLockon);
      println_Msg(calcsumStr);
      print_Error(F(""));
      display_Update();
    }
  }
  if (SnKmode == 3) {
    print_Msg(F("Adittional checksum..."));
    if (chksumSonic2 == calcCKSSonic2) {
      println_Msg(F("OK"));
      display_Update();
    } else {
      print_Msg(F("Error"));
      char calcsumStr[5];
      sprintf(calcsumStr, "%04X", calcCKSSonic2);
      println_Msg(calcsumStr);
      print_Error(F(""));
      display_Update();
    }
  }
}

/******************************************
  SRAM functions
*****************************************/
// Sonic 3 sram enable
void enableSram_MD(boolean enableSram) {
  dataOut_MD();

  // Set D0 to either 1(enable SRAM) or 0(enable ROM)
  PORTC = enableSram;

  // Strobe TIME(PJ0) LOW to latch the data
  PORTJ &= ~(1 << 0);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  // Set TIME(PJ0) HIGH
  PORTJ |= (1 << 0);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  dataIn_MD();
}

// Write sram to cartridge
void writeSram_MD() {
  dataOut_MD();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Write to the lower byte
    if (saveType == 1) {
      for (unsigned long currByte = sramBase; currByte < sramBase + sramSize; currByte++) {
        if (segaSram16bit > 0) {
          // skip high byte
          myFile.read();
        }
        word data = myFile.read() & 0xFF;
        writeWord_MD(currByte, data);
      }
    }
    // Write to the upper byte
    else if (saveType == 2) {
      for (unsigned long currByte = sramBase; currByte < sramBase + sramSize; currByte++) {
        word data = (myFile.read() << 8) & 0xFF00;
        writeWord_MD(currByte, data);
        if (segaSram16bit > 0) {
          // skip low byte
          myFile.read();
        }
      }
    }
    // Write to both bytes
    else if (saveType == 3) {
      for (unsigned long currByte = sramBase; currByte < sramBase + sramSize; currByte++) {
        word data = (myFile.read() << 8) & 0xFF00;
        data |= (myFile.read() & 0xFF);
        writeWord_MD(currByte, data);
      }
    } else
      print_Error(F("Unknown save type"));

    // Close the file:
    myFile.close();
    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }
  dataIn_MD();
}

// Read sram and save to the SD card
void readSram_MD() {
  dataIn_MD();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".srm");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "MD/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  for (unsigned long currBuffer = sramBase; currBuffer < sramBase + sramSize; currBuffer += 256) {
    for (int currWord = 0; currWord < 256; currWord++) {
      word myWord = readWord_MD(currBuffer + currWord);

      if (saveType == 2) {
        // Only use the upper byte
        if (segaSram16bit > 0) {
          sdBuffer[(currWord * 2) + 0] = ((myWord >> 8) & 0xFF);
          sdBuffer[(currWord * 2) + 1] = ((myWord >> 8) & 0xFF);
        } else {
          sdBuffer[currWord] = ((myWord >> 8) & 0xFF);
        }
      } else if (saveType == 1) {
        // Only use the lower byte
        if (segaSram16bit > 0) {
          sdBuffer[(currWord * 2) + 0] = (myWord & 0xFF);
          sdBuffer[(currWord * 2) + 1] = (myWord & 0xFF);
        } else {
          sdBuffer[currWord] = (myWord & 0xFF);
        }
      } else if (saveType == 3) {  // BOTH
        sdBuffer[currWord * 2] = ((myWord >> 8) & 0xFF);
        sdBuffer[(currWord * 2) + 1] = (myWord & 0xFF);
      }
    }
    if (saveType == 3 || segaSram16bit > 0)
      myFile.write(sdBuffer, 512);
    else
      myFile.write(sdBuffer, 256);
  }
  if (segaSram16bit == 2) {
    // pad to 64KB
    for (int i = 0; i < 512; i++) {
      sdBuffer[i] = 0xFF;
    }
    unsigned long padsize = (1UL << 16) - (sramSize << 1);
    unsigned long padblockcount = padsize >> 9;  // number of 512 byte blocks
    for (unsigned long i = 0; i < padblockcount; i++) {
      myFile.write(sdBuffer, 512);
    }
  }
  // Close the file:
  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));
  display_Update();
}

unsigned long verifySram_MD() {
  dataIn_MD();
  writeErrors = 0;

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currBuffer = sramBase; currBuffer < sramBase + sramSize; currBuffer += 256) {
      for (int currWord = 0; currWord < 256; currWord++) {
        word myWord = readWord_MD(currBuffer + currWord);

        if (saveType == 2) {
          // Only use the upper byte
          sdBuffer[currWord * 2] = ((myWord >> 8) & 0xFF);
        } else if (saveType == 1) {
          // Only use the lower byte
          sdBuffer[currWord * 2] = (myWord & 0xFF);
        } else if (saveType == 3) {  // BOTH
          sdBuffer[(currWord * 2) + 0] = ((myWord >> 8) & 0xFF);
          sdBuffer[(currWord * 2) + 1] = (myWord & 0xFF);
        }
      }
      int step = saveType == 3 ? 1 : 2;
      // Check sdBuffer content against file on sd card
      for (int i = 0; i < 512; i += step) {
        if (saveType == 1 && segaSram16bit > 0) {
          // skip high byte
          myFile.read();
        }
        byte b = myFile.read();
        if (saveType == 2 && segaSram16bit > 0) {
          // skip low byte
          myFile.read();
        }
        if (b != sdBuffer[i]) {
          writeErrors++;
        }
      }
    }

    // Close the file:
    myFile.close();
  } else {
    print_FatalError(sd_error_STR);
  }
  // Return 0 if verified ok, or number of errors
  return writeErrors;
}

#ifdef enable_FLASH
//******************************************
// Flashrom Functions
//******************************************
void resetFlash_MD() {
  // Set data pins to output
  dataOut_MD();

  // Reset command sequence
  writeFlash_MD(0x5555, 0xaa);
  writeFlash_MD(0x2aaa, 0x55);
  writeFlash_MD(0x5555, 0xf0);

  // Set data pins to input again
  dataIn_MD();
}

void write29F1610_MD() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 0);
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize) {
      print_FatalError(file_too_big_STR);
    }
    // Set data pins to output
    dataOut_MD();

    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    int d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 4096 == 0) {
        blinkLED();
      }

      // Write command sequence
      writeFlash_MD(0x5555, 0xaa);
      writeFlash_MD(0x2aaa, 0x55);
      writeFlash_MD(0x5555, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 64; c++) {
        word currWord = ((sdBuffer[d] & 0xFF) << 8) | (sdBuffer[d + 1] & 0xFF);
        writeFlash_MD(currByte + c, currWord);
        d += 2;
      }
      d = 0;

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck_MD();
    }

    // Set data pins to input again
    dataIn_MD();

    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

void idFlash_MD() {
  // Set data pins to output
  dataOut_MD();

  // ID command sequence
  writeFlash_MD(0x5555, 0xaa);
  writeFlash_MD(0x2aaa, 0x55);
  writeFlash_MD(0x5555, 0x90);

  // Set data pins to input again
  dataIn_MD();

  // Read the two id bytes into a string
  flashid = (readFlash_MD(0) & 0xFF) << 8;
  flashid |= readFlash_MD(1) & 0xFF;
  sprintf(flashid_str, "%04X", flashid);
}

byte readStatusReg_MD() {
  // Set data pins to output
  dataOut_MD();

  // Status reg command sequence
  writeFlash_MD(0x5555, 0xaa);
  writeFlash_MD(0x2aaa, 0x55);
  writeFlash_MD(0x5555, 0x70);

  // Set data pins to input again
  dataIn_MD();

  // Read the status register
  byte statusReg = readFlash_MD(0);
  return statusReg;
}

void eraseFlash_MD() {
  // Set data pins to output
  dataOut_MD();

  // Erase command sequence
  writeFlash_MD(0x5555, 0xaa);
  writeFlash_MD(0x2aaa, 0x55);
  writeFlash_MD(0x5555, 0x80);
  writeFlash_MD(0x5555, 0xaa);
  writeFlash_MD(0x2aaa, 0x55);
  writeFlash_MD(0x5555, 0x10);

  // Set data pins to input again
  dataIn_MD();

  busyCheck_MD();
}

void blankcheck_MD() {
  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte++) {
    if (readFlash_MD(currByte) != 0xFFFF) {
      currByte = flashSize / 2;
      blank = 0;
    }
    if (currByte % 4096 == 0) {
      blinkLED();
    }
  }
  if (!blank) {
    print_Error(F("Error: Not blank"));
  }
}

void verifyFlash_MD() {
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize) {
      print_FatalError(file_too_big_STR);
    }

    blank = 0;
    word d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 256) {
      if (currByte % 4096 == 0) {
        blinkLED();
      }
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 256; c++) {
        word currWord = ((sdBuffer[d] << 8) | sdBuffer[d + 1]);

        if (readFlash_MD(currByte + c) != currWord) {
          blank++;
        }
        d += 2;
      }
      d = 0;
    }
    if (blank == 0) {
      println_Msg(F("Flashrom verified OK"));
      display_Update();
    } else {
      print_STR(error_STR, 0);
      print_Msg(blank);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}
#endif

// Delay between write operations based on status register
void busyCheck_MD() {
  // Set data pins to input
  dataIn_MD();

  // Read the status register
  word statusReg = readFlash_MD(0);

  while ((statusReg | 0xFF7F) != 0xFFFF) {
    statusReg = readFlash_MD(0);
  }

  // Set data pins to output
  dataOut_MD();
}

//******************************************
// EEPROM Functions
//******************************************
void EepromInit(byte eepmode) {  // Acclaim Type 2
  PORTF = 0x00;                  // ADDR A0-A7
  PORTK = 0x00;                  // ADDR A8-A15
  PORTL = 0x10;                  // ADDR A16-A23
  PORTA = 0x00;                  // DATA D8-D15
  PORTH |= (1 << 0);             // /RES HIGH

  PORTC = eepmode;                 // EEPROM Switch:  0 = Enable (Read EEPROM), 1 = Disable (Read ROM)
  PORTH &= ~(1 << 3);              // CE LOW
  PORTH &= ~(1 << 4) & ~(1 << 5);  // /UDSW + /LDSW LOW
  PORTH |= (1 << 6);               // OE HIGH
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
  PORTH |= (1 << 4) | (1 << 5);  // /UDSW + /LDSW HIGH
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

void writeWord_SDA(unsigned long myAddress, word myData) { /* D0 goes to /SDA when only /LWR is asserted */
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTH &= ~(1 << 3);  // CE LOW
  PORTH &= ~(1 << 5);  // /LDSW LOW
  PORTH |= (1 << 4);   // /UDSW HIGH
  PORTH |= (1 << 6);   // OE HIGH
  if (eepSize > 0x100)
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  else
    delayMicroseconds(100);
  PORTH |= (1 << 5);  // /LDSW HIGH
  if (eepSize > 0x100)
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  else
    delayMicroseconds(100);
}

void writeWord_SCL(unsigned long myAddress, word myData) { /* D0 goes to /SCL when only /UWR is asserted */
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTH &= ~(1 << 3);  // CE LOW
  PORTH &= ~(1 << 4);  // /UDSW LOW
  PORTH |= (1 << 5);   // /LDSW HIGH
  PORTH |= (1 << 6);   // OE HIGH
  if (eepSize > 0x100)
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  else
    delayMicroseconds(100);
  PORTH |= (1 << 4);  // /UDSW HIGH
  if (eepSize > 0x100)
    __asm__("nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t");
  else
    delayMicroseconds(100);
}

void writeWord_CM(unsigned long myAddress, word myData) {  // Codemasters
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t");

  // Switch WR(PH4) to LOW
  PORTH &= ~(1 << 4);
  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);
  // Pulse CLK(PH1)
  PORTH ^= (1 << 1);

  // Leave WR low for at least 200ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Pulse CLK(PH1)
  PORTH ^= (1 << 1);
  // Setting CS(PH3) HIGH
  PORTH |= (1 << 3);
  // Switch WR(PH4) to HIGH
  PORTH |= (1 << 4);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

// EEPROM COMMANDS
void EepromStart() {
  if (eepType == 2) {               // Acclaim Type 2
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low
  } else if (eepType == 4) {        // EA
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
    writeWord_MD(0x100000, 0xC0);   // sda, scl high
    writeWord_MD(0x100000, 0x40);   // sda low, scl high
    writeWord_MD(0x100000, 0x00);   // START
  } else if (eepType == 5) {        // Codemasters
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
    writeWord_CM(0x180000, 0x02);   // sda low, scl high
    writeWord_CM(0x180000, 0x03);   // sda, scl high
    writeWord_CM(0x180000, 0x02);   // sda low, scl high
    writeWord_CM(0x180000, 0x00);   // START
  } else {
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
    writeWord_MD(0x100000, 0x03);  // sda, scl high
    writeWord_MD(0x100000, 0x02);  // sda low, scl high
    writeWord_MD(0x100000, 0x00);  // START
  }
}

void EepromSet0() {
  if (eepType == 2) {               // Acclaim Type 2
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low
  } else if (eepType == 4) {        // EA
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
    writeWord_MD(0x100000, 0x40);   // sda low, scl high // 0
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
  } else if (eepType == 5) {        // Codemasters
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
    writeWord_CM(0x180000, 0x02);   // sda low, scl high // 0
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
  } else {
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
    writeWord_MD(0x100000, 0x02);  // sda low, scl high // 0
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
  }
}

void EepromSet1() {
  if (eepType == 2) {               // Acclaim Type 2
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x00);  // scl low
  } else if (eepType == 4) {        // EA
    writeWord_MD(0x100000, 0x80);   // sda high, scl low
    writeWord_MD(0x100000, 0xC0);   // sda high, scl high // 1
    writeWord_MD(0x100000, 0x80);   // sda high, scl low
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
  } else if (eepType == 5) {        // Codemasters
    writeWord_CM(0x180000, 0x01);   // sda high, scl low
    writeWord_CM(0x180000, 0x03);   // sda high, scl high // 1
    writeWord_CM(0x180000, 0x01);   // sda high, scl low
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
  } else {
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x03);  // sda high, scl high // 1
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
  }
}


void EepromDevice() {  // 24C02+
  EepromSet1();
  EepromSet0();
  EepromSet1();
  EepromSet0();
}

void EepromSetDeviceAddress(word addrhi) {  // 24C02+
  for (int i = 0; i < 3; i++) {
    if ((addrhi >> 2) & 0x1)  // Bit is HIGH
      EepromSet1();
    else  // Bit is LOW
      EepromSet0();
    addrhi <<= 1;  // rotate to the next bit
  }
}

void EepromStatus() {  // ACK
  byte eepStatus = 1;
  if (eepType == 1) {              // Acclaim Type 1
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x03);  // sda high, scl high
    do {
      dataIn_MD();
      eepStatus = ((readWord_MD(0x100000) >> 1) & 0x1);
      dataOut_MD();
      delayMicroseconds(4);
    } while (eepStatus == 1);
    writeWord_MD(0x100000, 0x01);   // sda high, scl low
  } else if (eepType == 2) {        // Acclaim Type 2
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x01);  // scl high
    do {
      dataIn_MD();
      eepStatus = (readWord_MD(0x100000) & 0x1);
      dataOut_MD();
      delayMicroseconds(4);
    } while (eepStatus == 1);
    writeWord_SCL(0x100000, 0x00);  // scl low
  } else if (eepType == 3) {        // Capcom/Sega
    writeWord_MD(0x100000, 0x01);   // sda high, scl low
    writeWord_MD(0x100000, 0x03);   // sda high, scl high
    do {
      dataIn_MD();
      eepStatus = (readWord_MD(0x100000) & 0x1);
      dataOut_MD();
      delayMicroseconds(4);
    } while (eepStatus == 1);
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
  } else if (eepType == 4) {       // EA
    writeWord_MD(0x100000, 0x80);  // sda high, scl low
    writeWord_MD(0x100000, 0xC0);  // sda high, scl high
    do {
      dataIn_MD();
      eepStatus = ((readWord_MD(0x100000) >> 7) & 0x1);
      dataOut_MD();
      delayMicroseconds(4);
    } while (eepStatus == 1);
    writeWord_MD(0x100000, 0x80);  // sda high, scl low
  } else if (eepType == 5) {       // Codemasters
    writeWord_CM(0x180000, 0x01);  // sda high, scl low
    writeWord_CM(0x180000, 0x03);  // sda high, scl high
    do {
      dataIn_MD();
      eepStatus = ((readWord_MD(0x1C0000) >> 7) & 0x1);
      dataOut_MD();
      delayMicroseconds(4);
    } while (eepStatus == 1);
    writeWord_CM(0x180000, 0x01);  // sda high, scl low
  }
}

void EepromReadMode() {
  EepromSet1();    // READ
  EepromStatus();  // ACK
}

void EepromWriteMode() {
  EepromSet0();    // WRITE
  EepromStatus();  // ACK
}

void EepromReadData() {
  if (eepType == 1) {  // Acclaim Type 1
    for (int i = 0; i < 8; i++) {
      writeWord_MD(0x100000, 0x03);  // sda high, scl high
      dataIn_MD();
      eepbit[i] = ((readWord_MD(0x100000) >> 1) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 1)
      dataOut_MD();
      writeWord_MD(0x100000, 0x01);  // sda high, scl low
    }
  } else if (eepType == 2) {  // Acclaim Type 2
    for (int i = 0; i < 8; i++) {
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x01);  // scl high
      dataIn_MD();
      eepbit[i] = (readWord_MD(0x100000) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 0)
      dataOut_MD();
      writeWord_SDA(0x100000, 0x01);  // sda high
      writeWord_SCL(0x100000, 0x00);  // scl low
    }
  } else if (eepType == 3) {  // Capcom/Sega
    for (int i = 0; i < 8; i++) {
      writeWord_MD(0x100000, 0x03);  // sda high, scl high
      dataIn_MD();
      eepbit[i] = (readWord_MD(0x100000) & 0x1);  // Read 0x100000 with Mask 0x1 (bit 0)
      dataOut_MD();
      writeWord_MD(0x100000, 0x01);  // sda high, scl low
    }
  } else if (eepType == 4) {  // EA
    for (int i = 0; i < 8; i++) {
      writeWord_MD(0x100000, 0xC0);  // sda high, scl high
      dataIn_MD();
      eepbit[i] = ((readWord_MD(0x100000) >> 7) & 0x1);  // Read 0x100000 with Mask (bit 7)
      dataOut_MD();
      writeWord_MD(0x100000, 0x80);  // sda high, scl low
    }
  } else if (eepType == 5) {  // Codemasters
    for (int i = 0; i < 8; i++) {
      writeWord_CM(0x180000, 0x03);  // sda high, scl high
      dataIn_MD();
      eepbit[i] = ((readWord_MD(0x1C0000) >> 7) & 0x1);  // Read 0x1C0000 with Mask 0x1 (bit 7)
      dataOut_MD();
      writeWord_CM(0x180000, 0x01);  // sda high, scl low
    }
  }
}

void EepromWriteData(byte data) {
  for (int i = 0; i < 8; i++) {
    if ((data >> 7) & 0x1)  // Bit is HIGH
      EepromSet1();
    else  // Bit is LOW
      EepromSet0();
    data <<= 1;  // rotate to the next bit
  }
  EepromStatus();  // ACK
}

void EepromFinish() {
  if (eepType == 2) {               // Acclaim Type 2
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x00);  // scl low
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x00);  // scl low
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low
  } else if (eepType == 4) {        // EA
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
    writeWord_MD(0x100000, 0x80);   // sda high, scl low
    writeWord_MD(0x100000, 0xC0);   // sda high, scl high
    writeWord_MD(0x100000, 0x80);   // sda high, scl low
    writeWord_MD(0x100000, 0x00);   // sda low, scl low
  } else if (eepType == 5) {        // Codemasters
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
    writeWord_CM(0x180000, 0x01);   // sda high, scl low
    writeWord_CM(0x180000, 0x03);   // sda high, scl high
    writeWord_CM(0x180000, 0x01);   // sda high, scl low
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
  } else {
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x03);  // sda high, scl high
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x00);  // sda low, scl low
  }
}

void EepromStop() {
  if (eepType == 2) {               // Acclaim Type 2
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x01);  // scl high
    writeWord_SDA(0x100000, 0x01);  // sda high
    writeWord_SCL(0x100000, 0x00);  // scl low
    writeWord_SDA(0x100000, 0x00);  // sda low
    writeWord_SCL(0x100000, 0x00);  // scl low // STOP
  } else if (eepType == 4) {        // EA
    writeWord_MD(0x100000, 0x00);   // sda, scl low
    writeWord_MD(0x100000, 0x40);   // sda low, scl high
    writeWord_MD(0x100000, 0xC0);   // sda, scl high
    writeWord_MD(0x100000, 0x80);   // sda high, scl low
    writeWord_MD(0x100000, 0x00);   // STOP
  } else if (eepType == 5) {        // Codemasters
    writeWord_CM(0x180000, 0x00);   // sda low, scl low
    writeWord_CM(0x180000, 0x02);   // sda low, scl high
    writeWord_CM(0x180000, 0x03);   // sda, scl high
    writeWord_CM(0x180000, 0x01);   // sda high, scl low
    writeWord_CM(0x180000, 0x00);   // STOP
  } else {
    writeWord_MD(0x100000, 0x00);  // sda, scl low
    writeWord_MD(0x100000, 0x02);  // sda low, scl high
    writeWord_MD(0x100000, 0x03);  // sda, scl high
    writeWord_MD(0x100000, 0x01);  // sda high, scl low
    writeWord_MD(0x100000, 0x00);  // STOP
  }
}

void EepromSetAddress(word address) {
  if (eepSize > 0x80) {  // 24C02+
    for (int i = 0; i < 8; i++) {
      if ((address >> 7) & 0x1)  // Bit is HIGH
        EepromSet1();
      else  // Bit is LOW
        EepromSet0();
      address <<= 1;  // rotate to the next bit
    }
    EepromStatus();  // ACK
  } else {           // 24C01
    for (int i = 0; i < 7; i++) {
      if ((address >> 6) & 0x1)  // Bit is HIGH
        EepromSet1();
      else  // Bit is LOW
        EepromSet0();
      address <<= 1;  // rotate to the next bit
    }
  }
}

void readEepromByte(word address) {
  addrhi = address >> 8;
  addrlo = address & 0xFF;
  dataOut_MD();
  if (eepType == 2)
    EepromInit(0);  // Enable EEPROM
  EepromStart();    // START
  if (eepSize > 0x80) {
    EepromDevice();         // DEVICE [1010]
    if (eepSize > 0x800) {  // MODE 3 [24C65]
      EepromSetDeviceAddress(0);
      EepromWriteMode();
      EepromSetAddress(addrhi);        // ADDR [A15..A8]
    } else {                           // MODE 2 [24C02/24C08/24C16]
      EepromSetDeviceAddress(addrhi);  // ADDR [A10..A8]
      EepromWriteMode();
    }
  }
  EepromSetAddress(addrlo);
  if (eepSize > 0x80) {
    EepromStart();        // START
    EepromDevice();       // DEVICE [1010]
    if (eepSize > 0x800)  // MODE 3 [24C65]
      EepromSetDeviceAddress(0);
    else                               // MODE 2 [24C02/24C08/24C16]
      EepromSetDeviceAddress(addrhi);  // ADDR [A10..A8]
  }
  EepromReadMode();
  EepromReadData();
  EepromFinish();
  EepromStop();  // STOP
  if (eepType == 2)
    EepromInit(1);  // Disable EEPROM
  // OR 8 bits into byte
  eeptemp = eepbit[0] << 7 | eepbit[1] << 6 | eepbit[2] << 5 | eepbit[3] << 4 | eepbit[4] << 3 | eepbit[5] << 2 | eepbit[6] << 1 | eepbit[7];
  sdBuffer[addrlo] = eeptemp;
}

void writeEepromByte(word address) {
  addrhi = address >> 8;
  addrlo = address & 0xFF;
  eeptemp = sdBuffer[addrlo];
  dataOut_MD();
  if (eepType == 2)
    EepromInit(0);  // Enable EEPROM
  EepromStart();    // START
  if (eepSize > 0x80) {
    EepromDevice();                    // DEVICE [1010]
    if (eepSize > 0x800) {             // MODE 3 [24C65]
      EepromSetDeviceAddress(0);       // [A2-A0] = 000
      EepromWriteMode();               // WRITE
      EepromSetAddress(addrhi);        // ADDR [A15-A8]
    } else {                           // MODE 2 [24C02/24C08/24C16]
      EepromSetDeviceAddress(addrhi);  // ADDR [A10-A8]
      EepromWriteMode();               // WRITE
    }
    EepromSetAddress(addrlo);
  } else {  // 24C01
    EepromSetAddress(addrlo);
    EepromWriteMode();  // WRITE
  }
  EepromWriteData(eeptemp);
  EepromStop();  // STOP
  if (eepType == 2)
    EepromInit(1);  // Disable EEPROM
}

// Read EEPROM and save to the SD card
void readEEP_MD() {
  dataIn_MD();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".eep");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sd.chdir();
  sprintf(folder, "MD/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  println_Msg(F("Reading..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }
  if (eepSize > 0x100) {  // 24C04+
    for (word currByte = 0; currByte < eepSize; currByte += 256) {
      print_Msg(F("*"));
      display_Update();
      for (int i = 0; i < 256; i++) {
        readEepromByte(currByte + i);
      }
      myFile.write(sdBuffer, 256);
    }
  } else {  // 24C01/24C02
    for (word currByte = 0; currByte < eepSize; currByte++) {
      if ((currByte != 0) && ((currByte + 1) % 16 == 0)) {
        print_Msg(F("*"));
        display_Update();
      }
      readEepromByte(currByte);
    }
    myFile.write(sdBuffer, eepSize);
  }
  // Close the file:
  myFile.close();
  println_Msg(F(""));
  display_Clear();
  print_Msg(F("Saved to "));
  print_Msg(folder);

  display_Update();
}

void writeEEP_MD() {
  dataOut_MD();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    if (eepSize > 0x100) {  // 24C04+
      for (word currByte = 0; currByte < eepSize; currByte += 256) {
        myFile.read(sdBuffer, 256);
        for (int i = 0; i < 256; i++) {
          writeEepromByte(currByte + i);
          delay(50);  // DELAY NEEDED
        }
        print_Msg(F("."));
        display_Update();
      }
    } else {  // 24C01/24C02
      myFile.read(sdBuffer, eepSize);
      for (word currByte = 0; currByte < eepSize; currByte++) {
        writeEepromByte(currByte);
        print_Msg(F("."));
        if ((currByte != 0) && ((currByte + 1) % 64 == 0))
          println_Msg(F(""));
        display_Update();  // ON SERIAL = delay(100)
      }
    }
    // Close the file:
    myFile.close();
    println_Msg(F(""));
    display_Clear();
    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }
  dataIn_MD();
}

//******************************************
// CD Backup RAM Functions
//******************************************
void readBram_MD() {
  dataIn_MD();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, "Cart.brm");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sd.chdir();
  sprintf(folder, "MD/RAM/%d", foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  println_Msg(F("Reading..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  for (unsigned long currByte = 0; currByte < bramSize; currByte += 512) {
    for (int i = 0; i < 512; i++) {
      sdBuffer[i] = readWord_MD(0x300000 + currByte + i);
    }
    myFile.write(sdBuffer, 512);
  }

  // Close the file:
  myFile.close();
  println_Msg(F(""));
  display_Clear();
  print_Msg(F("Saved to "));
  print_Msg(folder);

  display_Update();
}

void writeBram_MD() {
  dataOut_MD();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // 0x700000-0x7FFFFF: Writes by /LWR latch D0; 1=RAM write enabled, 0=disabled
    writeWord_MD(0x380000, 1);  // Enable BRAM Writes

    for (unsigned long currByte = 0; currByte < bramSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      for (int i = 0; i < 512; i++) {
        writeWord_MD(0x300000 + currByte + i, sdBuffer[i]);
      }
    }
    writeWord_MD(0x380000, 0);  // Disable BRAM Writes
    // Close the file:
    myFile.close();
    println_Msg(F(""));
    display_Clear();
    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }
  dataIn_MD();
}

//******************************************
// Realtec Mapper Functions
//******************************************
void writeRealtec(unsigned long address, byte value) {  // Realtec 0x404000 (UPPER)/0x400000 (LOWER)
  dataOut_MD();
  PORTF = address & 0xFF;          // 0x00 ADDR A0-A7
  PORTK = (address >> 8) & 0xFF;   // ADDR A8-A15
  PORTL = (address >> 16) & 0xFF;  //0x20 ADDR A16-A23
  PORTA = 0x00;                    // DATA D8-D15
  PORTH |= (1 << 0);               // /RES HIGH

  PORTH |= (1 << 3);  // CE HIGH
  PORTC = value;
  PORTH &= ~(1 << 4) & ~(1 << 5);  // /UDSW + /LDSW LOW
  PORTH |= (1 << 4) | (1 << 5);    // /UDSW + /LDSW HIGH
  dataIn_MD();
}

void readRealtec_MD() {
  // Set control
  dataIn_MD();

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".MD");

  // create a new folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "MD/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_STR(saving_to_STR, 0);
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  // Realtec Registers
  writeWord_MD(0x201000, 4);  // Number of 128K Blocks 0x402000 (0x201000)
  writeRealtec(0x200000, 1);  // ROM Lower Address 0x400000 (0x200000)
  writeRealtec(0x202000, 0);  // ROM Upper Address 0x404000 (0x202000)

  word d = 0;
  for (unsigned long currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 256) {
    // Blink led
    if (currBuffer % 16384 == 0)
      blinkLED();

    for (int currWord = 0; currWord < 256; currWord++) {
      word myWord = readWord_MD(currBuffer + currWord);
      // Split word into two bytes
      // Left
      sdBuffer[d] = ((myWord >> 8) & 0xFF);
      // Right
      sdBuffer[d + 1] = (myWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
  }
  // Close the file:
  myFile.close();
}

#endif

//******************************************
// End of File
//******************************************
