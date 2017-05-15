//******************************************
// GAME BOY ADVANCE
//******************************************

/******************************************
   Variables
 *****************************************/
char calcChecksumStr[5];
boolean readType;

static const int nintendoLogo[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
  0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19,
  0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33,
  0x82, 0xE3, 0xCE, 0xBF, 0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
  0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27,
  0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00,
  0x40, 0xA7, 0x0E, 0xFD, 0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
  0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF,
  0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63,
  0x87, 0xF0, 0x3C, 0xAF, 0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07
};

/******************************************
   Menu
 *****************************************/
// GBA menu items
static const char GBAMenuItem1[] PROGMEM = "Read Rom";
static const char GBAMenuItem2[] PROGMEM = "Read Save";
static const char GBAMenuItem3[] PROGMEM = "Write Save";
static const char GBAMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsGBA[] PROGMEM = {GBAMenuItem1, GBAMenuItem2, GBAMenuItem3, GBAMenuItem4};

// Rom menu
static const char GBARomItem1[] PROGMEM = "1MB";
static const char GBARomItem2[] PROGMEM = "2MB";
static const char GBARomItem3[] PROGMEM = "4MB";
static const char GBARomItem4[] PROGMEM = "8MB";
static const char GBARomItem5[] PROGMEM = "16MB";
static const char GBARomItem6[] PROGMEM = "32MB";
static const char* const romOptionsGBA[] PROGMEM = {GBARomItem1, GBARomItem2, GBARomItem3, GBARomItem4, GBARomItem5, GBARomItem6};

// Save menu
static const char GBASaveItem1[] PROGMEM = "4K EEPROM";
static const char GBASaveItem2[] PROGMEM = "64K EEPROM";
static const char GBASaveItem3[] PROGMEM = "256K SRAM/FRAM";
static const char GBASaveItem4[] PROGMEM = "512K SRAM/FRAM";
static const char GBASaveItem5[] PROGMEM = "512K FLASHROM";
static const char GBASaveItem6[] PROGMEM = "1M FLASHROM";
static const char* const saveOptionsGBA[] PROGMEM = {GBASaveItem1, GBASaveItem2, GBASaveItem3, GBASaveItem4, GBASaveItem5, GBASaveItem6};

void gbaMenu() {
  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBA, 4);
  mainMenu = question_box("GBA Cart Reader", menuOptions, 4, 0);

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
          GBARomMenu = question_box("Select ROM size", menuOptions, 6, 0);

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
      compare_checksum_GBA();
      break;

    case 1:
      // Read save
      if (saveType == 0) {
        // create submenu with title and 6 options to choose from
        unsigned char GBASaveMenu;
        // Copy menuOptions out of progmem
        convertPgm(saveOptionsGBA, 6);
        GBASaveMenu = question_box("Select save type", menuOptions, 6, 0);

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
          readFRAM_GBA(32768);
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
          // Change working dir to root
          writeFRAM_GBA(1, 65536);
          writeErrors = verifyFRAM_GBA(65536);
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
      break;

    case 2:
      // Write save
      if (saveType == 0) {
        // create submenu with title and 6 options to choose from
        unsigned char GBASavesMenu;
        // Copy menuOptions out of progmem
        convertPgm(saveOptionsGBA, 6);
        GBASavesMenu = question_box("Select save type", menuOptions, 6, 0);
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
            // 1024K FLASH
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
          sd.chdir("/");
          // 256K SRAM/FRAM
          // Change working dir to root
          writeFRAM_GBA(1, 32768);
          writeErrors = verifyFRAM_GBA(32768);
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

        case 6:
          display_Clear();
          sd.chdir("/");
          // 512K SRAM/FRAM
          // Change working dir to root
          writeFRAM_GBA(1, 65536);
          writeErrors = verifyFRAM_GBA(65536);
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
          if (strcmp(flashid, "BFD4") != 0) {
            println_Msg(F("Flashrom Type not supported"));
            print_Msg(F("ID: "));
            println_Msg(flashid);
            print_Error(F(""), true);
          }
          eraseFLASH_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(1, 65536, 0);
            verifyFLASH_GBA(65536, 0);
          }
          else {
            print_Error(F("Erase failed"), false);
          }
          setROM_GBA();
          break;

        case 5:
          display_Clear();
          sd.chdir("/");
          // 1M FLASH
          idFlash_GBA();
          resetFLASH_GBA();
          if (strcmp(flashid, "C209") != 0) {
            println_Msg(F("Flashrom Type not supported"));
            print_Msg(F("ID: "));
            println_Msg(flashid);
            print_Error(F(""), true);
          }
          eraseFLASH_GBA();
          // 131072 bytes are divided into two 65536 byte banks
          switchBank_GBA(0x0);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(1, 65536, 0);
            verifyFLASH_GBA(65536, 0);
          }
          else {
            print_Error(F("Erase failed"), false);
          }
          switchBank_GBA(0x1);
          setROM_GBA();
          if (blankcheckFLASH_GBA(65536)) {
            writeFLASH_GBA(0, 65536, 65536);
            verifyFLASH_GBA(65536, 65536);
          }
          else {
            print_Error(F("Erase failed"), false);
          }
          setROM_GBA();
          break;
      }
      break;

    case 3:
      asm volatile ("  jmp 0");
      break;
  }
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_GBA() {
  setROM_GBA();

  // Print start page
  getCartInfo_GBA();
  display_Clear();

  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Cart ID: "));
  println_Msg(cartID);
  print_Msg(F("Rom Size: "));
  if (cartSize == 0)
    println_Msg(F("Unknown"));
  else {
    print_Msg(cartSize);
    println_Msg(F("MB"));
  }
  print_Msg(F("Save: "));
  switch (saveType)
  {
    case 0:
      println_Msg(F("Unknown"));
      break;

    case 1:
      println_Msg(F("4K Eeprom"));
      break;

    case 2:
      println_Msg(F("64K Eeprom"));
      break;

    case 3:
      println_Msg(F("256K Sram"));
      break;

    case 4:
      println_Msg(F("512K Flash"));
      break;

    case 5:
      println_Msg(F("1024K Flash"));
      break;
  }

  print_Msg(F("Checksum: "));
  println_Msg(checksumStr);
  print_Msg(F("Version: 1."));
  println_Msg(romVersion);

  // Wait for user input
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
  Low level functions
*****************************************/
// Setup all ports and pins for reading the rom
void setROM_GBA() {
  // Set address/data pins to OUTPUT
  // AD0-AD7
  DDRF = 0xFF;
  // AD8-AD15
  DDRK = 0xFF;
  // AD16-AD23
  DDRC = 0xFF;

  // Output a HIGH signal
  // AD0-AD7
  PORTF = 0xFF;
  // AD8-AD15
  PORTK = 0xFF;
  // AD16-AD23
  PORTC = 0xFF;

  // Set Control Pins to Output CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // CLK is N/C and IRQ is conected to GND inside the cartridge
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // At power-on all the control lines are high/disabled
  PORTH |= (1 << 0)  | (1 << 3) | (1 << 5) | (1 << 6);

  // Wait until all is stable
  delay(600);
}

void setAddress_GBA(unsigned long myAddress) {
  // Switch CS_ROM(PH3) to HIGH
  PORTH |= (1 << 3);

  // Switch RD(PH6) to HIGH
  PORTH |=  (1 << 6);

  // Set address/data ports to output
  DDRF = 0xFF;
  DDRK = 0xFF;
  DDRC = 0xFF;

  // Output address to address pins,
  PORTF = (myAddress / 2) & 0xFF;
  PORTK = ((myAddress / 2) >> 8) & 0xFF;
  PORTC = ((myAddress / 2) >> 16) & 0xFF;

  // Pull CS(PH3) to LOW
  PORTH &= ~ (1 << 3);

  // Output a high signal
  PORTF = 0XFF;
  PORTK = 0XFF;
  PORTC = 0XFF;

  // Set address/data ports to input
  DDRF = 0x00;
  DDRK = 0x00;

  // Pull RD(PH6) to LOW
  PORTH &= ~ (1 << 6);
}

// Read multiple bytes into an array toggle both CS and RD each time
void readRand_GBA(unsigned long myAddress, byte myArray[] , int numBytes) {
  for (int currByte = 0; currByte < numBytes; currByte += 2) {
    setAddress_GBA(myAddress + currByte);

    word currWord = ((PINF << 8) + PINK) & 0xFFFF;

    myArray[currByte] = (currWord >> 8) & 0xFF;
    myArray[currByte + 1] = currWord & 0xFF;
  }

  // setROM_GBA without delay
  // Set address/data pins to OUTPUT
  // AD0-AD7
  DDRF = 0xFF;
  // AD8-AD15
  DDRK = 0xFF;
  // AD16-AD23
  DDRC = 0xFF;

  // Output a HIGH signal
  // AD0-AD7
  PORTF = 0xFF;
  // AD8-AD15
  PORTK = 0xFF;
  // AD16-AD23
  PORTC = 0xFF;

  // Set Control Pins to Output CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // CLK is N/C and IRQ is conected to GND inside the cartridge
  DDRH |= (1 << 0) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on CS_SRAM(PH0) CS_ROM(PH3) WR(PH5) RD(PH6)
  // At power-on all the control lines are high/disabled
  PORTH |= (1 << 0)  | (1 << 3) | (1 << 5) | (1 << 6);
}

// Read multiple bytes into an array but only toggle CS once
void readSeq_GBA(byte myArray[] , int numBytes) {
  for (int currByte = 0; currByte < numBytes; currByte += 2) {
    word currWord = ((PINF << 8) + PINK) & 0xFFFF;

    myArray[currByte] = (currWord >> 8) & 0xFF;
    myArray[currByte + 1] = currWord & 0xFF;

    // Switch RD(PH6) to HIGH
    PORTH |=  (1 << 6);

    // Pull RD(PH6) to LOW
    PORTH &= ~ (1 << 6);
  }
}

/******************************************
  Game Boy ROM Functions
*****************************************/
// Test known Nintendo header for errors and sets read method to either sequential or random access
int testHeader() {
  // Set address to start of rom
  setAddress_GBA(0);
  // Read header into array sequentially
  readSeq_GBA(sdBuffer, 192);
  // Reset ports or the 1st maskrom byte on eeprom carts won't be read correctly
  setROM_GBA();

  // Check if Nintendo logo is read ok
  int logoErrors = checkLogo();

  if (logoErrors != 0) {
    // Nintendo logo has errors -> change read method
    setROM_GBA();
    setAddress_GBA(0);

    // Read Header into array in random access mode
    readRand_GBA(0, sdBuffer, 192);

    logoErrors = checkLogo();
    if (logoErrors == 0) {
      readType = 0;
    }
  }
  else {
    readType = 1;
  }
  return logoErrors;
}

// Compare Nintendo logo, 156 bytes starting at 0x04
int checkLogo() {
  int errors = 0;
  for (int currByte = 0x4; currByte < 0xA0; currByte++) {
    if (pgm_read_byte(&nintendoLogo[currByte]) != sdBuffer[currByte]) {
      errors++;
    }
  }
  return errors;
}

// Read info out of rom header
void getCartInfo_GBA() {
  // Test rom header for errors
  int logoErrors =  testHeader();

  if (logoErrors != 0) {
    print_Error(F("Nintendo Logo Error"), true);
  }
  else {
    char tempStr2[2];
    char tempStr[5];
    char sizeStr[3];
    char saveStr[2];

    // cart not in list
    cartSize = 0;
    saveType = 0;

    // Get cart ID
    cartID[0] = char(sdBuffer[0xAC]);
    cartID[1] = char(sdBuffer[0xAD]);
    cartID[2] = char(sdBuffer[0xAE]);
    cartID[3] = char(sdBuffer[0xAF]);

    if (myFile.open("gba.txt", O_READ)) {
      // Loop through file
      while (myFile.available()) {
        // Read 4 bytes into String, do it one at a time so byte order doesn't get mixed up
        sprintf(tempStr, "%c", myFile.read());
        for (byte i = 0; i < 3; i++) {
          sprintf(tempStr2, "%c", myFile.read());
          strcat(tempStr, tempStr2);
        }

        // Check if string is a match
        if (strcmp(tempStr, cartID) == 0) {
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

          // Read the next ascii character and subtract 48 to convert to decimal
          saveType = myFile.read() - 48;
        }
        // If no match, empty string, advance by 7 and try again
        else {
          myFile.seekSet(myFile.curPosition() + 7);
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
      if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
        romName[myLength] = char(myByte);
        myLength++;
      }
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
      print_Msg(F("Result: "));
      println_Msg(calcChecksumStr);
      print_Error(F("Checksum Error"), false);
      println_Msg(F(""));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
    }
  }
}

// Dump ROM
void readROM_GBA() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // create a new folder for the rom file
  EEPROM_readAnything(10, foldern);
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
  EEPROM_writeAnything(10, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  // Set starting address
  setAddress_GBA(0);

  if (readType == 0) {
    setROM_GBA();
  }

  // Read rom
  for (int myAddress = 0; myAddress < cartSize; myAddress += 512) {
    // Read either sequentially or in random acces mode
    if (readType == 1)
      readSeq_GBA(sdBuffer, 512);
    else
      readRand_GBA(myAddress, sdBuffer, 512);

    // Write to SD
    myFile.write(sdBuffer, 512);
  }

  // Close the file:
  myFile.close();
}

// Calculate the checksum of the dumped rom
boolean compare_checksum_GBA () {
  println_Msg(F("Calculating Checksum"));
  display_Update();

  strcpy(fileName, romName);
  strcat(fileName, ".gba");

  // last used rom folder
  EEPROM_readAnything(10, foldern);
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

    if (strcmp(calcChecksumStr, checksumStr) == 0) {
      println_Msg(F("Checksum matches"));
      display_Update();
      return 1;
    }
    else {
      print_Msg(F("Result: "));
      println_Msg(calcChecksumStr);
      print_Error(F("Checksum Error"), false);
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
  // Disable Pullups
  //PORTC = 0x00;

  // Output a LOW signal on  CE_SRAM(PH0) and OE_SRAM(PH6)
  PORTH &= ~((1 << 0) | (1 << 6));

  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".srm");

  // create a new folder for the save file
  EEPROM_readAnything(10, foldern);
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
  EEPROM_writeAnything(10, foldern);

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
  // Output a high signal
  //PORTC = 0xFF;

  // Output a LOW signal on CE_SRAM(PH0) and WE_SRAM(PH5)
  PORTH &= ~((1 << 0) | (1 << 5));

  if (browseFile) {
    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser("Select srm file");
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
  // Disable Pullups
  //PORTC = 0x00;

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
  writeByteFlash_GBA(0x5555, 0xAA);
  writeByteFlash_GBA(0x2AAA, 0x55);
  writeByteFlash_GBA(0x5555, 0xB0);
  writeByteFlash_GBA(0x0000, bankNum);

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
  // Disable Pullups
  //PORTC = 0x00;

  if (browseFile) {
    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".fla");

    // create a new folder for the save file
    EEPROM_readAnything(10, foldern);

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
    EEPROM_writeAnything(10, foldern);
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

void writeFLASH_GBA (boolean browseFile, unsigned long flashSize, uint32_t pos) {
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
    fileBrowser("Select fla file");
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
  fileBrowser("Select eep file");
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();

  print_Msg(F("Writing eeprom..."));
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
  EEPROM_readAnything(10, foldern);

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
  EEPROM_writeAnything(10, foldern);

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

//******************************************
// End of File
//******************************************
