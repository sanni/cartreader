//******************************************
// SF MEMORY MODULE
//******************************************
#if (defined(ENABLE_SFM) && defined(ENABLE_SNES))

/******************************************
   SF Memory Clock Source
******************************************/
// The clock signal for the SF Memory cassette
// is generated with the Adafruit Clock Generator
// or a similar external clock source

/******************************************
   Variables
 *****************************************/
// SF Memory status
byte sfmReady = 0;

// Arrays that hold game info
// int gameSize[8];
// int saveSize[8];
byte gameAddress[8];
// byte gameVersion[8];
// boolean hirom[8];

/******************************************
  Menu
*****************************************/
// SFM menu items
static const char sfmMenuItem1[] PROGMEM = "Game Menu";
static const char sfmMenuItem2[] PROGMEM = "Flash Menu";
static const char* const menuOptionsSFM[] PROGMEM = { sfmMenuItem1, sfmMenuItem2, FSTRING_RESET };

// SFM flash menu items
static const char sfmFlashMenuItem1[] PROGMEM = "Read Flash";
static const char sfmFlashMenuItem2[] PROGMEM = "Write Flash";
static const char sfmFlashMenuItem3[] PROGMEM = "Print Mapping";
static const char sfmFlashMenuItem4[] PROGMEM = "Read Mapping";
static const char sfmFlashMenuItem5[] PROGMEM = "Write Mapping";
static const char sfmFlashMenuItem6[] PROGMEM = "Back";
static const char* const menuOptionsSFMFlash[] PROGMEM = { sfmFlashMenuItem1, sfmFlashMenuItem2, sfmFlashMenuItem3, sfmFlashMenuItem4, sfmFlashMenuItem5, sfmFlashMenuItem6 };

// SFM game menu items
static const char sfmGameMenuItem2[] PROGMEM = "Read Game";
static const char sfmGameMenuItem4[] PROGMEM = "Switch Game";
static const char* const menuOptionsSFMGame[] PROGMEM = { FSTRING_READ_SAVE, sfmGameMenuItem2, FSTRING_WRITE_SAVE, sfmGameMenuItem4, FSTRING_RESET };

void sfmMenu() {
  // create menu with title and 3 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSFM, 3);
  mainMenu = question_box(F("SF Memory"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    // Game menu
    case 0:
      sfmGameMenu();
      break;
#ifdef ENABLE_FLASH
    // Flash menu
    case 1:
      mode = CORE_SFM_FLASH;
      break;
#endif
    // Reset
    case 2:
      resetArduino();
      break;
    default:
      print_MissingModule();
      break;
  }
}

void sfmGameMenu() {
  char menuOptionsSFMGames[8][20];
  byte numGames;
  boolean hasMenu;
  // Switch to hirom all
  if (send_SFM(0x04) == 0x2A) {
    delay(300);

    // Fill arrays with data
    getGames(menuOptionsSFMGames, &hasMenu, &numGames);

    if (hasMenu) {
      // Create menu with title and numGames options to choose from
      unsigned char gameSubMenu;
      // wait for user choice to come back from the question box menu
      gameSubMenu = question_box(F("Select Game"), menuOptionsSFMGames, numGames, 0);

      // Switch to game
      send_SFM(gameSubMenu + 0x80);
      delay(200);
      // Check for successfull switch
      byte timeout = 0;
      while (readBank_SFM(0, 0x2400) != 0x7D) {
        delay(200);
        // Try again
        send_SFM(gameSubMenu + 0x80);
        delay(200);
        timeout++;
        // Abort, something is wrong
        if (timeout == 5) {
          display_Clear();
          print_Msg(F("Game "));
          print_Msg(gameSubMenu + 0x80, HEX);
          println_Msg(F(" Timeout"));
          println_Msg(readBank_SFM(0, 0x2400), HEX);
          println_Msg(FS(FSTRING_EMPTY));
          print_FatalError(F("Powercycle SFM cart"));
        }
      }
      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, menuOptionsSFMGames[gameSubMenu + 1]);

      // Print info
      getCartInfo_SFM();
      mode = CORE_SFM_GAME;
    } else {
      // No menu so switch to only game
      // Switch to game
      send_SFM(0x80);
      delay(200);

      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, menuOptionsSFMGames[0]);

      // Print info
      getCartInfo_SFM();
      mode = CORE_SFM_GAME;
    }
  } else {
    print_Error(F("Switch to HiRom failed"));
  }
}

void sfmGameOptions() {
  // create menu with title and 5 options to choose from
  unsigned char gameSubMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSFMGame, 5);
  gameSubMenu = question_box(F("SFM Game Menu"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (gameSubMenu) {
    // Read sram
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readSRAM();
      break;

    // Read rom
    case 1:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_SFM();
      compare_checksum();
      compareCRC("snes.txt", 0, 1, 0);
      break;

    // Write sram
    case 2:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      writeSRAM(1);
      unsigned long wrErrors;
      wrErrors = verifySRAM();
      if (wrErrors == 0) {
        println_Msg(F("Verified OK"));
        display_Update();
      } else {
        print_STR(error_STR, 0);
        print_Msg(wrErrors);
        print_STR(_bytes_STR, 1);
        print_Error(did_not_verify_STR);
      }
      break;

    // Switch game
    case 3:
      sfmGameMenu();
      break;

    // Reset
    case 4:
      resetArduino();
      break;
  }
  if (gameSubMenu != 3) {
    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
}

#ifdef ENABLE_FLASH
void sfmFlashMenu() {
  // create menu with title and 6 options to choose from
  unsigned char flashSubMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsSFMFlash, 6);
  flashSubMenu = question_box(F("SFM Flash Menu"), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (flashSubMenu) {
    // Read Flash
    case 0:
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Reset to HIROM ALL
      romType = 1;
      print_Msg(F("Switch to HiRom..."));
      display_Update();
      if (send_SFM(0x04) == 0x2A) {
        println_Msg(FS(FSTRING_OK));
        display_Update();

        // Reset flash
        resetFlash_SFM(0xC0);
        resetFlash_SFM(0xE0);

        flashSize = 4194304;
        numBanks = 64;

        // Get name, add extension and convert to char array for sd lib
        EEPROM_readAnything(0, foldern);
        sprintf(fileName, "SFM%d", foldern);
        strcat(fileName, ".bin");
        sd.mkdir("NP", true);
        sd.chdir("NP");
        // write new folder number back to eeprom
        foldern = foldern + 1;
        EEPROM_writeAnything(0, foldern);

        // Read flash
        readFlash_SFM();
      } else {
        print_Error(F("Switch to HiRom failed"));
      }
      break;

    // Write Flash
    case 1:
      // Clear screen
      display_Clear();

      // Print warning
      println_Msg(F("Attention"));
      println_Msg(F("This will erase your"));
      println_Msg(F("NP Cartridge."));
      println_Msg("");
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();

      // Clear screen
      display_Clear();

      filePath[0] = '\0';
      sd.chdir("/");
      // Launch file browser
      fileBrowser(F("Select 4MB file"));
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);
      flashSize = 2097152;
      numBanks = 32;
      println_Msg(F("Writing 1st rom"));
      display_Update();
      // Program 1st flashrom
      write_SFM(0xC0, 0);
      display_Clear();
      println_Msg(F("Writing 2nd rom"));
      display_Update();
      // Program 2nd flashrom
      write_SFM(0xE0, 2097152);
      break;

    // Print mapping
    case 2:
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Reset to HIROM ALL
      romType = 1;
      print_Msg(F("Switch to HiRom..."));
      display_Update();
      if (send_SFM(0x04) == 0x2A) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
        idFlash_SFM(0xC0);
        if (flashid == 0xc2f3) {
          idFlash_SFM(0xE0);
          if (flashid == 0xc2f3) {
            // Reset flash
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
            delay(100);
            // Clear screen
            display_Clear();
            printMapping();
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
          } else {
            print_FatalError(F("Error: Wrong Flash ID"));
          }
        } else {
          print_FatalError(F("Error: Wrong Flash ID"));
        }
      } else {
        print_Error(F("failed"));
      }
      break;

    // Read mapping
    case 3:
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Reset to HIROM ALL
      romType = 1;
      print_Msg(F("Switch to HiRom..."));
      display_Update();
      if (send_SFM(0x04) == 0x2A) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
        idFlash_SFM(0xC0);
        if (flashid == 0xc2f3) {
          idFlash_SFM(0xE0);
          if (flashid == 0xc2f3) {
            // Reset flash
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
            delay(100);
            readMapping();
            resetFlash_SFM(0xC0);
            resetFlash_SFM(0xE0);
          } else {
            print_FatalError(F("Error: Wrong Flash ID"));
          }
        } else {
          print_FatalError(F("Error: Wrong Flash ID"));
        }
      } else {
        print_Error(F("failed"));
      }
      break;

    // Write mapping
    case 4:
      // Clear screen
      display_Clear();

      // Print warning
      println_Msg(F("Attention"));
      println_Msg(F("This will erase your"));
      println_Msg(F("NP Cartridge."));
      println_Msg("");
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();

      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Erase mapping
      eraseMapping(0xD0);
      eraseMapping(0xE0);
      print_Msg(F("Blankcheck..."));
      display_Update();
      if (blankcheckMapping_SFM()) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
      } else {
        println_Msg(F("Nope"));
        break;
      }

      // Clear screen
      display_Clear();

      // Clear filepath
      filePath[0] = '\0';

      // Reset to root directory
      sd.chdir("/");

      // Launch file browser
      fileBrowser(F("Select MAP file"));
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);
      display_Update();

      // Write mapping
      writeMapping_SFM(0xD0, 0);
      writeMapping_SFM(0xE0, 256);
      break;

    // Go back
    case 5:
      mode = CORE_SFM;
      break;
  }
  if (flashSubMenu != 5) {
    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
}
#endif

// Read the games from the menu area
void getGames(char gameCode[8][20], boolean* hasMenu, byte* numGames) {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  // Check if menu is present
  *hasMenu = true;
  byte menuString[] = { 0x4D, 0x45, 0x4E, 0x55, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D };
  for (int i = 0; i < 12; i++) {
    if (menuString[i] != readBank_SFM(0xC0, 0x7FC0 + i)) {
      *hasMenu = false;
    }
  }

  if (*hasMenu) {
    *numGames = 0;
    // Count number of games
    for (word i = 0x0000; i < 0xE000; i += 0x2000) {
      if (readBank_SFM(0xC6, i) == *numGames)
        (*numGames)++;
    }

    // Get game info
    for (int i = 0; i < *numGames; i++) {
      // Read starting address and size
      gameAddress[i] = 0xC0 + readBank_SFM(0xC6, i * 0x2000 + 0x01) * 0x8;
      // gameSize[i] = readBank_SFM(0xC6, i * 0x2000 + 0x03) * 128;
      // saveSize[i] = readBank_SFM(0xC6, i * 0x2000 + 0x05) / 8;

      //check if hirom
      // if (readBank_SFM(gameAddress[i], 0xFFD5) == 0x31) {
      //   hirom[i] = true;
      // } else if (readBank_SFM(gameAddress[i], 0xFFD5) == 0x21) {
      //   hirom[i] = true;
      // } else {
      //   hirom[i] = false;
      // }

      // if (hirom[i]) {
      //   gameVersion[i] = readBank_SFM(gameAddress[i], 0xFFDB);
      // } else {
      //   gameVersion[i] = readBank_SFM(gameAddress[i], 0x7FDB);
      // }

      // Read game code
      byte myByte = 0;
      byte myLength = 0;
      for (int j = 0; j < 9; j++) {
        myByte = readBank_SFM(0xC6, i * 0x2000 + 0x07 + j);
        // Remove funny characters
        if (((myByte >= ',' && myByte <= '9') || (myByte >= 'A' && myByte <= 'z')) && myLength < 9) {
          gameCode[i][myLength] = myByte;
          myLength++;
        }
      }
      // End char array in case game code is less than 9 chars
      gameCode[i][myLength] = '\0';
    }
  } else {
    word base;
    *numGames = 1;
    //check if hirom
    if (readBank_SFM(0xC0, 0xFFD5) == 0x31) {
      // hirom[0] = true;
      base = 0xFF00;
    } else {
      // hirom[0] = false;
      base = 0x7F00;
    }

    // gameVersion[0] = readBank_SFM(0xC0, base + 0xDB);
    gameCode[0][0] = 'G';
    gameCode[0][1] = 'A';
    gameCode[0][2] = 'M';
    gameCode[0][3] = 'E';
    gameCode[0][4] = '-';
    gameCode[0][5] = readBank_SFM(0xC0, base + 0xB2);
    gameCode[0][6] = readBank_SFM(0xC0, base + 0xB3);
    gameCode[0][7] = readBank_SFM(0xC0, base + 0xB4);
    gameCode[0][8] = readBank_SFM(0xC0, base + 0xB5);
    gameCode[0][9] = '\0';
    // gameSize[0] = 1 << (readBank_SFM(0xC0, base + 0xD7) - 7);
  }
}

/******************************************
   Setup
 *****************************************/
void setup_SFM() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal to disable snesCIC
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Switch RST(PH0) and WR(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  //PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Unused pins
  // Set CPU Clock(PH1) to Output
  DDRH |= (1 << 1);
  //PORTH &= ~(1 << 1);

  // Adafruit Clock Generator
  initializeClockOffset();

  if (i2c_found) {
    clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
    clockgen.set_freq(2147727200ULL, SI5351_CLK0);

    // start outputting master clock
    clockgen.output_enable(SI5351_CLK1, 0);
    clockgen.output_enable(SI5351_CLK2, 0);
    clockgen.output_enable(SI5351_CLK0, 1);
  }
#ifdef ENABLE_CLOCKGEN
  else {
    display_Clear();
    print_FatalError(F("Clock Generator not found"));
  }
#endif

  // Wait until all is stable
  delay(500);

  // Switch to HiRom All
  byte timeout = 0;
  send_SFM(0x04);
  delay(200);
  while (readBank_SFM(0, 0x2400) != 0x2A) {
    delay(100);
    // Try again
    send_SFM(0x04);
    delay(100);
    timeout++;
    // Abort, something is wrong
    if (timeout == 5) {
      println_Msg(F("Hirom All Timeout"));
      println_Msg(FS(FSTRING_EMPTY));
      println_Msg(FS(FSTRING_EMPTY));
      print_FatalError(F("Powercycle SFM cart"));
    }
  }
}

/******************************************
   I/O Functions
 *****************************************/
// Switch control pins to write
void controlOut_SFM() {
  // Switch RD(PH6) and WR(PH5) to HIGH
  PORTH |= (1 << 6) | (1 << 5);
  // Switch CS(PH3) to LOW
  PORTH &= ~(1 << 3);
}

// Switch control pins to read
void controlIn_SFM() {
  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);
  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
}

/******************************************
   Low level functions
 *****************************************/
// Write one byte of data to a location specified by bank and address, 00:0000
void writeBank_SFM(byte myBank, word myAddress, byte myData) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 60ns
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
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
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
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

// Read one byte of data from a location specified by bank and address, 00:0000
byte readBank_SFM(byte myBank, word myAddress) {
  PORTL = myBank;
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns -> 1000ns total
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
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINC;
  return tempByte;
}

/******************************************
  SNES ROM Functions
******************************************/
void getCartInfo_SFM() {
  // Print start page
  if (checkcart_SFM() == 0) {
    // Checksum either corrupt or 0000
    errorLvl = 1;
    rgbLed(red_color);
    display_Clear();
    println_Msg(F("ERROR"));
    println_Msg(F("Rom header corrupt"));
    println_Msg(F("or missing"));
    display_Update();
    wait();
    // Wait() clears errors but in this case we still have an error
    errorLvl = 1;
  }

  display_Clear();
  print_Msg(FS(FSTRING_NAME));
  println_Msg(romName);
  println_Msg(FS(FSTRING_SPACE));

  print_Msg(F("Version: 1."));
  println_Msg(romVersion);

  print_Msg(FS(FSTRING_CHECKSUM));
  println_Msg(checksumStr);

  print_Msg(FS(FSTRING_SIZE));
  print_Msg(romSize);
  println_Msg(F("Mbit "));

  print_Msg(F("Type: "));
  if (romType == 1)
    println_Msg(F("HiROM"));
  else if (romType == 0)
    println_Msg(F("LoROM"));
  else
    println_Msg(romType);

  print_Msg(F("Banks: "));
  println_Msg(numBanks);

  print_Msg(F("Sram: "));
  print_Msg(sramSize);
  println_Msg(F("Kbit"));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  // Wait for user input
  wait();
}

// Read header information
boolean checkcart_SFM() {
  // set control to read
  dataIn();

  // Read ROM header
  byte snesHeader[80] = { 0 };
  for (byte c = 0; c < 80; c++) {
    snesHeader[c] = readBank_SFM(0, 0xFFB0 + c);
  }

  // Calculate CRC32 of header
  char crcStr[9];
  sprintf(crcStr, "%08lX", calculateCRC(snesHeader, 80));

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readBank_SFM(0, 65503), readBank_SFM(0, 65502));

  romType = readBank_SFM(0, 0xFFD5);
  if ((romType >> 5) != 1) {  // Detect invalid romType byte due to too long ROM name (22 chars)
    romType = 0;              // LoROM   // Krusty's Super Fun House (U) 1.0 & Contra 3 (U)
  } else {
    romType &= 1;  // Must be LoROM or HiROM
  }

  // Check RomSize
  byte romSizeExp = readBank_SFM(0, 65495) - 7;
  romSize = 1;
  while (romSizeExp--)
    romSize *= 2;

  numBanks = (long(romSize) * 1024 * 1024 / 8) / (32768 + (long(romType) * 32768));

  //Check SD card for alt config, pass CRC32 of snesHeader but filter out 0000 and FFFF checksums
  if (!(strcmp(checksumStr, "0000") == 0) && !(strcmp(checksumStr, "FFFF") == 0)) {
    checkAltConf(crcStr);
  }

  // Get name
  byte myByte = 0;
  byte myLength = 0;
  for (unsigned int i = 65472; i < 65492; i++) {
    myByte = readBank_SFM(0, i);
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }
  // If name consists out of all japanese characters use game code
  if (myLength == 0) {
    // Get rom code
    romName[0] = 'S';
    romName[1] = 'H';
    romName[2] = 'V';
    romName[3] = 'C';
    romName[4] = '-';
    for (unsigned int i = 0; i < 4; i++) {
      myByte = readBank_SFM(0, 0xFFB2 + i);
      if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 4) {
        romName[myLength + 5] = char(myByte);
        myLength++;
      }
    }
    if (myLength == 0) {
      // Rom code unknown
      romName[0] = 'U';
      romName[1] = 'N';
      romName[2] = 'K';
      romName[3] = 'N';
      romName[4] = 'O';
      romName[5] = 'W';
      romName[6] = 'N';
    }
  }

  // Read sramSizeExp
  byte sramSizeExp = readBank_SFM(0, 0xFFD8);

  // Calculate sramSize
  if (sramSizeExp != 0) {
    sramSizeExp = sramSizeExp + 3;
    sramSize = 1;
    while (sramSizeExp--)
      sramSize *= 2;
  } else {
    sramSize = 0;
  }

  // ROM Version
  romVersion = readBank_SFM(0, 65499);

  // Test if checksum is equal to reverse checksum
  if (((word(readBank_SFM(0, 65500)) + (word(readBank_SFM(0, 65501)) * 256)) + (word(readBank_SFM(0, 65502)) + (word(readBank_SFM(0, 65503)) * 256))) == 65535) {
    if (strcmp("0000", checksumStr) == 0) {
      return 0;
    } else {
      return 1;
    }
  }
  // Either rom checksum is wrong or no cart is inserted
  else {
    return 0;
  }
}

// Read rom to SD card
void readROM_SFM() {
  // Set control
  dataIn();
  controlIn_SFM();

  // Get name, add extension and convert to char array for sd lib
  createFolder("NP", "ROM", romName, "sfc");

  //clear the screen
  display_Clear();
  print_Msg(F("Creating folder "));
  println_Msg(folder);
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }

  // Check if LoROM or HiROM...
  if (romType == 0) {
    println_Msg(F("Dumping LoRom..."));
    display_Update();

    // Read up to 96 banks starting at bank 0×00.
    for (word currBank = 0; currBank < numBanks; currBank++) {
      // Dump the bytes to SD 512B at a time
      for (long currByte = 32768; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }
  // Dump High-type ROM
  else {
    println_Msg(F("Dumping HiRom..."));
    display_Update();

    for (word currBank = 192; currBank < (numBanks + 192); currBank++) {
      for (long currByte = 0; currByte < 65536; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  print_Msg(F("Saved as "));
  println_Msg(fileName);
}

/******************************************
   29F1601 flashrom functions (NP)
 *****************************************/
// Reset the MX29F1601 flashrom, startbank is 0xC0 for first and 0xE0 for second flashrom
void resetFlash_SFM(int startBank) {
  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  // Reset command sequence
  if (romType) {
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0xf0);
  } else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xf0);
  }
}

// Print flashrom manufacturer and device ID
void idFlash_SFM(int startBank) {
  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  if (romType) {
    // ID command sequence
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SFM();

    // Read the two id bytes into a string
    flashid = readBank_SFM(startBank, 0x00) << 8;
    flashid |= readBank_SFM(startBank, 0x02);
    sprintf(flashid_str, "%04x", flashid);
  } else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SFM();

    // Read the two id bytes into a string
    flashid = readBank_SFM(0, 0x8000) << 8;
    flashid |= readBank_SFM(0, 0x8000 + 0x02);
    sprintf(flashid_str, "%04x", flashid);
  }
}

// Write the flashroms by reading a file from the SD card, pos defines where in the file the reading/writing should start
void writeFlash_SFM(int startBank, uint32_t pos) {
  display_Clear();
  print_Msg(F("Writing Bank 0x"));
  println_Msg(startBank, HEX);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    // Configure control pins
    controlOut_SFM();
    // Set data pins to output
    dataOut();

    if (romType) {
      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = numBanks * 0x10000;
      draw_progressbar(0, totalProgressBar);

      // Write hirom
      for (word currBank = startBank; currBank < startBank + numBanks; currBank++) {
        // Fill SDBuffer with 1 page at a time then write it repeat until all bytes are written
        for (unsigned long currByte = 0; currByte < 0x10000; currByte += 128) {
          myFile.read(sdBuffer, 128);
          // Write command sequence
          writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
          writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SFM(startBank, 0x5555L * 2, 0xa0);

          for (byte c = 0; c < 128; c++) {

            // Write one byte of data
            writeBank_SFM(currBank, currByte + c, sdBuffer[c]);

            if (c == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank_SFM(currBank, currByte + c, sdBuffer[c]);
            }
          }
          // Wait until write is finished
          busyCheck_SFM(startBank);
        }
        // update progress bar
        processedProgressBar += 0x10000;
        draw_progressbar(processedProgressBar, totalProgressBar);
      }
    } else {
      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = numBanks * 0x8000;
      draw_progressbar(0, totalProgressBar);

      // Write lorom
      for (word currBank = 0; currBank < numBanks; currBank++) {
        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 128) {
          myFile.read(sdBuffer, 128);
          // Write command sequence
          writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
          writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
          writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xa0);

          for (byte c = 0; c < 128; c++) {
            // Write one byte of data
            writeBank_SFM(currBank, currByte + c, sdBuffer[c]);

            if (c == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank_SFM(currBank, currByte + c, sdBuffer[c]);
            }
          }
          // Wait until write is finished
          busyCheck_SFM(startBank);
        }
        // update progress bar
        processedProgressBar += 0x8000;
        draw_progressbar(processedProgressBar, totalProgressBar);
      }
    }
    // Close the file:
    myFile.close();
    println_Msg("");
  } else {
    print_FatalError(open_file_STR);
  }
}

// Delay between write operations based on status register
void busyCheck_SFM(byte startBank) {
  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn_SFM();

  // Read register
  readBank_SFM(startBank, 0x0000);

  // Read D7 while D7 = 0
  //1 = B00000001, 1 << 7 = B10000000, PINC = B1XXXXXXX (X = don't care), & = bitwise and
  while (!(PINC & (1 << 7))) {
    // CE or OE must be toggled with each subsequent status read or the
    // completion of a program or erase operation will not be evident.
    // Switch RD(PH6) to HIGH
    PORTH |= (1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");

    // Switch RD(PH6) to LOW
    PORTH &= ~(1 << 6);

    // one nop ~62.5ns
    __asm__("nop\n\t");
  }

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();
}

// Erase the flashrom to 0xFF
void eraseFlash_SFM(int startBank) {

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  if (romType) {
    // Erase command sequence
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x80);
    writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
    writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SFM(startBank, 0x5555L * 2, 0x10);
  } else {
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x80);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SFM(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SFM(1, 0x8000 + 0x1555L * 2, 0x10);
  }

  // Wait for erase to complete
  busyCheck_SFM(startBank);
}

// Check if an erase succeeded, return 1 if blank and 0 if not
byte blankcheck_SFM(int startBank) {

  // Set data pins to input again
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  byte blank = 1;
  if (romType) {
    for (word currBank = startBank; currBank < startBank + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte++) {
        if (readBank_SFM(currBank, currByte) != 0xFF) {
          currBank = startBank + numBanks;
          blank = 0;
        }
      }
    }
  } else {
    for (word currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte++) {
        if (readBank_SFM(currBank, currByte) != 0xFF) {
          currBank = numBanks;
          blank = 0;
        }
      }
    }
  }
  return blank;
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyFlash_SFM(int startBank, uint32_t pos) {
  unsigned long verified = 0;

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Set file starting position
    myFile.seekCur(pos);

    // Set data pins to input
    dataIn();
    // Set control pins to input
    controlIn_SFM();

    if (romType) {
      for (word currBank = startBank; currBank < startBank + numBanks; currBank++) {
        for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank_SFM(currBank, currByte + c) != sdBuffer[c]) {
              verified++;
            }
          }
        }
      }
    } else {
      for (word currBank = 0; currBank < numBanks; currBank++) {
        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank_SFM(currBank, currByte + c) != sdBuffer[c]) {
              verified++;
            }
          }
        }
      }
    }
    // Close the file:
    myFile.close();
  } else {
    // SD Error
    verified = 999999;
    print_Error(F("Can't open file on SD"));
  }
  // Return 0 if verified ok, or number of errors
  return verified;
}

// Read flashroms and save them to the SD card
void readFlash_SFM() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SFM();

  print_Msg(F("Saving as NP/"));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }
  if (romType) {
    for (word currBank = 0xC0; currBank < 0xC0 + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  } else {
    for (word currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SFM(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }
  // Close the file:
  myFile.close();
  println_Msg("");
  println_Msg(F("Finished reading"));
  display_Update();
}

// Display protected sectors/banks as 0xc2 and unprotected as 0x00
void readSectorProtection_SFM(byte startBank) {

  // Configure control pins
  controlOut_SFM();
  // Set data pins to output
  dataOut();

  // Display Sector Protection Status
  writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
  writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
  writeBank_SFM(startBank, 0x5555L * 2, 0x90);

  // Configure control pins
  controlIn_SFM();
  // Set data pins to output
  dataIn();
  display_Clear();
  for (int i = 0; i <= 0x1F; i++) {
    print_Msg(F("Sector: 0x"));
    print_Msg(startBank + i, HEX);
    print_Msg(F(" Sector Protect: 0x"));
    println_Msg(readBank_SFM(startBank + i, 0x04), HEX);
  }
  display_Update();
}

// Read the current mapping from the hidden "page buffer" and print it
void printMapping() {
  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping out of the first chip
  char buffer[3];

  for (unsigned int currByte = 0xFF00; currByte < 0xFF50; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa(readBank_SFM(0xC0, currByte + c), buffer, 16);
      for (size_t i = 0; i < 2 - strlen(buffer); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(buffer);
    }
    println_Msg("");
  }
  display_Update();

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();
}

// Read the current mapping from the hidden "page buffer"
void readMapping() {

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sprintf(fileName, "NP%d", foldern);
  strcat(fileName, ".MAP");
  sd.mkdir("NP", true);
  sd.chdir("NP");

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank_SFM(0xC0, currByte));
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xE0, 0x0000, 0x38);
  writeBank_SFM(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank_SFM(0xE0, currByte));
  }

  // Close the file:
  myFile.close();

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Signal end of process
  print_Msg(F("Saved to NP/"));
  println_Msg(fileName);
  display_Update();
}

void eraseMapping(byte startBank) {

  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (flashid == 0xc2f3) {
      resetFlash_SFM(startBank);

      // Switch to write
      dataOut();
      controlOut_SFM();

      // Prepare to erase/write Page Buffer
      writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
      writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SFM(startBank, 0x5555L * 2, 0x77);
      // Erase Page Buffer
      writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
      writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SFM(startBank, 0x5555L * 2, 0xe0);

      // Wait until complete
      busyCheck_SFM(startBank);

      // Switch to read
      dataIn();
      controlIn_SFM();
    } else {
      print_FatalError(F("Error: Wrong Flash ID"));
    }
  } else {
    print_FatalError(F("Unlock failed"));
  }
}

// Check if the current mapping is all 0xFF
byte blankcheckMapping_SFM() {
  byte blank = 1;

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xC0, 0x0000, 0x38);
  writeBank_SFM(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SFM(0xC0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset to defaults
  writeBank_SFM(0xE0, 0x0000, 0x38);
  writeBank_SFM(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SFM(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SFM(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SFM(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SFM(0xE0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SFM();

  // Reset Flash
  writeBank_SFM(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SFM(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SFM(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SFM(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SFM();

  return blank;
}

void writeMapping_SFM(byte startBank, uint32_t pos) {

  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (flashid == 0xc2f3) {
      resetFlash_SFM(startBank);

      // Switch to write
      dataOut();
      controlOut_SFM();

      // Open file on sd card
      if (myFile.open(filePath, O_READ)) {

        // Seek to a new position in the file
        if (pos != 0)
          myFile.seekCur(pos);

        // Write to Page Buffer
        for (unsigned long currByte = 0xFF00; currByte < 0xFFFF; currByte += 128) {
          // Prepare to erase/write Page Buffer
          writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
          writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SFM(startBank, 0x5555L * 2, 0x77);
          // Write Page Buffer Command
          writeBank_SFM(startBank, 0x5555L * 2, 0xaa);
          writeBank_SFM(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SFM(startBank, 0x5555L * 2, 0x99);

          myFile.read(sdBuffer, 128);

          for (byte c = 0; c < 128; c++) {
            writeBank_SFM(startBank, currByte + c, sdBuffer[c]);
            // Write last byte twice
            if (c == 127) {
              writeBank_SFM(startBank, currByte + c, sdBuffer[c]);
            }
          }
          busyCheck_SFM(startBank);
        }

        // Close the file:
        myFile.close();
        println_Msg("");
      } else {
        print_Error(F("Can't open file on SD"));
      }

      // Switch to read
      dataIn();
      controlIn_SFM();
    } else {
      print_FatalError(F("Error: Wrong Flash ID"));
    }
  } else {
    print_FatalError(F("Unlock failed"));
  }
}

/******************************************
  SF Memory functions
*****************************************/
// Switch to HiRom All and unlock Write Protection
boolean unlockHirom() {
  romType = 1;
  print_Msg(F("Switch to HiRom..."));
  display_Update();
  if (send_SFM(0x04) == 0x2A) {
    println_Msg(FS(FSTRING_OK));
    display_Update();
    // Unlock Write Protection
    print_Msg(F("Enable Write..."));
    display_Update();
    send_SFM(0x02);
    if (readBank_SFM(0, 0x2401) == 0x4) {
      println_Msg(FS(FSTRING_OK));
      display_Update();
      return 1;
    } else {
      println_Msg(F("failed"));
      display_Update();
      return 0;
    }
  } else {
    println_Msg(F("failed"));
    display_Update();
    return 0;
  }
}

// Send a command to the MX15001 chip
byte send_SFM(byte command) {
  // Switch to write
  dataOut();
  controlOut_SFM();

  // Write command
  writeBank_SFM(0, 0x2400, 0x09);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read status
  sfmReady = readBank_SFM(0, 0x2400);

  // Switch to write
  dataOut();
  controlOut_SFM();

  writeBank_SFM(0, 0x2401, 0x28);
  writeBank_SFM(0, 0x2401, 0x84);

  // NP_CMD_06h, send this only if above read has returned 7Dh, not if it's already returning 2Ah
  if (sfmReady == 0x7D) {
    writeBank_SFM(0, 0x2400, 0x06);
    writeBank_SFM(0, 0x2400, 0x39);
  }

  // Write the command
  writeBank_SFM(0, 0x2400, command);

  // Switch to read
  dataIn();
  controlIn_SFM();

  // Read status
  sfmReady = readBank_SFM(0, 0x2400);
  return sfmReady;
}

// This function will erase and program the NP cart from a 4MB file off the SD card
void write_SFM(int startBank, uint32_t pos) {
  // Switch NP cart's mapping
  if (unlockHirom()) {
    // Get ID
    idFlash_SFM(startBank);
    if (flashid == 0xc2f3) {
      print_Msg(F("Flash ID: "));
      println_Msg(flashid_str);
      display_Update();
      resetFlash_SFM(startBank);
      delay(1000);
      // Erase flash
      print_Msg(F("Blankcheck..."));
      display_Update();
      if (blankcheck_SFM(startBank)) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
      } else {
        println_Msg(F("Nope"));
        display_Clear();
        print_Msg(F("Erasing..."));
        display_Update();
        eraseFlash_SFM(startBank);
        resetFlash_SFM(startBank);
        print_STR(done_STR, 1);
        print_Msg(F("Blankcheck..."));
        display_Update();
        if (blankcheck_SFM(startBank)) {
          println_Msg(FS(FSTRING_OK));
          display_Update();
        } else {
          print_FatalError(F("Could not erase flash"));
        }
      }
      // Write flash
      writeFlash_SFM(startBank, pos);

      // Reset flash
      resetFlash_SFM(startBank);

      // Checking for errors
      print_STR(verifying_STR, 0);
      display_Update();
      writeErrors = verifyFlash_SFM(startBank, pos);
      if (writeErrors == 0) {
        println_Msg(FS(FSTRING_OK));
        display_Update();
      } else {
        print_STR(error_STR, 0);
        print_Msg(writeErrors);
        print_STR(_bytes_STR, 1);
        print_FatalError(did_not_verify_STR);
      }
    } else {
      print_FatalError(F("Error: Wrong Flash ID"));
    }
  } else {
    print_FatalError(F("Unlock failed"));
  }
}

#endif

//******************************************
// End of File
//******************************************