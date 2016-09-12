//******************************************
// GAME BOY
//******************************************

/******************************************
   Variables
 *****************************************/
// Game Boy
int sramBanks;

/******************************************
   Menu
 *****************************************/
// GB menu items
const char GBMenuItem1[] PROGMEM = "Read Rom";
const char GBMenuItem2[] PROGMEM = "Read Save";
const char GBMenuItem3[] PROGMEM = "Write Save";
const char GBMenuItem4[] PROGMEM = "Reset";
const char* const menuOptionsGB[] PROGMEM = {GBMenuItem1, GBMenuItem2, GBMenuItem3, GBMenuItem4};

void gbMenu() {
  // Output a high signal on CS(PH3) WR(PH5) RD(PH6)
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);

  // create menu with title and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions of of progmem
  convertPgm(menuOptionsGB, 4);
  mainMenu = question_box("GB Cart Reader", menuOptions, 4, 0);

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
      // Change working dir to root
      sd.chdir("/");
      readSRAM_GB();
      break;

    case 2:
      display_Clear();
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
  // Enable Internal Pullups
  //PORTC = 0xFF;

  // Print start page
  getCartInfo_GB();
  display_Clear();
  if (strcmp(checksumStr, "00") != 0) {
    println_Msg(F("GB Cart Info"));
    print_Msg(F("Rom Name: "));
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
      case 0:
        print_Msg(F("32KByte"));
        break;
      case 1:
        print_Msg(F("64KByte"));
        break;
      case 2:
        print_Msg(F("128KByte"));
        break;
      case 3:
        print_Msg(F("256KByte"));
        break;
      case 4:
        print_Msg(F("512KByte"));
        break;
      case 5:
        if (romType == 1 || romType == 2 || romType == 3) {
          print_Msg(F("1MByte"));
        }
        else {
          print_Msg(F("1MByte"));
        }
        break;
      case 6:
        if (romType == 1 || romType == 2 || romType == 3) {
          print_Msg(F("2MByte"));
        }
        else {
          print_Msg(F("2MByte"));
        }
        break;
      case 7:
        print_Msg(F("4MByte"));
        break;
      case 82:
        print_Msg(F("1.1MByten"));
        break;
      case 83:
        print_Msg(F("1.2MByte"));
        break;
      case 84:
        print_Msg(F("1.5MByte"));
        break;
      default:
        print_Msg(F("Not found"));
    }
    println_Msg(F(""));
    print_Msg(F("Banks: "));
    println_Msg(numBanks);

    print_Msg(F("Sram Size: "));
    switch (sramSize) {
      case 0:
        if (romType == 6) {
          print_Msg(F("512 bytes"));
        }
        else {
          print_Msg(F("None"));
        }
        break;
      case 1:
        print_Msg(F("2 KBytes"));
        break;
      case 2:
        print_Msg(F("8 KBytes"));
        break;
      case 3:
        print_Msg(F("32 KBytes"));
        break;
      case 4:
        print_Msg(F("128 KBytes"));
        break;
      default:
        print_Msg(F("Not found"));
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
  // Dump name into 8.3 compatible format
  byte myByte = 0;
  byte myLength = 0;

  for (int addr = 0x0134; addr <= 0x13C; addr++) {
    myByte = readByte_GB(addr);
    if (((char(myByte) >= 48 && char(myByte) <= 57) || (char(myByte) >= 65 && char(myByte) <= 122)) && myLength < 8) {
      romName[myLength] = char(myByte);
      myLength++;
    }
  }
  // Get Checksum as string
  sprintf(checksumStr, "%02X%02X", readByte_GB(0x014E), readByte_GB(0x014F));
  romType = readByte_GB(0x0147);
  romSize = readByte_GB(0x0148);
  sramSize = readByte_GB(0x0149);
  numBanks = 2; // Default 32K
  if (romSize == 1) {
    numBanks = 4;
  }
  if (romSize == 2) {
    numBanks = 8;
  }
  if (romSize == 3) {
    numBanks = 16;
  }
  if (romSize == 4) {
    numBanks = 32;
  }
  if (romSize == 5 && (romType == 1 || romType == 2 || romType == 3)) {
    numBanks = 63;
  }
  else if (romSize == 5) {
    numBanks = 64;
  }
  if (romSize == 6 && (romType == 1 || romType == 2 || romType == 3)) {
    numBanks = 125;
  }
  else if (romSize == 6) {
    numBanks = 128;
  }
  if (romSize == 7) {
    numBanks = 255;
  }
  if (romSize == 82) {
    numBanks = 72;
  }
  if (romSize == 83) {
    numBanks = 80;
  }
  if (romSize == 84) {
    numBanks = 96;
  }
  sramBanks = 1; // Default 8K RAM
  if (sramSize == 3) {
    sramBanks = 4;
  }
  if (sramSize == 4) {
    sramBanks = 16;  // GB Camera
  }
}

// Dump ROM
void readROM_GB() {
  // Get name, add extension and convert to char array for sd lib
  char fileName[26];
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // create a new folder for the rom file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  //clear the screen
  display_Clear();
  println_Msg(F("Creating folder: "));
  println_Msg(folder);
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  unsigned int addr = 0;

  // Read x number of banks
  for (int y = 1; y < numBanks; y++) {
    dataOut();
    // Set ROM bank
    writeByte_GB(0x2100, y);
    dataIn();
    if (y > 1) {
      addr = 0x4000;
    }
    for (; addr <= 0x7FFF; addr = addr + 512) {
      uint8_t readData[512];
      for (int i = 0; i < 512; i++) {
        readData[i] = readByte_GB(addr + i);
      }
      myFile.write(readData, 512);
    }
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  print_Msg(F("Saved as "));
  println_Msg(fileName);
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

  char fileName[26];
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // last used rom folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "ROM/%s/%d", romName, foldern - 1);

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

  // Get name, add extension and convert to char array for sd lib
  char fileName[26];
  strcpy(fileName, romName);
  strcat(fileName, ".sav");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
  readByte_GB(0x0134);

  unsigned int addr = 0;
  unsigned int endaddr = 0;
  if (romType == 6 && sramSize == 0) {
    endaddr = 0xA1FF;  // MBC2 512bytes (nibbles)
  }
  if (sramSize == 1) {
    endaddr = 0xA7FF;  // 2K RAM
  }
  if (sramSize > 1) {
    endaddr = 0xBFFF;  // 8K RAM
  }

  // Does cartridge have RAM
  if (endaddr > 0) {
    dataOut();
    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch RAM banks
    for (int bank = 0; bank < sramBanks; bank++) {
      dataOut();
      writeByte_GB(0x4000, bank);

      // Read RAM
      dataIn();
      for (addr = 0xA000; addr <= endaddr; addr = addr + 64) {
        uint8_t readData[64];
        for (int i = 0; i < 64; i++) {
          readData[i] = readByte_GB(addr + i);
        }
        myFile.write(readData, 64);
      }
    }
    dataOut();
    // Disable RAM
    writeByte_GB(0x0000, 0x00);
    dataIn();
  }
  else {
    print_Error(F("Cart has no SRAM"), false);
  }
  // Close the file:
  myFile.close();

  // Signal end of process
  print_Msg(F("Saved to SAVE/"));
  print_Msg(romName);
  print_Msg(F("/"));
  print_Msg(foldern - 1);
  print_Msg(F("/"));
  println_Msg(fileName);
  display_Update();
}

// Write RAM
void writeSRAM_GB() {

  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser("Select sav file");
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);
    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (romType == 6 && sramSize == 0) {
      endaddr = 0xA1FF;  // MBC2 512bytes (nibbles)
    }
    if (sramSize == 1) {
      endaddr = 0xA7FF;  // 2K RAM
    }
    if (sramSize > 1) {
      endaddr = 0xBFFF;  // 8K RAM
    }

    // Does cartridge have RAM
    if (endaddr > 0) {
      dataOut();
      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch RAM banks
      for (int bank = 0; bank < sramBanks; bank++) {
        writeByte_GB(0x4000, bank);

        // Write RAM
        for (addr = 0xA000; addr <= endaddr; addr = addr + 64) {

          // Wait for serial input
          for (uint8_t i = 0; i < 64; i++) {
            // Pull CS(PH3) LOW
            PORTH &= ~(1 << 3);
            // Write to RAM
            writeByte_GB(addr + i, myFile.read());
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            // Pull CS(PH3) HIGH
            PORTH |= (1 << 3) ;
          }
        }
      }

      // Disable RAM
      writeByte_GB(0x0000, 0x00);
      Serial.flush(); // Flush any serial data that wasn't processed
    }
    else {
      print_Error(F("Cart has no SRAM"), false);
    }
    // Set pins to input
    dataIn();

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

// Check if the SRAM was written without any error
unsigned long verifySRAM_GB() {

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {

    // Variable for errors
    writeErrors = 0;

    // MBC2 Fix (unknown why this fixes it, maybe has to read ROM before RAM?)
    readByte_GB(0x0134);

    unsigned int addr = 0;
    unsigned int endaddr = 0;
    if (romType == 6 && sramSize == 0) {
      endaddr = 0xA1FF;  // MBC2 512bytes (nibbles)
    }
    if (sramSize == 1) {
      endaddr = 0xA7FF;  // 2K RAM
    }
    if (sramSize > 1) {
      endaddr = 0xBFFF;  // 8K RAM
    }

    // Does cartridge have RAM
    if (endaddr > 0) {
      dataOut();
      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch RAM banks
      for (int bank = 0; bank < sramBanks; bank++) {
        dataOut();
        writeByte_GB(0x4000, bank);

        // Read RAM
        dataIn();
        for (addr = 0xA000; addr <= endaddr; addr = addr + 64) {
          //fill sdBuffer
          myFile.read(sdBuffer, 64);
          for (int c = 0; c < 64; c++) {
            if (readByte_GB(addr + c) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
      dataOut();
      // Disable RAM
      writeByte_GB(0x0000, 0x00);
      dataIn();
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    print_Error(F("Can't open file"), true);
  }
}

//******************************************
// End of File
//******************************************
