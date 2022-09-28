//******************************************
// GAME BOY ADVANCE MODULE
//******************************************
#ifdef enable_GBX

/******************************************
   Variables
 *****************************************/
char calcChecksumStr[5];
boolean readType;

/******************************************
   Menu
 *****************************************/
// GBA menu items
static const char GBAMenuItem1[] PROGMEM = "Read ROM";
static const char GBAMenuItem2[] PROGMEM = "Read Save";
static const char GBAMenuItem3[] PROGMEM = "Write Save";
static const char GBAMenuItem4[] PROGMEM = "Force Savetype";
static const char GBAMenuItem5[] PROGMEM = "Flash Repro";
static const char GBAMenuItem6[] PROGMEM = "Reset";
static const char* const menuOptionsGBA[] PROGMEM = {GBAMenuItem1, GBAMenuItem2, GBAMenuItem3, GBAMenuItem4, GBAMenuItem5, GBAMenuItem6};

// Rom menu
static const char GBARomItem1[] PROGMEM = "1 MB";
static const char GBARomItem2[] PROGMEM = "2 MB";
static const char GBARomItem3[] PROGMEM = "4 MB";
static const char GBARomItem4[] PROGMEM = "8 MB";
static const char GBARomItem5[] PROGMEM = "16 MB";
static const char GBARomItem6[] PROGMEM = "32 MB";
static const char* const romOptionsGBA[] PROGMEM = {GBARomItem1, GBARomItem2, GBARomItem3, GBARomItem4, GBARomItem5, GBARomItem6};

// Save menu
static const char GBASaveItem1[] PROGMEM = "4K EEPROM";
static const char GBASaveItem2[] PROGMEM = "64K EEPROM";
static const char GBASaveItem3[] PROGMEM = "256K SRAM/FRAM";
static const char GBASaveItem4[] PROGMEM = "512K SRAM/FRAM";
static const char GBASaveItem5[] PROGMEM = "512K FLASH";
static const char GBASaveItem6[] PROGMEM = "1M FLASH";
static const char* const saveOptionsGBA[] PROGMEM = {GBASaveItem1, GBASaveItem2, GBASaveItem3, GBASaveItem4, GBASaveItem5, GBASaveItem6};

void gbaMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBA, 6);
  mainMenu = question_box(F("GBA Cart Reader"), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      // Read rom
      switch (cartSize)
      {
        case 0:
          // create submenu with title and 4 options to choose from
          unsigned char GBARomMenu;
          // Copy menuOptions out of progmem
          convertPgm(romOptionsGBA, 6);
          GBARomMenu = question_box(F("Select ROM size"), menuOptions, 6, 0);

          // wait for user choice to come back from the question box menu
          switch (GBARomMenu)
          {
            case 0:
              // 1MB
              cartSize = 0x100000;
              break;

            case 1:
              // 2MB
              cartSize = 0x200000;
              break;

            case 2:
              // 4MB
              cartSize = 0x400000;
              break;

            case 3:
              // 8MB
              cartSize = 0x800000;
              break;

            case 4:
              // 16MB
              cartSize = 0x1000000;
              break;

            case 5:
              // 32MB
              cartSize = 0x2000000;
              break;
          }
          break;

        case 1:
          // 1MB
          cartSize = 0x100000;
          break;

        case 4:
          // 4MB
          cartSize = 0x400000;
          break;

        case 8:
          // 8MB
          cartSize = 0x800000;
          break;

        case 16:
          // 16MB
          cartSize = 0x1000000;
          break;

        case 32:
          // 32MB
          cartSize = 0x2000000;
          break;
      }
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_GBA();
      sd.chdir("/");
      // Internal Checksum
      compare_checksum_GBA();
      // CRC32
      compareCRC("gba.txt", 0, 1, 0);
#ifdef global_log
      save_log();
#endif
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    case 1:
      // Read save
      if (saveType == 0) {
        // create submenu with title and 6 options to choose from
        unsigned char GBASaveMenu;
        // Copy menuOptions out of progmem
        convertPgm(saveOptionsGBA, 6);
        GBASaveMenu = question_box(F("Select save type"), menuOptions, 6, 0);

        // wait for user choice to come back from the question box menu
        switch (GBASaveMenu)
        {
          case 0:
            // 4K EEPROM
            saveType = 1;
            break;

          case 1:
            // 64K EEPROM
            saveType = 2;
            break;

          case 2:
            // 256K SRAM/FRAM
            saveType = 3;
            break;

          case 3:
            // 512K SRAM/FRAM
            saveType = 6;
            break;

          case 4:
            // 512K FLASH
            saveType = 4;
            break;

          case 5:
            // 1M FLASH
            saveType = 5;
            break;
        }
      }
      switch (saveType)
      {
        case 1:
          display_Clear();
          sd.chdir("/");
          // 4K EEPROM
          readEeprom_GBA(4);
          setROM_GBA();
          break;

        case 2:
          display_Clear();
          sd.chdir("/");
          // 64K EEPROM
          readEeprom_GBA(64);
          setROM_GBA();
          break;

        case 3:
          display_Clear();
          sd.chdir("/");
          // 256K SRAM/FRAM
          readSRAM_GBA(1, 32768, 0);
          setROM_GBA();
          break;

        case 4:
          display_Clear();
          sd.chdir("/");
          // 512K FLASH
          readFLASH_GBA(1, 65536, 0);
          setROM_GBA();
          break;

        case 5:
          display_Clear();
          sd.chdir("/");
          // 1M FLASH (divided into two banks)
          switchBank_GBA(0x0);
          setROM_GBA();
          readFLASH_GBA(1, 65536, 0);
          switchBank_GBA(0x1);
          setROM_GBA();
          readFLASH_GBA(0, 65536, 65536);
          setROM_GBA();
          break;

        case 6:
          display_Clear();
          sd.chdir("/");
          // 512K SRAM/FRAM
          readSRAM_GBA(1, 65536, 0);
          setROM_GBA();
          break;
      }
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    case 2:
      // Write save
      if (saveType == 0) {
        // create submenu with title and 6 options to choose from
        unsigned char GBASavesMenu;
        // Copy menuOptions out of progmem
        convertPgm(saveOptionsGBA, 6);
        GBASavesMenu = question_box(F("Select save type"), menuOptions, 6, 0);
        // wait for user choice to come back from the question box menu
        switch (GBASavesMenu)
        {
          case 0:
            // 4K EEPROM
            saveType = 1;
            break;

          case 1:
            // 64K EEPROM
            saveType = 2;
            break;

          case 2:
            // 256K SRAM/FRAM
            saveType = 3;
            break;

          case 3:
            // 512K SRAM/FRAM
            saveType = 6;
            break;

          case 4:
            // 512K FLASH
            saveType = 4;
            break;

          case 5:
            // 1M FLASH
            saveType = 5;
            break;
        }
      }

      switch (saveType)
      {
        case 1:
          display_Clear();
          sd.chdir("/");
          // 4K EEPROM
          writeEeprom_GBA(4);
          writeErrors = verifyEEP_GBA(4);
          if (writeErrors == 0) {
            println_Msg(F("Verified OK"));
            display_Update();
          }
          else {
            print_Msg(F("Error: "));
            print_Msg(writeErrors);
            println_Msg(F(" bytes "));
            print_Error(F("did not verify."), false);
          }
          setROM_GBA();
          break;

        case 2:
          display_Clear();
          sd.chdir("/");
          // 64K EEPROM
          writeEeprom_GBA(64);
          writeErrors = verifyEEP_GBA(64);
          if (writeErrors == 0) {
            println_Msg(F("Verified OK"));
            display_Update();
          }
          else {
            print_Msg(F("Error: "));
            print_Msg(writeErrors);
            println_Msg(F(" bytes "));
            print_Error(F("did not verify."), false);
          }
          setROM_GBA();
          break;

        case 3:
          display_Clear();
          // Change working dir to root
          sd.chdir("/");
          // 256K SRAM/FRAM
          writeSRAM_GBA(1, 32768, 0);
          writeErrors = verifySRAM_GBA(32768, 0);
          if (writeErrors == 0) {
            println_Msg(F("Verified OK"));
            display_Update();
          }
          else {
            print_Msg(F("Error: "));
            print_Msg(writeErrors);
            println_Msg(F(" bytes "));
            print_Error(F("did not verify."), false);
          }
          setROM_GBA();
          break;

        case 4:
          display_Clear();
          sd.chdir("/");
          // 512K FLASH
          idFlash_GBA();
          resetFLASH_GBA();

          print_Msg(F("FLASH ID: "));
          println_Msg(flashid);
          println_Msg(F(""));
          println_Msg(F("FLASH Type: "));
          if (strcmp(flashid, "1F3D") == 0) {
            println_Msg(F("Atmel AT29LV512"));
          }
          else if (strcmp(flashid, "BFD4") == 0) {
            println_Msg(F("SST 39VF512"));
          }
          else if (strcmp(flashid, "C21C") == 0) {
            println_Msg(F("Macronix MX29L512"));
          }
          else if (strcmp(flashid, "321B") == 0) {
            println_Msg(F("Panasonic MN63F805MNP"));
          }
          else {
            println_Msg(F("Unknown"));
            //print_Error(F(""), true);
          }
          println_Msg(F(""));
          println_Msg(F("Press Button..."));
          display_Update();
          wait();
          display_Clear();
          display_Update();

          if (strcmp(flashid, "1F3D") == 0) { // Atmel
            writeFLASH_GBA(1, 65536, 0, 1);
            verifyFLASH_GBA(65536, 0);
          } else {
            eraseFLASH_GBA();
            if (blankcheckFLASH_GBA(65536)) {
              writeFLASH_GBA(1, 65536, 0, 0);
              verifyFLASH_GBA(65536, 0);
            }
            else {
              print_Error(F("Erase failed"), false);
            }
          }
          setROM_GBA();
          break;

        case 5:
          display_Clear();
          sd.chdir("/");
          // 1M FLASH
          idFlash_GBA();
          resetFLASH_GBA();

          print_Msg(F("Flashrom ID: "));
          println_Msg(flashid);
          println_Msg(F(""));
          println_Msg(F("Flashrom Type: "));
          if (strcmp(flashid, "C209") == 0) {
            println_Msg(F("Macronix MX29L010"));
          }
          else if (strcmp(flashid, "6213") == 0) {
            println_Msg(F("SANYO LE26FV10N1TS"));
          }
          else {
            println_Msg(F("Unknown"));
            //print_Error(F(""), true);
          }
          println_Msg(F(""));
          println_Msg(F("Press Button..."));
          display_Update();
          wait();
          display_Clear();
          display_Update();

          eraseFLASH_GBA();
          // 131072 bytes are divided into two 65536 byte banks
          switchBank_GBA(0x0);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(1, 65536, 0, 0);
            verifyFLASH_GBA(65536, 0);
          }
          else {
            print_Error(F("Erase failed"), false);
          }
          switchBank_GBA(0x1);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(0, 65536, 65536, 0);
            verifyFLASH_GBA(65536, 65536);
          }
          else {
            print_Error(F("Erase failed"), false);
          }
          setROM_GBA();
          break;

        case 6:
          display_Clear();
          // Change working dir to root
          sd.chdir("/");
          // 512K SRAM/FRAM
          writeSRAM_GBA(1, 65536, 0);
          writeErrors = verifySRAM_GBA(65536, 0);
          if (writeErrors == 0) {
            println_Msg(F("Verified OK"));
            display_Update();
          }
          else {
            print_Msg(F("Error: "));
            print_Msg(writeErrors);
            println_Msg(F(" bytes "));
            print_Error(F("did not verify."), false);
          }
          setROM_GBA();
          break;
      }
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      break;

    case 3:
      display_Clear();
      // create submenu with title and 7 options to choose from
      unsigned char GBASaveMenu;
      // Copy menuOptions out of progmem
      convertPgm(saveOptionsGBA, 6);
      GBASaveMenu = question_box(F("Select save type"), menuOptions, 6, 0);

      // wait for user choice to come back from the question box menu
      switch (GBASaveMenu)
      {
        case 0:
          // 4K EEPROM
          saveType = 1;
          break;

        case 1:
          // 64K EEPROM
          saveType = 2;
          break;

        case 2:
          // 256K SRAM/FRAM
          saveType = 3;
          break;

        case 3:
          // 512K SRAM/FRAM
          saveType = 6;
          break;

        case 4:
          // 512K FLASH
          saveType = 4;
          break;

        case 5:
          // 1M FLASH
          saveType = 5;
          break;
      }
      display_Clear();
      break;

    case 4:
      display_Clear();
      flashRepro_GBA();
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      resetArduino();
      break;

    case 5:
      resetArduino();
      break;
  }

}

/******************************************
   Setup
 *****************************************/
void setup_GBA() {
  setROM_GBA();

  // Get cart info
  getCartInfo_GBA();
  display_Clear();

  // Print start page
  print_Msg(F("Title: "));
  println_Msg(romName);
  print_Msg(F("Serial: "));
  println_Msg(cartID);
  print_Msg(F("Revision: "));
  println_Msg(romVersion);
  print_Msg(F("ROM Size: "));
  if (cartSize == 0)
    println_Msg(F("Unknown"));
  else {
    print_Msg(cartSize);
    println_Msg(F(" MB"));
  }
  print_Msg(F("Save Type: "));
  switch (saveType)
  {
    case 0:
      println_Msg(F("None/Unknown"));
      break;

    case 1:
      println_Msg(F("4K EEPROM"));
      break;

    case 2:
      println_Msg(F("64K EEPROM"));
      break;

    case 3:
      println_Msg(F("256K SRAM"));
      break;

    case 4:
      println_Msg(F("512K FLASH"));
      break;

    case 5:
      println_Msg(F("1M FLASH"));
      break;
  }
  print_Msg(F("Header Checksum: "));
  println_Msg(checksumStr);

  // Wait for user input
  println_Msg("");
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Low level functions
*****************************************/
void setROM_GBA() {
  // CS_SRAM(PH0)
  DDRH |= (1 << 0); PORTH |= (1 << 0);
  // CS_ROM(PH3)
  DDRH |= (1 << 3); PORTH |= (1 << 3);
  // WR(PH5)
  DDRH |= (1 << 5); PORTH |= (1 << 5);
  // RD(PH6)
  DDRH |= (1 << 6); PORTH |= (1 << 6);
  // AD0-AD7
  DDRF = 0xFF;
  // AD8-AD15
  DDRK = 0xFF;
  // AD16-AD23
  DDRC = 0xFF;
  // Wait
  delay(500);
}

word readWord_GBA(unsigned long myAddress) {
  // Set address/data ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  // Divide address by two to get word addressing
  myAddress = myAddress >> 1;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  PORTC = myAddress >> 16;

  // Pull CS(PH3) to LOW
  PORTH &= ~ (1 << 3);

  // Set address/data ports to input
  PORTF = 0x0;
  PORTK = 0x0;
  DDRF = 0x0;
  DDRK = 0x0;

  // Pull RD(PH6) to LOW
  PORTH &= ~ (1 << 6);

  // Delay here or read error with repro
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  word myWord = (PINK << 8) | PINF;

  // Switch RD(PH6) to HIGH
  PORTH |= (1 << 6);

  // Switch CS_ROM(PH3) to HIGH
  PORTH |= (1 << 3);

  return myWord;
}

void writeWord_GBA(unsigned long myAddress, word myWord) {
  // Set address/data ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  // Divide address by two to get word addressing
  myAddress = myAddress >> 1;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  PORTC = myAddress >> 16;

  // Pull CS(PH3) to LOW
  PORTH &= ~ (1 << 3);

  __asm__("nop\n\t""nop\n\t");

  // Output data
  PORTF = myWord & 0xFF;
  PORTK = myWord >> 8;

  // Pull WR(PH5) to LOW
  PORTH &= ~ (1 << 5);

  __asm__("nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Switch CS_ROM(PH3) to HIGH
  PORTH |= (1 << 3);
}

// This function swaps bit at positions p1 and p2 in an integer n
word swapBits(word n, word p1, word p2)
{
  // Move p1'th to rightmost side
  word bit1 =  (n >> p1) & 1;

  // Move p2'th to rightmost side
  word bit2 =  (n >> p2) & 1;

  // XOR the two bits */
  word x = (bit1 ^ bit2);

  // Put the xor bit back to their original positions
  x = (x << p1) | (x << p2);

  // XOR 'x' with the original number so that the two sets are swapped
  word result = n ^ x;

  return result;
}

// Some repros have D0 and D1 switched
word readWord_GAB(unsigned long myAddress) {
  word tempWord = swapBits(readWord_GBA(myAddress), 0, 1);
  return tempWord;
}

void writeWord_GAB(unsigned long myAddress, word myWord) {
  writeWord_GBA(myAddress, swapBits(myWord, 0, 1));
}

byte readByte_GBA(unsigned long myAddress) {
  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to input
  DDRC = 0x0;

  // Output address to address pins,
  PORTF = myAddress;
  PORTK = myAddress >> 8;

  // Pull OE_SRAM(PH6) to LOW
  PORTH &= ~(1 << 6);
  // Pull CE_SRAM(PH0) to LOW
  PORTH &= ~(1 << 0);

  // Hold address for at least 25ns and wait 150ns before access
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read byte
  byte tempByte = PINC;

  // Pull CE_SRAM(PH0) HIGH
  PORTH |= (1 << 0);
  // Pull OE_SRAM(PH6) HIGH
  PORTH |= (1 << 6);

  return tempByte;
}

void writeByte_GBA(unsigned long myAddress, byte myData) {
  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to output
  DDRC = 0xFF;

  // Output address to address pins
  PORTF = myAddress;
  PORTK = myAddress >> 8;
  // Output data to data pins
  PORTC = myData;

  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WE_SRAM(PH5) to LOW
  PORTH &= ~(1 << 5);
  // Pull CE_SRAM(PH0) to LOW
  PORTH &= ~(1 << 0);

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull CE_SRAM(PH0) HIGH
  PORTH |= (1 << 0);
  // Pull WE_SRAM(PH5) HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

/******************************************
  GBA ROM Functions
*****************************************/
// Read info out of rom header
void getCartInfo_GBA() {
  char saveTypeStr[14];

  // Read Header into array
  for (int currWord = 0; currWord < 192; currWord += 2) {
    word tempWord = readWord_GBA(currWord);

    sdBuffer[currWord] = tempWord & 0xFF;
    sdBuffer[currWord + 1] = (tempWord >> 8) & 0xFF;
  }

  // Compare Nintendo logo against known checksum, 156 bytes starting at 0x04
  word logoChecksum = 0;
  for (int currByte = 0x4; currByte < 0xA0; currByte++) {
    logoChecksum += sdBuffer[currByte];
  }

  if (logoChecksum != 0x4B1B) {
    display_Clear();
    print_Error(F("CARTRIDGE ERROR"), false);
    strcpy(romName, "ERROR");
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F("Press Button to"));
    println_Msg(F("ignore or powercycle"));
    println_Msg(F("to try again"));
    display_Update();
    wait();
  }
  else {
    char tempStr2[2];
    char tempStr[5];

    // cart not in list
    cartSize = 0;
    saveType = 0;

    // Get cart ID
    cartID[0] = char(sdBuffer[0xAC]);
    cartID[1] = char(sdBuffer[0xAD]);
    cartID[2] = char(sdBuffer[0xAE]);
    cartID[3] = char(sdBuffer[0xAF]);

    display_Clear();
    println_Msg(F("Searching database..."));
    display_Update();

    //go to root
    sd.chdir();
    if (myFile.open("gba.txt", O_READ)) {
      char gamename[100];

      // Loop through file
      while (myFile.available()) {
        // Skip first line with name
        skip_line(&myFile);

        // Skip over the CRC checksum
        myFile.seekSet(myFile.curPosition() + 9);

        // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
        sprintf(tempStr, "%c", myFile.read());
        for (byte i = 0; i < 3; i++) {
          sprintf(tempStr2, "%c", myFile.read());
          strcat(tempStr, tempStr2);
        }

        // Check if string is a match
        if (strcmp(tempStr, cartID) == 0) {
          // Rewind to start of entry
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

          // Display database
          while (myFile.available()) {
            display_Clear();

            // Read game name
#if defined(enable_OLED)
            get_line(gamename, &myFile, 42);
#else
            get_line(gamename, &myFile, 96);
#endif

            // Skip over the CRC checksum
            myFile.seekSet(myFile.curPosition() + 9);

            // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
            sprintf(tempStr, "%c", myFile.read());
            for (byte i = 0; i < 3; i++) {
              sprintf(tempStr2, "%c", myFile.read());
              strcat(tempStr, tempStr2);
            }

            // Skip the , in the file
            myFile.seekSet(myFile.curPosition() + 1);

            // Read the next ascii character and subtract 48 to convert to decimal
            cartSize = myFile.read() - 48;
            // Remove leading 0 for single digit cart sizes
            if (cartSize != 0) {
              cartSize = cartSize * 10 +  myFile.read() - 48;
            }
            else {
              cartSize = myFile.read() - 48;
            }

            // Skip the , in the file
            myFile.seekSet(myFile.curPosition() + 1);

            // Read save type into string
            get_line(saveTypeStr, &myFile, 14);

            // skip third empty line
            skip_line(&myFile);

            // Print current database entry
            println_Msg(gamename);
            print_Msg(F("Serial: "));
            println_Msg(tempStr);
            print_Msg(F("ROM Size: "));
            print_Msg(cartSize);
            println_Msg(F(" MB"));
            print_Msg(F("Save Lib: "));
            println_Msg(saveTypeStr);

#if defined(enable_OLED)
            println_Msg(F("Press left to Change"));
            println_Msg(F("and right to Select"));
#elif defined(enable_LCD)
            println_Msg(F(""));
            println_Msg(F("Rotate to Change"));
            println_Msg(F("Press to Select"));
#elif defined(SERIAL_MONITOR)
            println_Msg(F(""));
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
                // Break out of loop to read next entry
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

              // Selection made
              else if (b == 3) {
                // Close file and break to exit both loops
                myFile.close();
                break;
              }
            }
          }
        }
        // If no match advance and try again
        else {
          // skip rest of line
          skip_line(&myFile);
          // skip third empty line
          skip_line(&myFile);
        }
      }
      // Close the file:
      myFile.close();
    }
    else {
      print_Error(F("GBA.txt missing"), true);
    }

    // Get name
    byte myByte = 0;
    byte myLength = 0;
    for (int addr = 0xA0; addr <= 0xAB; addr++) {
      myByte = sdBuffer[addr];
      if (isprint(myByte) && myByte != '<' && myByte != '>' && myByte != ':' && myByte != '"' && myByte != '/' && myByte != '\\' && myByte != '|' && myByte != '?' && myByte != '*') {
        romName[myLength] = char(myByte);
      } else {
        if (romName[myLength - 1] == 0x5F) myLength--;
        romName[myLength] = 0x5F;
      }
      myLength++;
    }

    // Strip trailing white space
    for (unsigned int i = myLength - 1; i > 0; i--) {
      if ((romName[i] != 0x5F) && (romName[i] != 0x20)) break;
      romName[i] = 0x00;
      myLength--;
    }

    // Get ROM version
    romVersion = sdBuffer[0xBC];

    // Get Checksum as string
    sprintf(checksumStr, "%02X", sdBuffer[0xBD]);

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;
    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);

    // Compare checksum
    if (strcmp(calcChecksumStr, checksumStr) != 0) {
      display_Clear();
      print_Msg(F("Result: "));
      println_Msg(calcChecksumStr);
      print_Error(F("Checksum Error"), false);
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
    }

    /* Convert saveTypeStr to saveType
      Save types in ROM
      EEPROM_Vnnn    EEPROM 512 bytes or 8 Kbytes (4Kbit or 64Kbit)
      SRAM_Vnnn      SRAM 32 Kbytes (256Kbit)
      SRAM_F_Vnnn    FRAM 32 Kbytes (256Kbit)
      FLASH_Vnnn     FLASH 64 Kbytes (512Kbit) (ID used in older files)
      FLASH512_Vnnn  FLASH 64 Kbytes (512Kbit) (ID used in newer files)
      FLASH1M_Vnnn   FLASH 128 Kbytes (1Mbit)

      Save types in Cart Reader Code
      0 = Unknown or no save
      1 = 4K EEPROM
      2 = 64K EEPROM
      3 = 256K SRAM
      4 = 512K FLASH
      5 = 1M FLASH
      6 = 512K SRAM
    */

    if (saveTypeStr[0] == 'N') {
      saveType = 0;
    }
    else if (saveTypeStr[0] == 'E') {
      // Test if 4kbit or 64kbit EEPROM

      // Disable interrupts for more uniform clock pulses
      noInterrupts();
      // Fill sd Buffer
      readBlock_EEP(0, 64);
      interrupts();
      delay(1000);
      // Enable ROM again
      setROM_GBA();

      saveType = 1;

      // Reading 4kbit EEPROM as 64kbit just gives the same 8 bytes repeated
      for (int currByte = 0; currByte < 512 - 8; currByte++) {
        if (sdBuffer[currByte] != sdBuffer[currByte + 8]) {
          saveType = 2;
          break;
        }
      }
    }
    else if (saveTypeStr[0] == 'S') {
      saveType = 3;
    }
    else if ((saveTypeStr[0] == 'F') && (saveTypeStr[5] == '1')) {
      saveType = 5;
    }
    else if (saveTypeStr[0] == 'F') {
      saveType = 4;
    }
  }
}

// Dump ROM
void readROM_GBA() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // create a new folder for the rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GBA/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  //clear the screen
  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize);
  draw_progressbar(0, totalProgressBar);

  // Read rom
  for (unsigned long myAddress = 0; myAddress < cartSize; myAddress += 512) {
    // Blink led
    if (myAddress % 16384 == 0)
      blinkLED();

    for (int currWord = 0; currWord < 512; currWord += 2) {
      word tempWord = readWord_GBA(myAddress + currWord);
      sdBuffer[currWord] = tempWord & 0xFF;
      sdBuffer[currWord + 1] = (tempWord >> 8) & 0xFF;
    }

    // Write to SD
    myFile.write(sdBuffer, 512);

    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

  // Close the file:
  myFile.close();
}

// Calculate the checksum of the dumped rom
boolean compare_checksum_GBA () {
  print_Msg(F("Checksum: "));
  display_Update();

  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // last used rom folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GBA/ROM/%s/%d", romName, foldern - 1);
  sd.chdir(folder);

  // If file exists
  if (myFile.open(fileName, O_READ)) {
    // Read rom header
    myFile.read(sdBuffer, 512);
    myFile.close();

    // Calculate Checksum
    int calcChecksum = 0x00;
    for (int n = 0xA0; n < 0xBD; n++) {
      calcChecksum -= sdBuffer[n];
    }
    calcChecksum = (calcChecksum - 0x19) & 0xFF;

    // Turn into string
    sprintf(calcChecksumStr, "%02X", calcChecksum);
    print_Msg(calcChecksumStr);

    if (strcmp(calcChecksumStr, checksumStr) == 0) {
      println_Msg(F(" -> OK"));
      display_Update();
      return 1;
    }
    else {
      print_Msg(F(" != "));
      println_Msg(checksumStr);
      print_Error(F("Invalid Checksum"), false);
      return 0;
    }
  }
  // Else show error
  else {
    print_Error(F("Failed to open rom"), false);
    return 0;
  }
}


/******************************************
  GBA SRAM SAVE Functions
*****************************************/
void readSRAM_GBA(boolean browseFile, unsigned long sramSize, uint32_t pos) {
  if (browseFile) {
    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".srm");

    // create a new folder for the save file
    EEPROM_readAnything(0, foldern);
    sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // Save location
    print_Msg(F("Saving to "));
    print_Msg(folder);
    println_Msg(F("/..."));
    display_Update();
    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(0, foldern);
  }

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    myFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByte_GBA(currAddress + c);
    }

    // Write sdBuffer to file
    myFile.write(sdBuffer, 512);
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  println_Msg(F("Done"));
  display_Update();
}

void writeSRAM_GBA(boolean browseFile, unsigned long sramSize, uint32_t pos) {
  if (browseFile) {
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select srm file"));
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
  }

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);

      for (int c = 0; c < 512; c++) {
        // Write byte
        writeByte_GBA(currAddress + c, sdBuffer[c]);
      }
    }
    // Close the file:
    myFile.close();
    println_Msg(F("SRAM writing finished"));
    display_Update();

  }
  else {
    print_Error(F("File doesnt exist"), false);
  }
}

unsigned long verifySRAM_GBA(unsigned long sramSize, uint32_t pos) {
  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Variable for errors
    writeErrors = 0;

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    for (unsigned long currAddress = 0; currAddress < sramSize; currAddress += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);

      for (int c = 0; c < 512; c++) {
        // Read byte
        if (readByte_GBA(currAddress + c) != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    print_Error(F("Can't open file"), false);
  }
}

/******************************************
  GBA FRAM SAVE Functions
*****************************************/
// MB85R256 FRAM (Ferroelectric Random Access Memory) 32,768 words x 8 bits
void readFRAM_GBA (unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".srm");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // Save location
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }
  for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Pull OE_SRAM(PH6) HIGH
      PORTH |= (1 << 6);

      // Set address
      PORTF = (currAddress + c) & 0xFF;
      PORTK = ((currAddress + c) >> 8) & 0xFF;

      // Arduino running at 16Mhz -> one nop = 62.5ns
      // Leave CS_SRAM HIGH for at least 85ns
      __asm__("nop\n\t""nop\n\t");

      // Pull OE_SRAM(PH6) LOW
      PORTH &= ~ (1 << 6);

      // Hold address for at least 25ns and wait 150ns before access
      __asm__("nop\n\t""nop\n\t""nop\n\t");

      // Read byte
      sdBuffer[c] = PINC;
    }
    // Write sdBuffer to file
    myFile.write(sdBuffer, 512);
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  println_Msg(F("Done"));
  display_Update();
}

// Write file to SRAM
void writeFRAM_GBA (boolean browseFile, unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) and OE_SRAM(PH6)
  PORTH |= (1 << 3) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data port to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_SRAM(PH0) and WE_SRAM(PH5)
  PORTH &= ~((1 << 0) | (1 << 5));

  if (browseFile) {
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select srm file"));
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
  }
  else
    sprintf(filePath, "%s", fileName);

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);

      for (int c = 0; c < 512; c++) {
        // Output Data on PORTC
        PORTC = sdBuffer[c];

        // Arduino running at 16Mhz -> one nop = 62.5ns
        // Data setup time 50ns
        __asm__("nop\n\t");

        // Pull WE_SRAM (PH5) HIGH
        PORTH |= (1 << 5);

        // Set address
        PORTF = (currAddress + c) & 0xFF;
        PORTK = ((currAddress + c) >> 8) & 0xFF;

        // Leave WE_SRAM (PH5) HIGH for at least 85ns
        __asm__("nop\n\t""nop\n\t");

        // Pull WE_SRAM (PH5) LOW
        PORTH &= ~ (1 << 5);

        // Hold address for at least 25ns and wait 150ns before next write
        __asm__("nop\n\t""nop\n\t""nop\n\t");
      }
    }
    // Close the file:
    myFile.close();
    println_Msg(F("SRAM writing finished"));
    display_Update();

  }
  else {
    print_Error(F("File doesnt exist"), false);
  }
}

// Check if the SRAM was written without any error
unsigned long verifyFRAM_GBA(unsigned long framSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_SRAM(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Variable for errors
    writeErrors = 0;

    for (unsigned long currAddress = 0; currAddress < framSize; currAddress += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);

      for (int c = 0; c < 512; c++) {
        // Pull OE_SRAM(PH6) HIGH
        PORTH |= (1 << 6);

        // Set address
        PORTF = (currAddress + c) & 0xFF;
        PORTK = ((currAddress + c) >> 8) & 0xFF;

        // Arduino running at 16Mhz -> one nop = 62.5ns
        // Leave CS_SRAM HIGH for at least 85ns
        __asm__("nop\n\t""nop\n\t");

        // Pull OE_SRAM(PH6) LOW
        PORTH &= ~ (1 << 6);

        // Hold address for at least 25ns and wait 150ns before access
        __asm__("nop\n\t""nop\n\t""nop\n\t");

        // Read byte
        if (PINC != sdBuffer[c]) {
          writeErrors++;
        }
      }
    }

    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    print_Error(F("Can't open file"), false);
  }
}

/******************************************
  GBA FLASH SAVE Functions
*****************************************/
// SST 39VF512 Flashrom
void idFlash_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // ID command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x90);

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  // Wait 150ns before reading ID
  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t");

  // Read the two id bytes into a string
  sprintf(flashid, "%02X%02X", readByteFlash_GBA(0), readByteFlash_GBA(1));

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);
}

// Reset FLASH
void resetFLASH_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Reset command sequence
  writeByteFlash_GBA(0x5555, 0xAA);
  writeByteFlash_GBA(0x2AAA, 0x55);
  writeByteFlash_GBA(0x5555, 0xf0);
  writeByteFlash_GBA(0x5555, 0xf0);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Wait
  delay(100);
}

byte readByteFlash_GBA(unsigned long myAddress) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  // Wait until byte is ready to read
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read byte
  byte tempByte = PINC;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeByteFlash_GBA(unsigned long myAddress, byte myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE_FLASH(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 40ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WE_FLASH(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WE high for a bit
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

// Erase FLASH
void eraseFLASH_GBA() {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Erase command sequence
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x80);
  writeByteFlash_GBA(0x5555, 0xaa);
  writeByteFlash_GBA(0x2aaa, 0x55);
  writeByteFlash_GBA(0x5555, 0x10);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Wait until all is erased
  delay(500);
}

boolean blankcheckFLASH_GBA (unsigned long flashSize) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set address to 0
  PORTF = 0x00;
  PORTK = 0x00;

  // Set data pins to input
  DDRC = 0x00;
  // Disable Pullups
  //PORTC = 0x00;

  boolean blank = 1;

  // Output a LOW signal on  CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    // Fill buffer
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Check buffer
    for (unsigned long currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != 0xFF) {
        currByte = 512;
        currAddress = flashSize;
        blank = 0;
      }
    }
  }
  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  return blank;
}

// The MX29L010 is 131072 bytes in size and has 16 sectors per bank
// each sector is 4096 bytes, there are 32 sectors total
// therefore the bank size is 65536 bytes, so we have two banks in total
void switchBank_GBA(byte bankNum) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data pins to output
  DDRC = 0xFF;

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Switch bank command sequence
  writeByte_GBA(0x5555, 0xAA);
  writeByte_GBA(0x2AAA, 0x55);
  writeByte_GBA(0x5555, 0xB0);
  writeByte_GBA(0x0000, bankNum);

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);
}

void readFLASH_GBA (boolean browseFile, unsigned long flashSize, uint32_t pos) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set address to 0
  PORTF = 0x00;
  PORTK = 0x00;

  // Set data pins to input
  DDRC = 0x00;

  if (browseFile) {
    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".fla");

    // create a new folder for the save file
    EEPROM_readAnything(0, foldern);

    sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // Save location
    print_Msg(F("Saving to "));
    print_Msg(folder);
    println_Msg(F("/..."));
    display_Update();

    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(0, foldern);
  }

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    myFile.seekCur(pos);

  // Output a LOW signal on CE_FLASH(PH0)
  PORTH &= ~(1 << 0);

  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByteFlash_GBA(currAddress + c);
    }
    // Write sdBuffer to file
    myFile.write(sdBuffer, 512);
  }
  myFile.close();

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  // Signal end of process
  println_Msg(F("Done"));
  display_Update();
}

void busyCheck_GBA(int currByte) {
  // Set data pins to input
  DDRC = 0x00;
  // Output a LOW signal on OE_FLASH(PH6)
  PORTH &= ~(1 << 6);
  // Read PINC
  while (PINC != sdBuffer[currByte]) {}
  // Output a HIGH signal on OE_FLASH(PH6)
  PORTH |= (1 << 6);
  // Set data pins to output
  DDRC = 0xFF;
}

void writeFLASH_GBA (boolean browseFile, unsigned long flashSize, uint32_t pos, boolean isAtmel) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5) and OE_FLASH(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  // Set data port to output
  DDRC = 0xFF;

  if (browseFile) {
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select fla file"));
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);
    display_Clear();
  }

  print_Msg(F("Writing flash..."));
  display_Update();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Seek to a new position in the file
    if (pos != 0)
      myFile.seekCur(pos);

    // Output a LOW signal on CE_FLASH(PH0)
    PORTH &= ~(1 << 0);

    if (!isAtmel) {
      for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
        //fill sdBuffer
        myFile.read(sdBuffer, 512);

        for (int c = 0; c < 512; c++) {
          // Write command sequence
          writeByteFlash_GBA(0x5555, 0xaa);
          writeByteFlash_GBA(0x2aaa, 0x55);
          writeByteFlash_GBA(0x5555, 0xa0);
          // Write current byte
          writeByteFlash_GBA(currAddress + c, sdBuffer[c]);

          // Wait
          busyCheck_GBA(c);
        }
      }
    }
    else {
      for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 128) {
        //fill sdBuffer
        myFile.read(sdBuffer, 128);

        // Write command sequence
        writeByteFlash_GBA(0x5555, 0xaa);
        writeByteFlash_GBA(0x2aaa, 0x55);
        writeByteFlash_GBA(0x5555, 0xa0);
        for (int c = 0; c < 128; c++) {
          writeByteFlash_GBA(currAddress + c, sdBuffer[c]);
        }
        delay(15);
      }
    }
    // Set CS_FLASH(PH0) high
    PORTH |= (1 << 0);

    // Close the file:
    myFile.close();
    println_Msg(F("done"));
    display_Update();

  }
  else {
    println_Msg(F("Error"));
    print_Error(F("File doesnt exist"), false);
  }
}

// Check if the Flashrom was written without any error
void verifyFLASH_GBA(unsigned long flashSize, uint32_t pos) {
  // Output a HIGH signal on CS_ROM(PH3) WE_FLASH(PH5)
  PORTH |= (1 << 3) | (1 << 5);

  // Set address ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;

  // Set data pins to input
  DDRC = 0x00;

  // Output a LOW signal on CE_FLASH(PH0) and  OE_FLASH(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  // Signal beginning of process
  print_Msg(F("Verify..."));
  display_Update();

  unsigned long wrError = 0;

  //open file on sd card
  if (!myFile.open(filePath, O_READ)) {
    print_Error(F("SD Error"), true);
  }

  // Seek to a new position in the file
  if (pos != 0)
    myFile.seekCur(pos);

  for (unsigned long currAddress = 0; currAddress < flashSize; currAddress += 512) {
    myFile.read(sdBuffer, 512);

    for (int c = 0; c < 512; c++) {
      // Read byte
      if (sdBuffer[c] != readByteFlash_GBA(currAddress + c)) {
        wrError++;
      }
    }
  }
  myFile.close();

  // Set CS_FLASH(PH0) high
  PORTH |= (1 << 0);

  if (wrError == 0) {
    println_Msg(F("OK"));
  }
  else {
    print_Msg(wrError);
    print_Error(F(" Errors"), false);
  }
}

/******************************************
  GBA Eeprom SAVE Functions
*****************************************/
// Write eeprom from file
void writeEeprom_GBA(word eepSize) {
  // Launch Filebrowser
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select eep file"));
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();

  print_Msg(F("Writing EEPROM..."));
  display_Update();

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    for (word i = 0; i < eepSize * 16; i += 64) {
      // Fill romBuffer
      myFile.read(sdBuffer, 512);
      // Disable interrupts for more uniform clock pulses
      noInterrupts();
      // Write 512 bytes
      writeBlock_EEP(i, eepSize);
      interrupts();

      // Wait
      delayMicroseconds(200);
    }

    // Close the file:
    myFile.close();
    println_Msg(F("done"));
    display_Update();
  }
  else {
    println_Msg(F("Error"));
    print_Error(F("File doesnt exist"), false);
  }
}

// Read eeprom to file
void readEeprom_GBA(word eepSize) {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".eep");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);

  sprintf(folder, "GBA/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // Save location
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  // Each block contains 8 Bytes, so for a 8KB eeprom 1024 blocks need to be read
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) {
    // Disable interrupts for more uniform clock pulses
    noInterrupts();
    // Fill sd Buffer
    readBlock_EEP(currAddress, eepSize);
    interrupts();

    // Write sdBuffer to file
    myFile.write(sdBuffer, 512);

    // Wait
    delayMicroseconds(200);
  }
  myFile.close();
}

// Send address as bits to eeprom
void send_GBA(word currAddr, word numBits) {
  for (word addrBit = numBits; addrBit > 0; addrBit--) {
    // If you want the k-th bit of n, then do
    // (n & ( 1 << k )) >> k
    if (((currAddr & ( 1 << (addrBit - 1))) >> (addrBit - 1))) {
      // Set A0(PF0) to High
      PORTF |= (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~ (1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
    }
    else {
      // Set A0(PF0) to Low
      PORTF &= ~ (1 << 0);
      // Set WR(PH5) to LOW
      PORTH &= ~ (1 << 5);
      // Set WR(PH5) to High
      PORTH |= (1 << 5);
    }
  }
}

// Write 512K eeprom block
void writeBlock_EEP(word startAddr, word eepSize) {
  // Setup
  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to Output
  DDRF |= (1 << 0);
  // Set A23/D7(PC7) to Output
  DDRC |= (1 << 7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to High
  PORTF |= (1 << 0);
  // Set A23/D7(PC7) to High
  PORTC |= (1 << 7);

  __asm__("nop\n\t""nop\n\t");

  // Write 64*8=512 bytes
  for (word currAddr = startAddr; currAddr < startAddr + 64; currAddr++) {
    // Set CS_ROM(PH3) to LOW
    PORTH &= ~ (1 << 3);

    // Send write request "10"
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Send data
    for (byte currByte = 0; currByte < 8; currByte++) {
      send_GBA(sdBuffer[(currAddr - startAddr) * 8 + currByte], 8);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // WR(PH5) to High
    PORTH |=  (1 << 5);

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);

    // Wait until done
    // Set A0(PF0) to Input
    DDRF &= ~ (1 << 0);

    do {
      // Set  CS_ROM(PH3) RD(PH6) to LOW
      PORTH &= ~((1 << 3) | (1 << 6));
      // Set  CS_ROM(PH3) RD(PH6) to High
      PORTH  |= (1 << 3) | (1 << 6);
    }
    while ((PINF & 0x1) == 0);

    // Set A0(PF0) to Output
    DDRF |= (1 << 0);
  }
}

// Reads 512 bytes from eeprom
void readBlock_EEP(word startAddress, word eepSize) {
  // Setup
  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to Output
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to Output
  DDRF |= (1 << 0);
  // Set A23/D7(PC7) to Output
  DDRC |= (1 << 7);

  // Set CS_ROM(PH3) WR(PH5) RD(PH6) to High
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Set A0(PF0) to High
  PORTF |= (1 << 0);
  // Set A23/D7(PC7) to High
  PORTC |= (1 << 7);

  __asm__("nop\n\t""nop\n\t");

  // Read 64*8=512 bytes
  for (word currAddr = startAddress; currAddr < startAddress + 64; currAddr++) {
    // Set CS_ROM(PH3) to LOW
    PORTH &= ~ (1 << 3);

    // Send read request "11"
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // Set WR(PH5) to High
    PORTH |= (1 << 5);

    // Send either 6 or 14 bit address
    if (eepSize == 4) {
      send_GBA(currAddr, 6);
    }
    else {
      send_GBA(currAddr, 14);
    }

    // Send stop bit
    // Set A0(PF0) to LOW
    PORTF &= ~ (1 << 0);
    // Set WR(PH5) to LOW
    PORTH &= ~ (1 << 5);
    // WR(PH5) to High
    PORTH |=  (1 << 5);

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);

    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read data
    // Set A0(PF0) to Input
    DDRF &= ~ (1 << 0);
    // Set CS_ROM(PH3) to low
    PORTH &= ~(1 << 3);

    // Array that holds the bits
    bool tempBits[65];

    // Ignore the first 4 bits
    for (byte i = 0; i < 4; i++) {
      // Set RD(PH6) to LOW
      PORTH &= ~ (1 << 6);
      // Set RD(PH6) to High
      PORTH  |= (1 << 6);
    }

    // Read the remaining 64bits into array
    for (byte currBit = 0; currBit < 64; currBit++) {
      // Set RD(PH6) to LOW
      PORTH &= ~ (1 << 6);
      // Set RD(PH6) to High
      PORTH  |= (1 << 6);

      // Read bit from A0(PF0)
      tempBits[currBit] = (PINF & 0x1);
    }

    // Set CS_ROM(PH3) to High
    PORTH |= (1 << 3);
    // Set A0(PF0) to High
    PORTF |= (1 << 0);
    // Set A0(PF0) to Output
    DDRF |= (1 << 0);

    // OR 8 bits into one byte for a total of 8 bytes
    for (byte j = 0; j < 64; j += 8) {
      sdBuffer[((currAddr - startAddress) * 8) + (j / 8)] = tempBits[0 + j] << 7 | tempBits[1 + j] << 6 | tempBits[2 + j] << 5 | tempBits[3 + j] << 4 | tempBits[4 + j] << 3 | tempBits[5 + j] << 2 | tempBits[6 + j] << 1 | tempBits[7 + j];
    }
  }
}

// Check if the SRAM was written without any error
unsigned long verifyEEP_GBA(word eepSize) {
  unsigned long wrError = 0;

  //open file on sd card
  if (!myFile.open(filePath, O_READ)) {
    print_Error(F("SD Error"), true);
  }

  // Fill sd Buffer
  for (word currAddress = 0; currAddress < eepSize * 16; currAddress += 64) {
    // Disable interrupts for more uniform clock pulses
    noInterrupts();
    readBlock_EEP(currAddress, eepSize);
    interrupts();

    // Compare
    for (int currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != myFile.read()) {
        wrError++;
      }
    }
  }
  myFile.close();
  return wrError;
}

/******************************************
  GBA REPRO Functions (32MB Intel 4000L0YBQ0 and 16MB MX29GL128E)
*****************************************/
// Reset to read mode
void resetIntel_GBA(unsigned long partitionSize) {
  for (unsigned long currPartition = 0; currPartition < cartSize; currPartition += partitionSize) {
    writeWord_GBA(currPartition, 0xFFFF);
  }
}

void resetMX29GL128E_GBA() {
  writeWord_GAB(0, 0xF0);
}

boolean sectorCheckMX29GL128E_GBA() {
  boolean sectorProtect = 0;
  writeWord_GAB(0xAAA, 0xAA);
  writeWord_GAB(0x555, 0x55);
  writeWord_GAB(0xAAA, 0x90);
  for (unsigned long currSector = 0x0; currSector < 0xFFFFFF; currSector += 0x20000) {
    if (readWord_GAB(currSector + 0x04) != 0x0)
      sectorProtect = 1;
  }
  resetMX29GL128E_GBA();
  return sectorProtect;
}

void idFlashrom_GBA() {
  // Send Intel ID command to flashrom
  writeWord_GBA(0, 0x90);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read flashrom ID
  sprintf(flashid, "%02X%02X", ((readWord_GBA(0x2) >> 8) & 0xFF), (readWord_GBA(0x4) & 0xFF));

  // Intel Strataflash
  if (strcmp(flashid, "8802") == 0 || (strcmp(flashid, "8816") == 0)) {
    cartSize = 0x2000000;
  }
  else {
    // Send swapped MX29GL128E/MSP55LV128 ID command to flashrom
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x90);
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    // Read flashrom ID
    sprintf(flashid, "%02X%02X", ((readWord_GAB(0x2) >> 8) & 0xFF), (readWord_GAB(0x2) & 0xFF));

    // MX29GL128E or MSP55LV128
    if (strcmp(flashid, "227E") == 0) {
      // MX is 0xC2 and MSP is 0x4 or 0x1
      romType = (readWord_GAB(0x0) & 0xFF);
      cartSize = 0x1000000;
      resetMX29GL128E_GBA();
    }
    else {
      println_Msg(F("Error"));
      println_Msg(F(""));
      println_Msg(F("Unknown Flash"));
      print_Msg(F("Flash ID: "));
      println_Msg(flashid);
      println_Msg(F(""));
      print_Error(F("Check voltage"), true);
    }
  }
}

boolean blankcheckFlashrom_GBA() {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) {
    // Blink led
    blinkLED();

    for (unsigned long currByte = 0; currByte < 0x20000; currByte += 2) {
      if (readWord_GBA(currSector + currByte) != 0xFFFF) {
        return 0;
      }
    }
  }
  return 1;
}

void eraseIntel4000_GBA() {
  // If the game is smaller than 16Mbit only erase the needed blocks
  unsigned long lastBlock = 0xFFFFFF;
  if (fileSize < 0xFFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
  }

  // Erase 126 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    blinkLED();
  }

  // Erase the second chip
  if (fileSize > 0xFFFFFF) {
    // 126 blocks with 64kwords each
    for (unsigned long currBlock = 0x1000000; currBlock < 0x1FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      blinkLED();
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x1FE0000; currBlock < 0x1FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      blinkLED();
    }
  }
}

void eraseIntel4400_GBA() {
  // If the game is smaller than 32Mbit only erase the needed blocks
  unsigned long lastBlock = 0x1FFFFFF;
  if (fileSize < 0x1FFFFFF)
    lastBlock = fileSize;

  // Erase 4 blocks with 16kwords each
  for (unsigned long currBlock = 0x0; currBlock < 0x1FFFF; currBlock += 0x8000) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
  }

  // Erase 255 blocks with 64kwords each
  for (unsigned long currBlock = 0x20000; currBlock < lastBlock; currBlock += 0x1FFFF) {
    // Unlock Block
    writeWord_GBA(currBlock, 0x60);
    writeWord_GBA(currBlock, 0xD0);

    // Erase Command
    writeWord_GBA(currBlock, 0x20);
    writeWord_GBA(currBlock, 0xD0);

    // Read the status register
    word statusReg = readWord_GBA(currBlock);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GBA(currBlock);
    }
    // Blink led
    blinkLED();
  }

  /* No need to erase the second chip as max rom size is 32MB
    if (fileSize > 0x2000000) {
    // 255 blocks with 64kwords each
    for (unsigned long currBlock = 0x2000000; currBlock < 0x3FDFFFF; currBlock += 0x1FFFF) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      blinkLED();
    }

    // 4 blocks with 16kword each
    for (unsigned long currBlock = 0x3FE0000; currBlock < 0x3FFFFFF; currBlock += 0x8000) {
      // Unlock Block
      writeWord_GBA(currBlock, 0x60);
      writeWord_GBA(currBlock, 0xD0);

      // Erase Command
      writeWord_GBA(currBlock, 0x20);
      writeWord_GBA(currBlock, 0xD0);

      // Read the status register
      word statusReg = readWord_GBA(currBlock);
      while ((statusReg | 0xFF7F) != 0xFFFF) {
        statusReg = readWord_GBA(currBlock);
      }
      // Blink led
      blinkLED();
    }
    }*/
}

void sectorEraseMSP55LV128_GBA() {
  unsigned long lastSector = 0xFFFFFF;

  // Erase 256 sectors with 64kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x10000) {
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x80);
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(currSector, 0x30);

    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GAB(currSector);
    }
    // Blink LED
    blinkLED();
  }
}

void sectorEraseMX29GL128E_GBA() {
  unsigned long lastSector = 0xFFFFFF;

  // Erase 128 sectors with 128kbytes each
  unsigned long currSector;
  for (currSector = 0x0; currSector < lastSector; currSector += 0x20000) {
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(0xAAA, 0x80);
    writeWord_GAB(0xAAA, 0xAA);
    writeWord_GAB(0x555, 0x55);
    writeWord_GAB(currSector, 0x30);

    // Read the status register
    word statusReg = readWord_GAB(currSector);
    while ((statusReg | 0xFF7F) != 0xFFFF) {
      statusReg = readWord_GAB(currSector);
    }
    // Blink LED
    blinkLED();
  }
}

void writeIntel4000_GBA() {
  for (unsigned long currBlock = 0; currBlock < fileSize; currBlock += 0x20000) {
    // Blink led
    blinkLED();

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Unlock Block
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x60);
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xD0);

        // Buffered program command
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0xE8);

        // Check Status register
        word statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer);
        }

        // Write word count (minus 1)
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer, 0x1F);

        // Write buffer
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          word currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Write Buffer to Flash
        writeWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62, 0xD0);

        // Read the status register at last written address
        statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        while ((statusReg | 0xFF7F) != 0xFFFF) {
          statusReg = readWord_GBA(currBlock + currSdBuffer + currWriteBuffer + 62);
        }
      }
    }
  }
}

void writeMSP55LV128_GBA() {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x10000) {
    // Blink led
    blinkLED();

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x10000; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 16 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 32) {
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        writeWord_GAB(0x555, 0x55);
        writeWord_GAB(currSector, 0x25);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0xF);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 32; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 30);
        }
      }
    }
  }
}

void writeMX29GL128E_GBA() {
  for (unsigned long currSector = 0; currSector < fileSize; currSector += 0x20000) {
    // Blink led
    blinkLED();

    // Write to flashrom
    for (unsigned long currSdBuffer = 0; currSdBuffer < 0x20000; currSdBuffer += 512) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);

      // Write 32 words at a time
      for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += 64) {
        // Write Buffer command
        writeWord_GAB(0xAAA, 0xAA);
        writeWord_GAB(0x555, 0x55);
        writeWord_GAB(currSector, 0x25);

        // Write word count (minus 1)
        writeWord_GAB(currSector, 0x1F);

        // Write buffer
        word currWord;
        for (byte currByte = 0; currByte < 64; currByte += 2) {
          // Join two bytes into one word
          currWord = ( ( sdBuffer[currWriteBuffer + currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currWriteBuffer + currByte] & 0xFF );
          writeWord_GBA(currSector + currSdBuffer + currWriteBuffer + currByte, currWord);
        }

        // Confirm write buffer
        writeWord_GAB(currSector, 0x29);

        // Read the status register
        word statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);

        while ((statusReg | 0xFF7F) != (currWord | 0xFF7F)) {
          statusReg = readWord_GAB(currSector + currSdBuffer + currWriteBuffer + 62);
        }
      }
    }
  }
}

boolean verifyFlashrom_GBA() {
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    writeErrors = 0;

    for (unsigned long currSector = 0; currSector < fileSize; currSector += 131072) {
      // Blink led
      blinkLED();
      for (unsigned long currSdBuffer = 0; currSdBuffer < 131072; currSdBuffer += 512) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);

        for (int currByte = 0; currByte < 512; currByte += 2) {
          // Join two bytes into one word
          word currWord = ( ( sdBuffer[currByte + 1] & 0xFF ) << 8 ) | ( sdBuffer[currByte] & 0xFF );

          // Compare both
          if (readWord_GBA(currSector + currSdBuffer + currByte) != currWord) {
            writeErrors++;
            myFile.close();
            return 0;
          }
        }
      }
    }
    // Close the file:
    myFile.close();
    if (writeErrors == 0) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    print_Error(F("Can't open file"), true);
    return 9999;
  }
}

void flashRepro_GBA() {
  // Check flashrom ID's
  idFlashrom_GBA();

  if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0) || (strcmp(flashid, "227E") == 0)) {
    print_Msg(F("ID: "));
    print_Msg(flashid);
    print_Msg(F(" Size: "));
    print_Msg(cartSize / 0x100000);
    println_Msg(F("MB"));
    // MX29GL128E or MSP55LV128(N)
    if (strcmp(flashid, "227E") == 0) {
      // MX is 0xC2 and MSP55LV128 is 0x4 and MSP55LV128N 0x1
      if (romType == 0xC2) {
        println_Msg(F("Macronix MX29GL128E"));
      }
      else if ((romType == 0x1) || (romType == 0x4)) {
        println_Msg(F("Fujitsu MSP55LV128N"));
      }
      else if ((romType == 0x89)) {
        println_Msg(F("Intel PC28F256M29"));
      }
      else if ((romType == 0x20)) {
        println_Msg(F("ST M29W128GH"));
      }
      else {
        print_Msg(F("romType: 0x"));
        println_Msg(romType, HEX);
        print_Error(F("Unknown manufacturer"), true);
      }
    }
    // Intel 4000L0YBQ0
    else if (strcmp(flashid, "8802") == 0) {
      println_Msg(F("Intel 4000L0YBQ0"));
    }
    // Intel 4400L0ZDQ0
    else if (strcmp(flashid, "8816") == 0) {
      println_Msg(F("Intel 4400L0ZDQ0"));
    }
    println_Msg("");
    println_Msg(F("This will erase your"));
    println_Msg(F("Repro Cartridge."));
    println_Msg(F(""));
    println_Msg("");
    println_Msg(F("Press Button"));
    display_Update();
    wait();

    // Launch file browser
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser(F("Select gba file"));
    display_Clear();
    display_Update();

    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);

    // Open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // Get rom size from file
      fileSize = myFile.fileSize();
      print_Msg(F("File size: "));
      print_Msg(fileSize / 0x100000);
      println_Msg(F("MB"));
      display_Update();

      // Erase needed sectors
      if (strcmp(flashid, "8802") == 0) {
        println_Msg(F("Erasing..."));
        display_Update();
        eraseIntel4000_GBA();
        resetIntel_GBA(0x200000);
      }
      else if (strcmp(flashid, "8816") == 0) {
        println_Msg(F("Erasing..."));
        display_Update();
        eraseIntel4400_GBA();
        resetIntel_GBA(0x200000);
      }
      else if (strcmp(flashid, "227E") == 0) {
        //if (sectorCheckMX29GL128E_GBA()) {
        //print_Error(F("Sector Protected"), true);
        //}
        //else {
        println_Msg(F("Erasing..."));
        display_Update();
        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
          //MX29GL128E
          //PC28F256M29 (0x89)
          sectorEraseMX29GL128E_GBA();
        }
        else if ((romType == 0x1) || (romType == 0x4)) {
          //MSP55LV128(N)
          sectorEraseMSP55LV128_GBA();
        }
        //}
      }
      /* Skip blankcheck to save time
        print_Msg(F("Blankcheck..."));
        display_Update();
        if (blankcheckFlashrom_GBA()) {
        println_Msg(F("OK"));
      */

      //Write flashrom
      print_Msg(F("Writing "));
      println_Msg(filePath);
      display_Update();
      if ((strcmp(flashid, "8802") == 0) || (strcmp(flashid, "8816") == 0)) {
        writeIntel4000_GBA();
      }
      else if (strcmp(flashid, "227E") == 0) {
        if ((romType == 0xC2) || (romType == 0x89) || (romType == 0x20)) {
          //MX29GL128E (0xC2)
          //PC28F256M29 (0x89)
          writeMX29GL128E_GBA();
        }
        else if ((romType == 0x1) || (romType == 0x4)) {
          //MSP55LV128(N)
          writeMSP55LV128_GBA();
        }
      }

      // Close the file:
      myFile.close();

      // Verify
      print_Msg(F("Verifying..."));
      display_Update();
      if (strcmp(flashid, "8802") == 0) {
        // Don't know the correct size so just take some guesses
        resetIntel_GBA(0x8000);
        delay(1000);
        resetIntel_GBA(0x100000);
        delay(1000);
        resetIntel_GBA(0x200000);
        delay(1000);
      }
      else if (strcmp(flashid, "8816") == 0) {
        resetIntel_GBA(0x200000);
        delay(1000);
      }

      else if (strcmp(flashid, "227E") == 0) {
        resetMX29GL128E_GBA();
        delay(1000);
      }
      if (verifyFlashrom_GBA() == 1) {
        println_Msg(F("OK"));
        display_Update();
      }
      else {
        print_Error(F("ERROR"), true);
      }
      /* Skipped blankcheck
        }
        else {
        print_Error(F("failed"), true);
        }
      */
    }
    else {
      print_Error(F("Can't open file"), true);
    }
  }
  else {
    println_Msg(F("Error"));
    println_Msg(F(""));
    println_Msg(F("Unknown Flash"));
    print_Msg(F("Flash ID: "));
    println_Msg(flashid);
    println_Msg(F(""));
    print_Error(F("Check voltage"), true);
  }
}

#endif

//******************************************
// End of File
//******************************************
