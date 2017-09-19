//******************************************
// GAME BOY
//******************************************

/******************************************
   Variables
 *****************************************/
// Game Boy
int sramBanks;
int romBanks;
uint16_t sramEndAddress = 0;

/******************************************
   Menu
 *****************************************/
// GB menu items
static const char GBMenuItem1[] PROGMEM = "Read Rom";
static const char GBMenuItem2[] PROGMEM = "Read Save";
static const char GBMenuItem3[] PROGMEM = "Write Save";
static const char GBMenuItem4[] PROGMEM = "Write Flashcart";
static const char GBMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsGB[] PROGMEM = {GBMenuItem1, GBMenuItem2, GBMenuItem3, GBMenuItem4, GBMenuItem5};

void gbMenu() {
  // create menu with title and 5 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGB, 5);
  mainMenu = question_box("GB Cart Reader", menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_GB();
      compare_checksum_GB();
      break;

    case 1:
      display_Clear();
      // Does cartridge have SRAM
      if (sramEndAddress > 0) {
        // Change working dir to root
        sd.chdir("/");
        readSRAM_GB();
      }
      else {
        print_Error(F("Cart has no Sram"), false);
      }
      break;

    case 2:
      display_Clear();
      // Does cartridge have SRAM
      if (sramEndAddress > 0) {
        // Change working dir to root
        sd.chdir("/");
        writeSRAM_GB();
        unsigned long wrErrors;
        wrErrors = verifySRAM_GB();
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
      }
      else {
        print_Error(F("Cart has no Sram"), false);
      }
      break;

    case 3:
      // Change working dir to root
      sd.chdir("/");
      writeFlash_GB();
      // Reset
      wait();
      asm volatile ("  jmp 0");
      break;

    case 4:
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
void setup_GB() {
  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Disable Internal Pullups
  //PORTC = 0x00;

  delay(400);

  // Print start page
  getCartInfo_GB();
  display_Clear();
  if (strcmp(checksumStr, "00") != 0) {
    println_Msg(F("GB Cart Info"));
    print_Msg(F("Name: "));
    println_Msg(romName);
    print_Msg(F("Rom Type: "));
    switch (romType) {
      case 0: print_Msg(F("ROM ONLY")); break;
      case 1: print_Msg(F("MBC1")); break;
      case 2: print_Msg(F("MBC1+RAM")); break;
      case 3: print_Msg(F("MBC1+RAM")); break;
      case 5: print_Msg(F("MBC2")); break;
      case 6: print_Msg(F("MBC2")); break;
      case 8: print_Msg(F("ROM+RAM")); break;
      case 9: print_Msg(F("ROM ONLY")); break;
      case 11: print_Msg(F("MMM01")); break;
      case 12: print_Msg(F("MMM01+RAM")); break;
      case 13: print_Msg(F("MMM01+RAM")); break;
      case 15: print_Msg(F("MBC3+TIMER")); break;
      case 16: print_Msg(F("MBC3+TIMER+RAM")); break;
      case 17: print_Msg(F("MBC3")); break;
      case 18: print_Msg(F("MBC3+RAM")); break;
      case 19: print_Msg(F("MBC3+RAM")); break;
      case 21: print_Msg(F("MBC4")); break;
      case 22: print_Msg(F("MBC4+RAM")); break;
      case 23: print_Msg(F("MBC4+RAM")); break;
      case 25: print_Msg(F("MBC5")); break;
      case 26: print_Msg(F("MBC5+RAM")); break;
      case 27: print_Msg(F("MBC5+RAM")); break;
      case 28: print_Msg(F("MBC5+RUMBLE")); break;
      case 29: print_Msg(F("MBC5+RUMBLE+RAM")); break;
      case 30: print_Msg(F("MBC5+RUMBLE+RAM")); break;
      case 252: print_Msg(F("Gameboy Camera")); break;
      default: print_Msg(F("Not found"));
    }
    println_Msg(F(" "));
    print_Msg(F("Rom Size: "));
    switch (romSize) {
      case 0: print_Msg(F("32KB")); break;
      case 1: print_Msg(F("64KB")); break;
      case 2: print_Msg(F("128KB")); break;
      case 3: print_Msg(F("256KB")); break;
      case 4: print_Msg(F("512KB")); break;
      case 5:
        if (romType == 1 || romType == 2 || romType == 3) {
          print_Msg(F("1MB"));
        }
        else {
          print_Msg(F("1MB"));
        }
        break;
      case 6:
        if (romType == 1 || romType == 2 || romType == 3) {
          print_Msg(F("2MB"));
        }
        else {
          print_Msg(F("2MB"));
        }
        break;
      case 7: print_Msg(F("4MB")); break;
      case 82: print_Msg(F("1.1MB")); break;
      case 83: print_Msg(F("1.2MB")); break;
      case 84: print_Msg(F("1.5MB)")); break;
      default: print_Msg(F("Not found"));
    }
    println_Msg(F(""));
    print_Msg(F("Banks: "));
    println_Msg(romBanks);

    print_Msg(F("Sram Size: "));
    switch (sramSize) {
      case 0:
        if (romType == 6) {
          print_Msg(F("512B"));
        }
        else {
          print_Msg(F("None"));
        }
        break;
      case 1: print_Msg(F("2KB")); break;
      case 2: print_Msg(F("8KB")); break;
      case 3: print_Msg(F("32KB")); break;
      case 4: print_Msg(F("128KB")); break;
      default: print_Msg(F("Not found"));
    }
    println_Msg(F(""));
    print_Msg(F("Checksum: "));
    println_Msg(checksumStr);
    display_Update();

    // Wait for user input
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
  else {
    print_Error(F("GAMEPAK ERROR"), true);
  }
}

/******************************************
   I/O Functions
 *****************************************/

/******************************************
  Low level functions
*****************************************/
// Switch data pins to read
void dataIn_GB() {
  // Set to Input
  DDRC = 0x00;
}

byte readByte_GB(word myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CS(PH3) and RD(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeByte_GB(int myAddress, uint8_t myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) low
  PORTH &= ~(1 << 5);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) HIGH
  PORTH |= (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

/******************************************
  Game Boy functions
*****************************************/
// Read Cartridge Header
void getCartInfo_GB() {
  romType = readByte_GB(0x0147);
  romSize = readByte_GB(0x0148);
  sramSize = readByte_GB(0x0149);

  // ROM banks
  romBanks = 2; // Default 32K
  if (romSize >= 1) { // Calculate rom size
    romBanks = 2 << romSize;
  }

  // RAM banks
  sramBanks = 0; // Default 0K RAM
  if (romType == 6) {
    sramBanks = 1;
  }
  if (sramSize == 2) {
    sramBanks = 1;
  }
  if (sramSize == 3) {
    sramBanks = 4;
  }
  if (sramSize == 4) {
    sramBanks = 16;
  }
  if (sramSize == 5) {
    sramBanks = 8;
  }

  // RAM end address
  if (romType == 6) {
    sramEndAddress = 0xA1FF;  // MBC2 512bytes (nibbles)
  }
  if (sramSize == 1) {
    sramEndAddress = 0xA7FF;  // 2K RAM
  }
  if (sramSize > 1) {
    sramEndAddress = 0xBFFF;  // 8K RAM
  }

  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readByte_GB(0x014E), readByte_GB(0x014F));

  // Get name
  byte myByte = 0;
  byte myLength = 0;

  for (int addr = 0x0134; addr <= 0x13C; addr++) {
    myByte = readByte_GB(addr);
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 15) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }
}

// Dump ROM
void readROM_GB() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // create a new folder for the rom file
  EEPROM_readAnything(10, foldern);
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

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

  uint16_t romAddress = 0;

  // Read number of banks and switch banks
  for (uint16_t bank = 1; bank < romBanks; bank++) {
    // Switch data pins to output
    dataOut();

    if (romType >= 5) { // MBC2 and above
      writeByte_GB(0x2100, bank); // Set ROM bank
    }
    else { // MBC1
      writeByte_GB(0x6000, 0); // Set ROM Mode
      writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
      writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
    }

    // Switch data pins to intput
    dataIn_GB();

    if (bank > 1) {
      romAddress = 0x4000;
    }

    // Read up to 7FFF per bank
    while (romAddress <= 0x7FFF) {
      uint8_t readData[512];
      for (int i = 0; i < 512; i++) {
        readData[i] = readByte_GB(romAddress + i);
      }
      myFile.write(readData, 512);
      romAddress += 512;
    }
  }

  // Close the file:
  myFile.close();
}

unsigned int calc_checksum_GB (char* fileName, char* folder) {
  unsigned int calcChecksum = 0;
  int calcFilesize = 0;
  unsigned long i = 0;
  int c = 0;

  if (strcmp(folder, "root") != 0)
    sd.chdir(folder);

  // If file exists
  if (myFile.open(fileName, O_READ)) {
    calcFilesize = myFile.fileSize() * 8 / 1024 / 1024;
    for (i = 0; i < (myFile.fileSize() / 512); i++) {
      myFile.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksum += sdBuffer[c];
      }
    }
    myFile.close();
    sd.chdir();
    // Subtract checksum bytes
    calcChecksum -= readByte_GB(0x014E);
    calcChecksum -= readByte_GB(0x014F);

    // Return result
    return (calcChecksum);
  }
  // Else show error
  else {
    print_Error(F("DUMP ROM 1ST"), false);
    return 0;
  }
}

boolean compare_checksum_GB() {

  println_Msg(F("Calculating Checksum"));
  display_Update();

  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // last used rom folder
  EEPROM_readAnything(10, foldern);
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern - 1);

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum_GB(fileName, folder));

  if (strcmp(calcsumStr, checksumStr) == 0) {
    print_Msg(F("Result: "));
    println_Msg(calcsumStr);
    println_Msg(F("Checksum matches"));
    display_Update();
    return 1;
  }
  else {
    print_Msg(F("Result: "));
    println_Msg(calcsumStr);
    print_Error(F("Checksum Error"), false);
    return 0;
  }
}

// Read RAM
void readSRAM_GB() {
  // Does cartridge have RAM
  if (sramEndAddress > 0) {

    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".sav");

    // create a new folder for the save file
    EEPROM_readAnything(10, foldern);
    sprintf(folder, "GB/SAVE/%s/%d", romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(10, foldern);

    //open file on sd card
    if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
      print_Error(F("SD Error"), true);
    }

    dataIn_GB();

    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);

    dataOut();
    if (romType <= 4) { // MBC1
      writeByte_GB(0x6000, 1); // Set RAM Mode
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch SRAM banks
    for (uint8_t bank = 0; bank < sramBanks; bank++) {
      dataOut();
      writeByte_GB(0x4000, bank);

      // Read SRAM
      dataIn_GB();
      for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress += 64) {
        uint8_t readData[64];
        for (uint8_t i = 0; i < 64; i++) {
          readData[i] = readByte_GB(sramAddress + i);
        }
        myFile.write(readData, 64);
      }
    }

    // Disable SRAM
    dataOut();
    writeByte_GB(0x0000, 0x00);
    dataIn_GB();

    // Close the file:
    myFile.close();

    // Signal end of process
    print_Msg(F("Saved to "));
    print_Msg(folder);
    println_Msg(F("/"));
    display_Update();
  }
  else {
    print_Error(F("Cart has no SRAM"), false);
  }
}

// Write RAM
void writeSRAM_GB() {
  // Does cartridge have SRAM
  if (sramEndAddress > 0) {

    filePath[0] = '\0';
    sd.chdir("/");
    fileBrowser("Select sav file");
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // Set pins to input
      dataIn_GB();

      // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
      readByte_GB(0x0134);

      dataOut();

      if (romType <= 4) { // MBC1
        writeByte_GB(0x6000, 1); // Set RAM Mode
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch RAM banks
      for (uint8_t bank = 0; bank < sramBanks; bank++) {
        writeByte_GB(0x4000, bank);

        // Write RAM
        for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress++) {
          // Pull CS(PH3) LOW
          PORTH &= ~(1 << 3);
          // Write to RAM
          writeByte_GB(sramAddress, myFile.read());
          asm volatile("nop");
          asm volatile("nop");
          asm volatile("nop");
          // Pull CS(PH3) HIGH
          PORTH |= (1 << 3) ;
        }
      }
      // Disable RAM
      writeByte_GB(0x0000, 0x00);

      // Set pins to input
      dataIn_GB();

      // Close the file:
      myFile.close();
      display_Clear();
      println_Msg(F("SRAM writing finished"));
      display_Update();

    }
    else {
      print_Error(F("File doesnt exist"), false);
    }
  }
  else {
    print_Error(F("Cart has no SRAM"), false);
  }
}

// Check if the SRAM was written without any error
unsigned long verifySRAM_GB() {

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Variable for errors
    writeErrors = 0;

    dataIn_GB();

    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);

    // Does cartridge have RAM
    if (sramEndAddress > 0) {
      dataOut();
      if (romType <= 4) { // MBC1
        writeByte_GB(0x6000, 1); // Set RAM Mode
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch SRAM banks
      for (uint8_t bank = 0; bank < sramBanks; bank++) {
        dataOut();
        writeByte_GB(0x4000, bank);

        // Read SRAM
        dataIn_GB();
        for (uint16_t sramAddress = 0xA000; sramAddress <= sramEndAddress; sramAddress += 64) {
          //fill sdBuffer
          myFile.read(sdBuffer, 64);
          for (int c = 0; c < 64; c++) {
            if (readByte_GB(sramAddress + c) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
      dataOut();
      // Disable RAM
      writeByte_GB(0x0000, 0x00);
      dataIn_GB();
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    print_Error(F("Can't open file"), true);
  }
}

// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
void writeFlash_GB() {
  // Launch filebrowser
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser("Select file");
  display_Clear();

  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    myFile.seekCur(0x147);
    romType = myFile.read();
    romSize = myFile.read();
    // Go back to file beginning
    myFile.seekSet(0);

    // ROM banks
    romBanks = 2; // Default 32K
    if (romSize >= 1) { // Calculate rom size
      romBanks = 2 << romSize;
    }

    // Set data pins to output
    dataOut();

    // Set ROM bank hi 0
    writeByte_GB(0x3000, 0);
    // Set ROM bank low 0
    writeByte_GB(0x2000, 0);
    delay(100);

    // Reset flash
    writeByte_GB(0x555, 0xf0);
    delay(100);

    // ID command sequence
    writeByte_GB(0x555, 0xaa);
    writeByte_GB(0x2aa, 0x55);
    writeByte_GB(0x555, 0x90);

    dataIn_GB();

    // Read the two id bytes into a string
    sprintf(flashid, "%02X%02X", readByte_GB(0), readByte_GB(1));

    if (strcmp(flashid, "04D4") == 0) {
      println_Msg(F("MBM29F033C"));
      print_Msg(F("Banks: "));
      print_Msg(romBanks);
      println_Msg(F("/256"));
      display_Update();
    }
    else if (strcmp(flashid, "0141") == 0) {
      println_Msg(F("AM29F032B"));
      print_Msg(F("Banks: "));
      print_Msg(romBanks);
      println_Msg(F("/256"));
      display_Update();
    }
    else if (strcmp(flashid, "01AD") == 0) {
      println_Msg(F("AM29F016B"));
      print_Msg(F("Banks: "));
      print_Msg(romBanks);
      println_Msg(F("/128"));
      display_Update();
    }
    else {
      print_Msg(F("Flash ID: "));
      println_Msg(flashid);
      display_Update();
      print_Error(F("Unknown flashrom"), true);
    }
    dataOut();

    // Reset flash
    writeByte_GB(0x555, 0xf0);

    delay(100);
    println_Msg(F("Erasing flash"));
    display_Update();

    // Erase flash
    writeByte_GB(0x555, 0xaa);
    writeByte_GB(0x2aa, 0x55);
    writeByte_GB(0x555, 0x80);
    writeByte_GB(0x555, 0xaa);
    writeByte_GB(0x2aa, 0x55);
    writeByte_GB(0x555, 0x10);

    dataIn_GB();

    // Read the status register
    byte statusReg = readByte_GB(0);

    // After a completed erase D7 will output 1
    while ((statusReg & 0x80) != 0x80) {
      // Blink led
      PORTB ^= (1 << 4);
      delay(100);
      // Update Status
      statusReg = readByte_GB(0);
    }

    // Blankcheck
    println_Msg(F("Blankcheck"));
    display_Update();

    // Read x number of banks
    for (int currBank = 0; currBank < romBanks; currBank++) {
      // Blink led
      PORTB ^= (1 << 4);

      dataOut();

      // Set ROM bank
      writeByte_GB(0x2000, currBank);
      dataIn();

      for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
        uint8_t readData[512];
        for (int currByte = 0; currByte < 512; currByte++) {
          readData[currByte] = readByte_GB(currAddr + currByte);
        }
        for (int j = 0; j < 512; j++) {
          if (readData[j] != 0xFF) {
            println_Msg(F("Not empty"));
            print_Error(F("Erase failed"), true);
          }
        }
      }
    }

    println_Msg(F("Writing flash"));
    display_Update();

    // Write flash
    dataOut();

    for (int currBank = 0; currBank < romBanks; currBank++) {
      // Blink led
      PORTB ^= (1 << 4);

      // Set ROM bank
      writeByte_GB(0x2000, currBank);
      // 0x2A8000 fix
      writeByte_GB(0x4000, 0x0);

      for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
        myFile.read(sdBuffer, 512);

        for (int currByte = 0; currByte < 512; currByte++) {
          // Write command sequence
          writeByte_GB(0x555, 0xaa);
          writeByte_GB(0x2aa, 0x55);
          writeByte_GB(0x555, 0xa0);
          // Write current byte
          writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

          // Set data pins to input
          dataIn();

          // Setting CS(PH3) and OE/RD(PH6) LOW
          PORTH &= ~((1 << 3) | (1 << 6));

          // Busy check
          //int timeout = 0;
          while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
            /* __asm__("nop\n\t");
              // timeout in case writing fails
              timeout++;
              if (timeout > 32760) {
               break;
              }*/
          }

          // Switch CS(PH3) and OE/RD(PH6) to HIGH
          PORTH |= (1 << 3) | (1 << 6);

          // Set data pins to output
          dataOut();
        }
      }
    }

    // Set data pins to input again
    dataIn_GB();

    println_Msg(F("Verifying"));
    display_Update();

    // Go back to file beginning
    myFile.seekSet(0);
    unsigned int addr = 0;
    writeErrors = 0;

    // Verify flashrom
    uint16_t romAddress = 0;

    // Read number of banks and switch banks
    for (uint16_t bank = 1; bank < romBanks; bank++) {
      // Switch data pins to output
      dataOut();

      if (romType >= 5) { // MBC2 and above
        writeByte_GB(0x2100, bank); // Set ROM bank
      }
      else { // MBC1
        writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }

      // Switch data pins to intput
      dataIn_GB();

      if (bank > 1) {
        romAddress = 0x4000;
      }
      // Blink led
      PORTB ^= (1 << 4);

      // Read up to 7FFF per bank
      while (romAddress <= 0x7FFF) {
        // Fill sdBuffer
        myFile.read(sdBuffer, 512);
        // Compare
        for (int i = 0; i < 512; i++) {
          if (readByte_GB(romAddress + i) != sdBuffer[i]) {
            writeErrors++;
          }
        }
        romAddress += 512;
      }
    }
    // Close the file:
    myFile.close();

    if (writeErrors == 0) {
      println_Msg(F("OK"));
      display_Update();
    }
    else {
      print_Msg(F("Error: "));
      print_Msg(writeErrors);
      println_Msg(F(" bytes "));
      print_Error(F("did not verify."), false);
    }
  }
  else {
    println_Msg(F("Can't open file"));
    display_Update();
  }
}

//******************************************
// End of File
//******************************************
