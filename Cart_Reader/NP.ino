//******************************************
// NINTENDO POWER
//******************************************

/******************************************
   NP Clock Source
******************************************/
// The clock signal for the Nintendo Power cart
// is generated with the Adafruit Clock Generator
// If you don't have one just plug a wire into CLK0
// (Snes Pin 1) and let the 50/60Hz noise of your
// mains do the rest, works in ~80% of all cases ;)

/******************************************
   Variables
 *****************************************/
// Nintendo Power status
byte NPReady = 0;

// NP Menu
boolean hasMenu = true;
byte numGames = 0;

// Arrays that hold game info
int gameSize[8];
int saveSize[8];
byte gameAddress[8];
byte gameVersion[8];
char gameCode[8][10];
boolean hirom[8];

/******************************************
  Menu
*****************************************/
// NP menu items
static const char NPMenuItem1[] PROGMEM = "Game Menu";
static const char NPMenuItem2[] PROGMEM = "Flash Menu";
static const char NPMenuItem3[] PROGMEM = "Reset";
static const char* const menuOptionsNP[] PROGMEM = {NPMenuItem1, NPMenuItem2, NPMenuItem3};

// NP flash menu items
static const char NPFlashMenuItem1[] PROGMEM = "Read Flash";
static const char NPFlashMenuItem2[] PROGMEM = "Write Flash";
static const char NPFlashMenuItem3[] PROGMEM = "Print Mapping";
static const char NPFlashMenuItem4[] PROGMEM = "Read Mapping";
static const char NPFlashMenuItem5[] PROGMEM = "Write Mapping";
static const char NPFlashMenuItem6[] PROGMEM = "Back";
static const char* const menuOptionsNPFlash[] PROGMEM = {NPFlashMenuItem1, NPFlashMenuItem2, NPFlashMenuItem3, NPFlashMenuItem4, NPFlashMenuItem5, NPFlashMenuItem6};

// NP game menu items
static const char NPGameMenuItem1[] PROGMEM = "Read Sram";
static const char NPGameMenuItem2[] PROGMEM = "Read Game";
static const char NPGameMenuItem3[] PROGMEM = "Write Sram";
static const char NPGameMenuItem4[] PROGMEM = "Switch Game";
static const char NPGameMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsNPGame[] PROGMEM = {NPGameMenuItem1, NPGameMenuItem2, NPGameMenuItem3, NPGameMenuItem4, NPGameMenuItem5};

void npMenu() {
  // create menu with title and 3 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsNP, 3);
  mainMenu = question_box("Nintendo Power", menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    // Game menu
    case 0:
      NPGameMenu();
      break;
    // Flash menu
    case 1:
      mode = mode_NPFlash;
      break;
    // Reset
    case 2:
      asm volatile ("  jmp 0");
      break;
  }
}

void NPGameMenu() {
  // Switch to hirom all
  if (send_NP(0x04) == 0x2A) {
    delay(300);

    // Fill arrays with data
    getGames();

    if (hasMenu) {
      // Create submenu options
      char menuOptionsNPGames[8][20];
      for (int i = 0; i < (numGames); i++) {
        strncpy(menuOptionsNPGames[i], gameCode[i], 10);
      }

      // Create menu with title and numGames options to choose from
      unsigned char gameSubMenu;
      // wait for user choice to come back from the question box menu
      gameSubMenu = question_box("Select Game", menuOptionsNPGames, numGames, 0);

      // Switch to game
      send_NP(gameSubMenu + 0x80);
      delay(200);
      // Check for successfull switch
      byte timeout = 0;
      while (readBank_SNES(0, 0x2400) != 0x7D) {
        delay(200);
        // Try again
        send_NP(gameSubMenu + 0x80);
        delay(200);
        timeout++;
        // Abort, something is wrong
        if (timeout == 5) {
          display_Clear();
          print_Msg(F("Game "));
          print_Msg(gameSubMenu + 0x80, HEX);
          println_Msg(F(" Timeout"));
          println_Msg(readBank_SNES(0, 0x2400), HEX);
          println_Msg(F(""));
          print_Error(F("Please powercycle NP cart"), true);
        }
      }
      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, gameCode[gameSubMenu + 1]);

      // Print info
      getCartInfo_SNES();
      mode = mode_NPGame;
    }
    else {
      // No menu so switch to only game
      // Switch to game
      send_NP(0x80);
      delay(200);

      // Copy gameCode to romName in case of japanese chars in romName
      strcpy(romName, gameCode[0]);

      // Print info
      getCartInfo_SNES();
      mode = mode_NPGame;
    }
  }
  else {
    print_Error(F("Switch to HiRom failed"), false);
  }
}

void NPGameOptions() {
  // create menu with title and 3 options to choose from
  unsigned char gameSubMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsNPGame, 5);
  gameSubMenu = question_box("NP Game Menu", menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (gameSubMenu)
  {
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
      readROM_SNES();
      compare_checksum();
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
      }
      else {
        print_Msg(F("Error: "));
        print_Msg(wrErrors);
        println_Msg(F(" bytes "));
        print_Error(F("did not verify."), false);
      }
      break;

    // Switch game
    case 3:
      NPGameMenu();
      break;

    // Reset
    case 4:
      asm volatile ("  jmp 0");
      break;
  }
  if (gameSubMenu != 3) {
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
}

void NPFlashMenu() {
  // create menu with title and 6 options to choose from
  unsigned char flashSubMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsNPFlash, 6);
  flashSubMenu = question_box("NP Flash Menu", menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (flashSubMenu)
  {
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
      if (send_NP(0x04) == 0x2A) {
        println_Msg(F("OK"));
        display_Update();

        // Reset flash
        resetFlash_NP(0xC0);
        resetFlash_NP(0xE0);

        flashSize = 4194304;
        numBanks = 64;

        // Get name, add extension and convert to char array for sd lib
        EEPROM_readAnything(0, foldern);
        sprintf(fileName, "NP%d", foldern);
        strcat(fileName, ".bin");
        sd.mkdir("NP", true);
        sd.chdir("NP");
        // write new folder number back to eeprom
        foldern = foldern + 1;
        EEPROM_writeAnything(0, foldern);

        // Read flash
        readFlash_NP();
      }
      else {
        print_Error(F("Switch to HiRom failed"), false);
      }
      break;

    // Write Flash
    case 1:
      filePath[0] = '\0';
      sd.chdir("/");
      // Launch file browser
      fileBrowser("Select 4MB file");
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);
      flashSize = 2097152;
      numBanks = 32;
      println_Msg(F("Writing 1st rom"));
      display_Update();
      // Program 1st flashrom
      write_NP(0xC0, 0);
      display_Clear();
      println_Msg(F("Writing 2nd rom"));
      display_Update();
      // Program 2nd flashrom
      write_NP(0xE0, 2097152);
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
      if (send_NP(0x04) == 0x2A) {
        println_Msg(F("OK"));
        display_Update();
        idFlash_NP(0xC0);
        if (strcmp(flashid, "c2f3") == 0) {
          idFlash_NP(0xE0);
          if (strcmp(flashid, "c2f3") == 0) {
            // Reset flash
            resetFlash_NP(0xC0);
            resetFlash_NP(0xE0);
            delay(100);
            // Clear screen
            display_Clear();
            printMapping();
            resetFlash_NP(0xC0);
            resetFlash_NP(0xE0);
          }
          else {
            print_Error(F("Error: Wrong Flash ID"), true);
          }
        }
        else {
          print_Error(F("Error: Wrong Flash ID"), true);
        }
      }
      else {
        print_Error(F("failed"), false);
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
      if (send_NP(0x04) == 0x2A) {
        println_Msg(F("OK"));
        display_Update();
        idFlash_NP(0xC0);
        if (strcmp(flashid, "c2f3") == 0) {
          idFlash_NP(0xE0);
          if (strcmp(flashid, "c2f3") == 0) {
            // Reset flash
            resetFlash_NP(0xC0);
            resetFlash_NP(0xE0);
            delay(100);
            readMapping();
            resetFlash_NP(0xC0);
            resetFlash_NP(0xE0);
          }
          else {
            print_Error(F("Error: Wrong Flash ID"), true);
          }
        }
        else {
          print_Error(F("Error: Wrong Flash ID"), true);
        }
      }
      else {
        print_Error(F("failed"), false);
      }
      break;

    // Write mapping
    case 4:
      // Clear screen
      display_Clear();

      // Reset to root directory
      sd.chdir("/");

      // Erase mapping
      eraseMapping(0xD0);
      eraseMapping(0xE0);
      print_Msg(F("Blankcheck..."));
      display_Update();
      if (blankcheckMapping()) {
        println_Msg(F("OK"));
        display_Update();
      }
      else {
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
      fileBrowser("Select MAP file");
      display_Clear();
      sprintf(filePath, "%s/%s", filePath, fileName);
      display_Update();

      // Write mapping
      writeMapping(0xD0, 0);
      writeMapping(0xE0, 256);
      break;

    // Go back
    case 5:
      mode = mode_NP;
      break;
  }
  if (flashSubMenu != 5) {
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
}

// Read the games from the menu area
void getGames() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SNES();

  // Check if menu is present
  byte menuString[] = {0x4D, 0x45, 0x4E, 0x55, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D};
  for (int i = 0; i < 12; i++) {
    if (menuString[i] != readBank_SNES(0xC0, 0x7FC0 + i)) {
      hasMenu = false;
    }
  }

  if (hasMenu) {
    // Count number of games
    for (word i = 0x0000; i < 0xE000; i += 0x2000) {
      if (readBank_SNES(0xC6, i) == numGames )
        numGames++;
    }

    // Get game info
    for (int i = 0; i < numGames; i++) {
      // Read starting address and size
      gameAddress[i] = 0xC0 + readBank_SNES(0xC6, i * 0x2000 + 0x01) * 0x8;
      gameSize[i] = readBank_SNES(0xC6, i * 0x2000 + 0x03) * 128;
      saveSize[i] = readBank_SNES(0xC6, i * 0x2000 + 0x05) / 8;

      //check if hirom
      if (readBank_SNES(gameAddress[i], 0xFFD5) == 0x31) {
        hirom[i] = true;
      }
      else if (readBank_SNES(gameAddress[i], 0xFFD5) == 0x21) {
        hirom[i] = true;
      }
      else {
        hirom[i] = false;
      }

      if (hirom[i]) {
        gameVersion[i] = readBank_SNES(gameAddress[i], 0xFFDB);
      } else {
        gameVersion[i] = readBank_SNES(gameAddress[i], 0x7FDB);
      }

      // Read game code
      byte myByte = 0;
      byte myLength = 0;
      for (int j = 0; j < 9; j++) {
        myByte = readBank_SNES(0xC6, i * 0x2000 + 0x07 + j);
        // Remove funny characters
        if (((char(myByte) >= 44 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 9) {
          gameCode[i][myLength] = char(myByte);
          myLength++;
        }
      }
      // End char array in case game code is less than 9 chars
      gameCode[i][myLength] = '\0';
    }
  }
  else {
    //check if hirom
    if (readBank_SNES(0xC0, 0xFFD5) == 0x31) {
      hirom[0] = true;
    }
    else {
      hirom[0] = false;
    }

    if (hirom[0]) {
      gameVersion[0] = readBank_SNES(0xC0, 0xFFDB);
      gameCode[0][0] = 'G';
      gameCode[0][1] = 'A';
      gameCode[0][2] = 'M';
      gameCode[0][3] = 'E';
      gameCode[0][4] = '-';
      gameCode[0][5] = char(readBank_SNES(0xC0, 0xFFB2));
      gameCode[0][6] = char(readBank_SNES(0xC0, 0xFFB3));
      gameCode[0][7] = char(readBank_SNES(0xC0, 0xFFB4));
      gameCode[0][8] = char(readBank_SNES(0xC0, 0xFFB5));
      gameCode[0][9] = '\0';

      byte romSizeExp = readBank_SNES(0xC0, 0xFFD7) - 7;
      gameSize[0] = 1;
      while (romSizeExp--)
        gameSize[0] *= 2;
    }
    else {
      gameVersion[0] = readBank_SNES(0xC0, 0x7FDB);
      gameCode[0][0] = 'G';
      gameCode[0][1] = 'A';
      gameCode[0][2] = 'M';
      gameCode[0][3] = 'E';
      gameCode[0][4] = '-';
      gameCode[0][5] = char(readBank_SNES(0xC0, 0x7FB2));
      gameCode[0][6] = char(readBank_SNES(0xC0, 0x7FB3));
      gameCode[0][7] = char(readBank_SNES(0xC0, 0x7FB4));
      gameCode[0][8] = char(readBank_SNES(0xC0, 0x7FB5));
      gameCode[0][9] = '\0';

      byte romSizeExp = readBank_SNES(0xC0, 0x7FD7) - 7;
      gameSize[0] = 1;
      while (romSizeExp--)
        gameSize[0] *= 2;
    }
  }
}

/******************************************
   Setup
 *****************************************/
void setup_NP() {
  // Set cicrstPin(PG1) to Output
  DDRG |= (1 << 1);
  // Output a high signal to disable snesCIC if installed
  PORTG |= (1 << 1);
  // Set cichstPin(PG0) to Input
  DDRG &= ~(1 << 0);

  // Adafruit Clock Generator
  //clockgen.set_correction(-29000);
  clockgen.set_correction(0);
  clockgen.init(SI5351_CRYSTAL_LOAD_8PF, 0);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  clockgen.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
  // Half Clock
  //clockgen.set_freq(1073863600ULL, SI5351_PLL_FIXED, SI5351_CLK0);
  // Full Clock
  clockgen.set_freq(2147727200ULL, SI5351_PLL_FIXED, SI5351_CLK0);
  // CIC Clock
  //clockgen.set_freq(307200000ULL, SI5351_PLL_FIXED, SI5351_CLK2);
  clockgen.output_enable(SI5351_CLK0, 1);
  clockgen.output_enable(SI5351_CLK1, 0);
  clockgen.output_enable(SI5351_CLK2, 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //BA0-BA7
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);

  // Set IRQ(PH4) to Input
  DDRH &= ~(1 << 4);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 4);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  PORTC = 0xFF;

  // Wait until all is stable
  delay(200);

  // Switch to HiRom All
  byte timeout = 0;
  send_NP(0x04);
  delay(100);
  while (readBank_SNES(0, 0x2400) != 0x2A) {
    delay(100);
    // Try again
    send_NP(0x04);
    delay(100);
    timeout++;
    // Abort, something is wrong
    if (timeout == 5) {
      println_Msg(F("Hirom All Timeout"));
      println_Msg(F(""));
      println_Msg(F(""));
      print_Error(F("Please powercycle NP cart"), true);
    }
  }
}

/******************************************
   29F1601 flashrom functions (NP)
 *****************************************/
// Reset the MX29F1601 flashrom, startbank is 0xC0 for first and 0xE0 for second flashrom
void resetFlash_NP(int startBank) {
  // Configure control pins
  controlOut_SNES();
  // Set data pins to output
  dataOut();

  // Reset command sequence
  if (romType) {
    writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
    writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SNES(startBank, 0x5555L * 2, 0xf0);
  }
  else {
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SNES(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xf0);
  }
}

// Print flashrom manufacturer and device ID
void idFlash_NP(int startBank) {
  // Configure control pins
  controlOut_SNES();
  // Set data pins to output
  dataOut();

  if (romType) {
    // ID command sequence
    writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
    writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SNES(startBank, 0x5555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SNES();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank_SNES(startBank, 0x00), readBank_SNES(startBank, 0x02));
  }
  else {
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SNES(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0x90);

    // Set data pins to input again
    dataIn();
    // Set control pins to input
    controlIn_SNES();

    // Read the two id bytes into a string
    sprintf(flashid, "%x%x", readBank_SNES(0, 0x8000), readBank_SNES(0, 0x8000 + 0x02));
  }
}

// Write the flashroms by reading a file from the SD card, pos defines where in the file the reading/writing should start
void writeFlash_NP(int startBank, uint32_t pos) {
  display_Clear();
  print_Msg(F("Writing Bank 0x"));
  print_Msg(startBank, HEX);
  print_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    // Configure control pins
    controlOut_SNES();
    // Set data pins to output
    dataOut();

    if (romType) {
      // Write hirom
      for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
        // Fill SDBuffer with 1 page at a time then write it repeat until all bytes are written
        for (unsigned long currByte = 0; currByte < 0x10000; currByte += 128) {
          myFile.read(sdBuffer, 128);
          // Write command sequence
          writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
          writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SNES(startBank, 0x5555L * 2, 0xa0);

          for (byte c = 0; c < 128; c++) {

            // Write one byte of data
            writeBank_SNES(currBank, currByte + c, sdBuffer[c]);

            if (c == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank_SNES(currBank, currByte + c, sdBuffer[c]);
            }
          }
          // Wait until write is finished
          busyCheck_NP(startBank);
        }
      }
    }
    else {
      // Write lorom
      for (int currBank = 0; currBank < numBanks; currBank++) {
        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 128) {
          myFile.read(sdBuffer, 128);
          // Write command sequence
          writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xaa);
          writeBank_SNES(0, 0x8000 + 0x2AAAL * 2, 0x55);
          writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xa0);

          for (byte c = 0; c < 128; c++) {
            // Write one byte of data
            writeBank_SNES(currBank, currByte + c, sdBuffer[c]);

            if (c == 127) {
              // Write the last byte twice or else it won't write at all
              writeBank_SNES(currBank, currByte + c, sdBuffer[c]);
            }
          }
          // Wait until write is finished
          busyCheck_NP(startBank);
        }
      }
    }
    // Close the file:
    myFile.close();
    println_Msg("");
  }
  else {
    print_Error(F("Can't open file on SD"), true);
  }
}

// Delay between write operations based on status register
void busyCheck_NP(byte startBank) {
  // Set data pins to input
  dataIn();
  // Set control pins to input and therefore pull CE low and latch status register content
  controlIn_SNES();

  // Read register
  readBank_SNES(startBank, 0x0000);

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
  controlOut_SNES();
  // Set data pins to output
  dataOut();
}

// Erase the flashrom to 0xFF
void eraseFlash_NP(int startBank) {

  // Configure control pins
  controlOut_SNES();
  // Set data pins to output
  dataOut();

  if (romType) {
    // Erase command sequence
    writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
    writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SNES(startBank, 0x5555L * 2, 0x80);
    writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
    writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
    writeBank_SNES(startBank, 0x5555L * 2, 0x10);
  }
  else {
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SNES(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0x80);
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0xaa);
    writeBank_SNES(0, 0x8000 + 0x2AAAL * 2, 0x55);
    writeBank_SNES(1, 0x8000 + 0x1555L * 2, 0x10);
  }

  // Wait for erase to complete
  busyCheck_NP(startBank);
}

// Check if an erase succeeded, return 1 if blank and 0 if not
byte blankcheck_NP(int startBank) {

  // Set data pins to input again
  dataIn();
  // Set control pins to input
  controlIn_SNES();

  byte blank = 1;
  if (romType) {
    for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte++) {
        if (readBank_SNES(currBank, currByte) != 0xFF) {
          currBank =  startBank + numBanks;
          blank = 0;
        }
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte++) {
        if (readBank_SNES(currBank, currByte) != 0xFF) {
          currBank = numBanks;
          blank = 0;
        }
      }
    }
  }
  return blank;
}

// Check if a write succeeded, returns 0 if all is ok and number of errors if not
unsigned long verifyFlash_NP(int startBank, uint32_t pos) {
  unsigned long  verified = 0;

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Set file starting position
    myFile.seekCur(pos);

    // Set data pins to input
    dataIn();
    // Set control pins to input
    controlIn_SNES();

    if (romType) {
      for (int currBank = startBank; currBank < startBank + numBanks; currBank++) {
        for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank_SNES(currBank, currByte + c) != sdBuffer[c]) {
              verified++;
            }
          }
        }
      }
    }
    else {
      for (int currBank = 0; currBank < numBanks; currBank++) {
        for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
          // Fill SDBuffer
          myFile.read(sdBuffer, 512);
          for (int c = 0; c < 512; c++) {
            if (readBank_SNES(currBank, currByte + c) != sdBuffer[c]) {
              verified++;
            }
          }
        }
      }
    }
    // Close the file:
    myFile.close();
  }
  else {
    // SD Error
    verified = 999999;
    print_Error(F("Can't open file on SD"), false);
  }
  // Return 0 if verified ok, or number of errors
  return verified;
}

// Read flashroms and save them to the SD card
void readFlash_NP() {
  // Set data pins to input
  dataIn();
  // Set control pins to input
  controlIn_SNES();

  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }
  if (romType) {
    for (int currBank = 0xC0; currBank < 0xC0 + numBanks; currBank++) {
      for (unsigned long currByte = 0; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
        }
        myFile.write(sdBuffer, 512);
      }
    }
  }
  else {
    for (int currBank = 0; currBank < numBanks; currBank++) {
      for (unsigned long currByte = 0x8000; currByte < 0x10000; currByte += 512) {
        for (int c = 0; c < 512; c++) {
          sdBuffer[c] = readBank_SNES(currBank, currByte + c);
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
void readSectorProtection_NP(byte startBank) {

  // Configure control pins
  controlOut_SNES();
  // Set data pins to output
  dataOut();

  // Display Sector Protection Status
  writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
  writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
  writeBank_SNES(startBank, 0x5555L * 2, 0x90);

  // Configure control pins
  controlIn_SNES();
  // Set data pins to output
  dataIn();
  display_Clear();
  for (int i = 0; i <= 0x1F; i++) {
    print_Msg(F("Sector: 0x"));
    print_Msg(startBank + i, HEX);
    print_Msg(F(" Sector Protect: 0x"));
    println_Msg(readBank_SNES(startBank + i, 0x04), HEX);
  }
  display_Update();
}

// Read the current mapping from the hidden "page buffer" and print it
void printMapping() {
  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset to defaults
  writeBank_SNES(0xC0, 0x0000, 0x38);
  writeBank_SNES(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SNES(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SNES(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SNES(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read the mapping out of the first chip
  char buffer[3];

  for (int currByte = 0xFF00; currByte < 0xFF50; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa (readBank_SNES(0xC0, currByte + c), buffer, 16);
      for (int i = 0; i < 2 - strlen(buffer); i++) {
        print_Msg("0");
      }
      // Now print the significant bits
      print_Msg(buffer);
    }
    println_Msg("");
  }
  display_Update();

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset Flash
  writeBank_SNES(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SNES(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SNES();
}

// Read the current mapping from the hidden "page buffer"
void readMapping() {

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset to defaults
  writeBank_SNES(0xC0, 0x0000, 0x38);
  writeBank_SNES(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SNES(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SNES(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SNES(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SNES();

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
    print_Error(F("SD Error"), true);
  }

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank_SNES(0xC0, currByte));
  }

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset to defaults
  writeBank_SNES(0xE0, 0x0000, 0x38);
  writeBank_SNES(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SNES(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SNES(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SNES(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    myFile.write(readBank_SNES(0xE0, currByte));
  }

  // Close the file:
  myFile.close();

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset Flash
  writeBank_SNES(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SNES(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Signal end of process
  print_Msg(F("Saved to NP/"));
  println_Msg(fileName);
  display_Update();
}

void eraseMapping(byte startBank) {

  if (unlockHirom()) {
    // Get ID
    idFlash_NP(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      resetFlash_NP(startBank);

      // Switch to write
      dataOut();
      controlOut_SNES();

      // Prepare to erase/write Page Buffer
      writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
      writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SNES(startBank, 0x5555L * 2, 0x77);
      // Erase Page Buffer
      writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
      writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
      writeBank_SNES(startBank, 0x5555L * 2, 0xe0);

      // Wait until complete
      busyCheck_NP(startBank);

      // Switch to read
      dataIn();
      controlIn_SNES();
    }
    else {
      print_Error(F("Error: Wrong Flash ID"), true);
    }
  }
  else {
    print_Error(F("Unlock failed"), true);
  }
}

// Check if the current mapping is all 0xFF
byte blankcheckMapping() {
  byte blank = 1;

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset to defaults
  writeBank_SNES(0xC0, 0x0000, 0x38);
  writeBank_SNES(0xC0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SNES(0xC0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SNES(0xC0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SNES(0xC0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SNES(0xC0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset to defaults
  writeBank_SNES(0xE0, 0x0000, 0x38);
  writeBank_SNES(0xE0, 0x0000, 0xd0);
  // Read Extended Status Register (GSR and PSR)
  writeBank_SNES(0xE0, 0x0000, 0x71);
  // Page Buffer Swap
  writeBank_SNES(0xE0, 0x0000, 0x72);
  // Read Page Buffer
  writeBank_SNES(0xE0, 0x0000, 0x75);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read the mapping info out of the 1st chip
  for (unsigned long currByte = 0xFF00; currByte <= 0xFFFF; currByte++) {
    if (readBank_SNES(0xE0, currByte) != 0xFF) {
      blank = 0;
    }
  }

  // Switch to write
  dataOut();
  controlOut_SNES();

  // Reset Flash
  writeBank_SNES(0xC0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xC0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xC0, 0x5555L * 2, 0xf0);

  // Reset Flash
  writeBank_SNES(0xE0, 0x5555L * 2, 0xaa);
  writeBank_SNES(0xE0, 0x2AAAL * 2, 0x55);
  writeBank_SNES(0xE0, 0x5555L * 2, 0xf0);

  // Switch to read
  dataIn();
  controlIn_SNES();

  return blank;
}

void writeMapping(byte startBank, uint32_t pos) {

  if (unlockHirom()) {
    // Get ID
    idFlash_NP(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      resetFlash_NP(startBank);

      // Switch to write
      dataOut();
      controlOut_SNES();

      // Open file on sd card
      if (myFile.open(filePath, O_READ)) {

        // Seek to a new position in the file
        if (pos != 0)
          myFile.seekCur(pos);

        // Write to Page Buffer
        for (unsigned long currByte = 0xFF00; currByte < 0xFFFF; currByte += 128) {
          // Prepare to erase/write Page Buffer
          writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
          writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SNES(startBank, 0x5555L * 2, 0x77);
          // Write Page Buffer Command
          writeBank_SNES(startBank, 0x5555L * 2, 0xaa);
          writeBank_SNES(startBank, 0x2AAAL * 2, 0x55);
          writeBank_SNES(startBank, 0x5555L * 2, 0x99);

          myFile.read(sdBuffer, 128);

          for (byte c = 0; c < 128; c++) {
            writeBank_SNES(startBank, currByte + c, sdBuffer[c]);
            // Write last byte twice
            if (c == 127) {
              writeBank_SNES(startBank, currByte + c, sdBuffer[c]);
            }
          }
          busyCheck_NP(startBank);
        }

        // Close the file:
        myFile.close();
        println_Msg("");
      }
      else {
        print_Error(F("Can't open file on SD"), false);
      }

      // Switch to read
      dataIn();
      controlIn_SNES();
    }
    else {
      print_Error(F("Error: Wrong Flash ID"), true);
    }
  }
  else {
    print_Error(F("Unlock failed"), true);
  }
}

/******************************************
  Nintendo Power functions
*****************************************/
// Switch to HiRom All and unlock Write Protection
boolean unlockHirom() {
  romType = 1;
  print_Msg(F("Switch to HiRom..."));
  display_Update();
  if (send_NP(0x04) == 0x2A) {
    println_Msg(F("OK"));
    display_Update();
    // Unlock Write Protection
    print_Msg(F("Enable Write..."));
    display_Update();
    send_NP(0x02);
    if (readBank_SNES(0, 0x2401) == 0x4) {
      println_Msg(F("OK"));
      display_Update();
      return 1;
    }
    else {
      println_Msg(F("failed"));
      display_Update();
      return 0;
    }
  }
  else {
    println_Msg(F("failed"));
    display_Update();
    return 0;
  }
}

// Send a command to the MX15001 chip
byte send_NP(byte command) {
  // Switch to write
  dataOut();
  controlOut_SNES();

  // Write command
  writeBank_SNES(0, 0x2400, 0x09);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read status
  NPReady = readBank_SNES(0, 0x2400);

  // Switch to write
  dataOut();
  controlOut_SNES();

  writeBank_SNES(0, 0x2401, 0x28);
  writeBank_SNES(0, 0x2401, 0x84);

  // NP_CMD_06h, send this only if above read has returned 7Dh, not if it's already returning 2Ah
  if (NPReady == 0x7D) {
    writeBank_SNES(0, 0x2400, 0x06);
    writeBank_SNES(0, 0x2400, 0x39);
  }

  // Write the command
  writeBank_SNES(0, 0x2400, command);

  // Switch to read
  dataIn();
  controlIn_SNES();

  // Read status
  NPReady = readBank_SNES(0, 0x2400);
  return NPReady;
}

// This function will erase and program the NP cart from a 4MB file off the SD card
void write_NP(int startBank, uint32_t pos) {
  // Switch NP cart's mapping
  if (unlockHirom()) {
    // Get ID
    idFlash_NP(startBank);
    if (strcmp(flashid, "c2f3") == 0) {
      print_Msg(F("Flash ID: "));
      println_Msg(flashid);
      display_Update();
      resetFlash_NP(startBank);
      delay(1000);
      // Erase flash
      print_Msg(F("Blankcheck..."));
      display_Update();
      if (blankcheck_NP(startBank)) {
        println_Msg(F("OK"));
        display_Update();
      }
      else {
        println_Msg(F("Nope"));
        display_Clear();
        print_Msg(F("Erasing..."));
        display_Update();
        eraseFlash_NP(startBank);
        resetFlash_NP(startBank);
        println_Msg(F("Done"));
        print_Msg(F("Blankcheck..."));
        display_Update();
        if (blankcheck_NP(startBank)) {
          println_Msg(F("OK"));
          display_Update();
        }
        else {
          print_Error(F("Could not erase flash"), true);
        }
      }
      // Write flash
      writeFlash_NP(startBank, pos);

      // Reset flash
      resetFlash_NP(startBank);

      // Checking for errors
      print_Msg(F("Verifying..."));
      display_Update();
      writeErrors = verifyFlash_NP(startBank, pos);
      if (writeErrors == 0) {
        println_Msg(F("OK"));
        display_Update();
      }
      else {
        print_Msg(F("Error: "));
        print_Msg(writeErrors);
        println_Msg(F(" bytes "));
        print_Error(F("did not verify."), true);
      }
    }
    else {
      print_Error(F("Error: Wrong Flash ID"), true);
    }
  }
  else {
    print_Error(F("Unlock failed"), true);
  }
}

//******************************************
// End of File
//******************************************
