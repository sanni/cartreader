//******************************************
// GAME BOY MODULE
//******************************************
#ifdef enable_GBX

/******************************************
   Variables
 *****************************************/
// Game Boy
int sramBanks;
int romBanks;
word lastByte = 0;

/******************************************
   Menu
 *****************************************/
// GBx start menu
static const char gbxMenuItem1[] PROGMEM = "Game Boy (Color)";
static const char gbxMenuItem2[] PROGMEM = "GB Advance (3V)";
static const char gbxMenuItem3[] PROGMEM = "Flash GBC Cart";
static const char gbxMenuItem4[] PROGMEM = "NPower GB Memory";
static const char gbxMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsGBx[] PROGMEM = {gbxMenuItem1, gbxMenuItem2, gbxMenuItem3, gbxMenuItem4, gbxMenuItem5};

// GB menu items
static const char GBMenuItem1[] PROGMEM = "Read ROM";
static const char GBMenuItem2[] PROGMEM = "Read Save";
static const char GBMenuItem3[] PROGMEM = "Write Save";
static const char GBMenuItem4[] PROGMEM = "Reset";
static const char* const menuOptionsGB[] PROGMEM = {GBMenuItem1, GBMenuItem2, GBMenuItem3, GBMenuItem4};

// GB Flash items
static const char GBFlashItem1[] PROGMEM = "29F Cart (MBC3)";
static const char GBFlashItem2[] PROGMEM = "29F Cart (MBC5)";
static const char GBFlashItem3[] PROGMEM = "29F Cart (CAM)";
static const char GBFlashItem4[] PROGMEM = "CFI Cart";
static const char GBFlashItem5[] PROGMEM = "CFI Cart and Save";
static const char GBFlashItem6[] PROGMEM = "GB Smart";
static const char GBFlashItem7[] PROGMEM = "Reset";
static const char* const menuOptionsGBFlash[] PROGMEM = {GBFlashItem1, GBFlashItem2, GBFlashItem3, GBFlashItem4, GBFlashItem5, GBFlashItem6, GBFlashItem7};

// Start menu for both GB and GBA
void gbxMenu() {
  // create menu with title and 4 options to choose from
  unsigned char gbType;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBx, 5);
  gbType = question_box(F("Select Game Boy"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (gbType)
  {
    case 0:
      display_Clear();
      display_Update();
      setup_GB();
      mode =  mode_GB;
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_GBA();
      mode =  mode_GBA;
      break;

    case 2:
      // create submenu with title and 7 options to choose from
      unsigned char gbFlash;
      // Copy menuOptions out of progmem
      convertPgm(menuOptionsGBFlash, 7);
      gbFlash = question_box(F("Select type"), menuOptions, 7, 0);

      // wait for user choice to come back from the question box menu
      switch (gbFlash)
      {
        case 0:
          //Flash MBC3
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;

          // Change working dir to root
          sd.chdir("/");
          //MBC3
          writeFlash29F_GB(3, 1);
          // Reset
          println_Msg(F("Press Button..."));
          display_Update();
          wait();
          resetArduino();
          break;

        case 1:
          //Flash MBC5
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;

          // Change working dir to root
          sd.chdir("/");
          //MBC5
          writeFlash29F_GB(5, 1);
          // Reset
          println_Msg(F("Press Button..."));
          display_Update();
          wait();
          resetArduino();
          break;

        case 2:
          //Flash GB Camera
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;

          //Flash first bank with erase
          // Change working dir to root
          sd.chdir("/");
          //MBC3
          writeFlash29F_GB(3, 1);
          println_Msg(F("Press Button..."));
          display_Update();
          wait();

          display_Clear();
          println_Msg(F("Please change the"));
          println_Msg(F("switch on the cart"));
          println_Msg(F("to B2 (Bank 2)"));
          println_Msg(F("if you want to flash"));
          println_Msg(F("a second game"));
          println_Msg(F(""));
          println_Msg(F("Press Button..."));
          display_Update();
          wait();

          // Flash second bank without erase
          // Change working dir to root
          sd.chdir("/");
          //MBC3
          writeFlash29F_GB(3, 0);

          // Reset
          println_Msg(F(""));
          println_Msg(F("Press Button..."));
          display_Update();
          wait();
          resetArduino();
          break;

        case 3:
          // Flash CFI
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;

          // Change working dir to root
          sd.chdir("/");
          // Launch filebrowser
          filePath[0] = '\0';
          sd.chdir("/");
          fileBrowser(F("Select file"));
          display_Clear();
          identifyCFI_GB();
          if (!writeCFI_GB()) {
            display_Clear();
            println_Msg(F("Flashing failed, time out!"));
            println_Msg(F("Press Button..."));
            display_Update();
            wait();
          }
          // Reset
          wait();
          resetArduino();
          break;

        case 4:
          // Flash CFI and Save
          display_Clear();
          display_Update();
          setup_GB();
          mode =  mode_GB;

          // Change working dir to root
          sd.chdir("/");
          // Launch filebrowser
          filePath[0] = '\0';
          sd.chdir("/");
          fileBrowser(F("Select file"));
          display_Clear();
          identifyCFI_GB();
          if (!writeCFI_GB()) {
            display_Clear();
            println_Msg(F("Flashing failed, time out!"));
            println_Msg(F("Press Button..."));
            display_Update();
            wait();
            resetArduino();
          }
          getCartInfo_GB();
          // Does cartridge have SRAM
          if (lastByte > 0) {
            // Remove file name ending
            int pos = -1;
            while (fileName[++pos] != '\0') {
              if (fileName[pos] == '.') {
                fileName[pos] = '\0';
                break;
              }
            }
            sprintf(filePath, "/GB/SAVE/%s/", fileName);
            bool saveFound = false;
            if (sd.exists(filePath)) {
              EEPROM_readAnything(0, foldern);
              for (int i = foldern; i >= 0; i--) {
                sprintf(filePath, "/GB/SAVE/%s/%d/%s.SAV", fileName, i, fileName);
                if (sd.exists(filePath)) {
                  print_Msg(F("Save number "));
                  print_Msg(i);
                  println_Msg(F(" found."));
                  saveFound = true;
                  sprintf(filePath, "/GB/SAVE/%s/%d", fileName, i);
                  sprintf(fileName, "%s.SAV", fileName);
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
                }
              }
            }
            if (!saveFound) {
              println_Msg(F("Error: No save found."));
            }
          }
          else {
            print_Error(F("Cart has no Sram"), false);
          }
          // Reset
          wait();
          resetArduino();
          break;

        case 5:
          // Flash GB Smart
          display_Clear();
          display_Update();
          setup_GBSmart();
          mode = mode_GB_GBSmart;
          break;

        case 6:
          resetArduino();
          break;
      }
      break;

    case 3:
      // Flash GB Memory
      display_Clear();
      display_Update();
      setup_GBM();
      mode =  mode_GBM;
      break;

    case 4:
      resetArduino();
      break;
  }
}

void gbMenu() {
  // create menu with title and 3 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGB, 4);
  mainMenu = question_box(F("GB Cart Reader"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_GB();
      compare_checksums_GB();
#ifdef global_log
      save_log();
#endif
      break;

    case 1:
      display_Clear();
      // Does cartridge have SRAM
      if (lastByte > 0) {
        // Change working dir to root
        sd.chdir("/");
        if (romType == 32)
          readSRAMFLASH_MBC6_GB();
        else
          readSRAM_GB();
      }
      else {
        print_Error(F("No save or unsupported type"), false);
      }
      println_Msg(F(""));
      break;

    case 2:
      display_Clear();
      // Does cartridge have SRAM
      if (lastByte > 0) {
        // Change working dir to root
        sd.chdir("/");
        filePath[0] = '\0';
        fileBrowser(F("Select sav file"));

        if (romType == 32) {
          writeSRAMFLASH_MBC6_GB();
        }
        else {
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
      }
      else {
        print_Error(F("No save or unsupported type"), false);
      }
      println_Msg(F(""));
      break;

    case 3:
      resetArduino();
      break;
  }
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_GB() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output RST(PH0) CLK(PH1) CS(PH3) WR(PH5) RD(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 3) | (1 << 5) | (1 << 6);
  // Output a low signal on CLK(PH1) to disable writing GB Camera RAM
  // Output a low signal on RST(PH0) to initialize MMC correctly
  PORTH &= ~((1 << 0) | (1 << 1));

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Enable Internal Pullups
  PORTC = 0xFF;

  delay(400);

  // RST(PH0) to H
  PORTH |= (1 << 0);

  // Print start page
  getCartInfo_GB();
  showCartInfo_GB();

  // MMM01 initialize
  if (romType >= 11 && romType <= 13) {
    writeByte_GB(0x3fff, 0x00);
    writeByte_GB(0x5fff, 0x40);
    writeByte_GB(0x7fff, 0x01);
    writeByte_GB(0x1fff, 0x3a);
    writeByte_GB(0x1fff, 0x7a);
  }
}

void showCartInfo_GB() {
  display_Clear();
  if (strcmp(checksumStr, "00") != 0) {
    print_Msg(F("Title: "));
    println_Msg(romName);
    if (cartID[0] != 0) {
      print_Msg(F("Serial: "));
      println_Msg(cartID);
    }
    print_Msg(F("Revision: "));
    println_Msg(romVersion);

    print_Msg(F("Mapper: "));
    if ((romType == 0) || (romType == 8) || (romType == 9))
      print_Msg(F("none"));
    else if ((romType == 1) || (romType == 2) || (romType == 3))
      print_Msg(F("MBC1"));
    else if ((romType == 5) || (romType == 6))
      print_Msg(F("MBC2"));
    else if ((romType == 11) || (romType == 12) || (romType == 13))
      print_Msg(F("MMM01"));
    else if ((romType == 15) || (romType == 16) || (romType == 17) || (romType == 18) || (romType == 19))
      print_Msg(F("MBC3"));
    else if ((romType == 21) || (romType == 22) || (romType == 23))
      print_Msg(F("MBC4"));
    else if ((romType == 25) || (romType == 26) || (romType == 27) || (romType == 28) || (romType == 29) || (romType == 309))
      print_Msg(F("MBC5"));
    else if (romType == 32)
      print_Msg(F("MBC6"));
    else if (romType == 34)
      print_Msg(F("MBC7"));
    else if (romType == 252)
      print_Msg(F("Camera"));
    else if (romType == 253)
      print_Msg(F("TAMA5"));
    else if (romType == 254)
      print_Msg(F("HuC-3"));
    else if (romType == 255)
      print_Msg(F("HuC-1"));
    else if ((romType == 0x101) || (romType == 0x103))
      print_Msg(F("MBC1M"));
    else if (romType == 0x104)
      print_Msg(F("M161"));

    println_Msg(F(""));
    print_Msg(F("ROM Size: "));
    switch (romSize) {
      case 0:
        print_Msg(F("32 KB"));
        break;

      case 1:
        print_Msg(F("64 KB"));
        break;

      case 2:
        print_Msg(F("128 KB"));
        break;

      case 3:
        print_Msg(F("256 KB"));
        break;

      case 4:
        print_Msg(F("512 KB"));
        break;

      case 5:
        print_Msg(F("1 MB"));
        break;

      case 6:
        print_Msg(F("2 MB"));
        break;

      case 7:
        print_Msg(F("4 MB"));
        break;

      case 8:
        print_Msg(F("8 MB"));
        break;
    }

    println_Msg(F(""));
    //print_Msg(F("Banks: "));
    //println_Msg(romBanks);

    print_Msg(F("Save Size: "));
    switch (sramSize) {
      case 0:
        if (romType == 6) {
          print_Msg(F("512 Byte"));
        }
        else if (romType == 0x22) {
          if (strncmp(cartID, "KCEJ", 4) == 0) {
            print_Msg(F("512 Byte"));
          }
          else {
            print_Msg(F("256 Byte"));
          }
        }
        else if (romType == 0xFD) {
          print_Msg(F("32 Byte"));
        }
        else {
          print_Msg(F("None"));
        }
        break;
      case 1:
        print_Msg(F("2 KB"));
        break;

      case 2:
        print_Msg(F("8 KB"));
        break;

      case 3:
        if (romType == 0x20) {
          print_Msg(F("1.03 MB"));
        } else {
          print_Msg(F("32 KB"));
        }
        break;

      case 4:
        print_Msg(F("128 KB"));
        break;

      case 5:
        print_Msg(F("64 KB"));
        break;

      default: print_Msg(F("None"));
    }
    println_Msg(F(""));
    //print_Msg(F("Checksum: "));
    //println_Msg(checksumStr);
    //display_Update();

    // Wait for user input
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
  else {
    print_Error(F("GAMEPAK ERROR"), true);
  }
}

/******************************************
  Low level functions
*****************************************/
byte readByte_GB(word myAddress) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  // Switch data pins to input
  DDRC = 0x00;
  // Enable pullups
  PORTC = 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch RD(PH6) to LOW
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch and RD(PH6) to HIGH
  PORTH |= (1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

void writeByte_GB(int myAddress, byte myData) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  // Set data
  PORTC = myData;
  // Switch data pins to output
  DDRC = 0xFF;

  // Wait till output is stable
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) low
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull WR(PH5) HIGH
  PORTH |= (1 << 5);
  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch data pins to input
  DDRC = 0x00;
  // Enable pullups
  PORTC = 0xFF;
}

// Triggers CS and CLK pin
byte readByteSRAM_GB(word myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  // Switch data pins to input
  DDRC = 0x00;
  // Enable pullups
  PORTC = 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
  PORTH &= ~((1 << 3) | (1 << 1));
  // Pull RD(PH6) LOW
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  byte tempByte = PINC;

  // Pull RD(PH6) HIGH
  PORTH |=  (1 << 6);
  if (romType == 252) {
    // Pull CS(PH3) HIGH
    PORTH |= (1 << 3);
  }
  else {
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
    PORTH |= (1 << 3) | (1 << 1);
  }
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempByte;
}

// Triggers CS and CLK pin
void writeByteSRAM_GB(int myAddress, byte myData) {
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  // Set data
  PORTC = myData;
  // Switch data pins to output
  DDRC = 0xFF;

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (romType == 252 || romType == 253) {
    // Pull CS(PH3) LOW
    PORTH &= ~(1 << 3);
    // Pull CLK(PH1)(for GB CAM) HIGH
    PORTH |= (1 << 1);
    // Pull WR(PH5) low
    PORTH &= ~(1 << 5);
  }
  else {
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) LOW
    PORTH &= ~((1 << 3) | (1 << 1));
    // Pull WR(PH5) low
    PORTH &= ~(1 << 5);
  }

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  if (romType == 252 || romType == 253) {
    // Pull WR(PH5) HIGH
    PORTH |= (1 << 5);
    // Pull CS(PH3) HIGH
    PORTH |= (1 << 3);
    // Pull  CLK(PH1) LOW (for GB CAM)
    PORTH &= ~(1 << 1);
  }
  else {
    // Pull WR(PH5) HIGH
    PORTH |= (1 << 5);
    // Pull CS(PH3) CLK(PH1)(for FRAM MOD) HIGH
    PORTH |= (1 << 3) | (1 << 1);
  }

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch data pins to input
  DDRC = 0x00;
  // Enable pullups
  PORTC = 0xFF;
}

/******************************************
  Game Boy functions
*****************************************/
// Read Cartridge Header
void getCartInfo_GB() {
  // Read Header into array
  for (int currByte = 0x100; currByte < 0x150; currByte++) {
    sdBuffer[currByte] = readByte_GB(currByte);
  }

  /* Compare Nintendo logo against known checksum, 156 bytes starting at 0x04
    word logoChecksum = 0;
    for (int currByte = 0x104; currByte < 0x134; currByte++) {
    logoChecksum += sdBuffer[currByte];
    }

    if (logoChecksum != 0x1546) {
    print_Error(F("STARTUP LOGO ERROR"), false);
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F("Press Button to"));
    println_Msg(F("ignore or powercycle"));
    println_Msg(F("to try again"));
    display_Update();
    wait();
    }
  */

  // Calculate header checksum
  byte headerChecksum = 0;
  for (int currByte = 0x134; currByte < 0x14D; currByte++) {
    headerChecksum = headerChecksum - sdBuffer[currByte] - 1;
  }

  if (headerChecksum != sdBuffer[0x14D]) {
    // Read Header into array a second time
    for (int currByte = 0x100; currByte < 0x150; currByte++) {
      sdBuffer[currByte] = readByte_GB(currByte);
    }
    // Calculate header checksum a second time
    headerChecksum = 0;
    for (int currByte = 0x134; currByte < 0x14D; currByte++) {
      headerChecksum = headerChecksum - sdBuffer[currByte] - 1;
    }
  }

  if (headerChecksum != sdBuffer[0x14D]) {
    print_Error(F("HEADER CHECKSUM ERROR"), false);
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F("Press Button to"));
    println_Msg(F("ignore or clean"));
    println_Msg(F("cart and try again"));
    display_Update();
    wait();
  }

  romType = sdBuffer[0x147];
  romSize = sdBuffer[0x148];
  sramSize = sdBuffer[0x149];

  // ROM banks
  switch (romSize) {
    case 0x00:
      romBanks = 2;
      break;
    case 0x01:
      romBanks = 4;
      break;
    case 0x02:
      romBanks = 8;
      break;
    case 0x03:
      romBanks = 16;
      break;
    case 0x04:
      romBanks = 32;
      break;
    case 0x05:
      romBanks = 64;
      break;
    case 0x06:
      romBanks = 128;
      break;
    case 0x07:
      romBanks = 256;
      break;
    case 0x08:
      romBanks = 512;
      break;
    default:
      romBanks = 2;
  }

  // SRAM banks
  sramBanks = 0;
  if (romType == 6) {
    sramBanks = 1;
  }

  // SRAM size
  switch (sramSize) {
    case 2:
      sramBanks = 1;
      break;
    case 3:
      sramBanks = 4;
      break;
    case 4:
      sramBanks = 16;
      break;
    case 5:
      sramBanks = 8;
      break;
  }

  // Last byte of SRAM
  if (romType == 6) {
    lastByte = 0xA1FF;
  }
  if (sramSize == 1) {
    lastByte = 0xA7FF;
  }
  else if (sramSize > 1) {
    lastByte = 0xBFFF;
  }
  
  // MBC6
  if (romType == 32) {
    sramBanks = 8;
    lastByte = 0xAFFF;
  }
  
  // Get Checksum as string
  eepbit[6] = sdBuffer[0x14E];
  eepbit[7] = sdBuffer[0x14F];
  sprintf(checksumStr, "%02X%02X", eepbit[6], eepbit[7]);

  // Get name
  byte myByte = 0;
  byte myLength = 0;
  byte x = 0;
  if (sdBuffer[0x143] == 0x80 || sdBuffer[0x143] == 0xC0) {
    x++;
  }
  for (int addr = 0x0134; addr <= 0x0143 - x; addr++) {
    myByte = sdBuffer[addr];
    if (isprint(myByte) && myByte != '<' && myByte != '>' && myByte != ':' && myByte != '"' && myByte != '/' && myByte != '\\' && myByte != '|' && myByte != '?' && myByte != '*') {
      romName[myLength] = char(myByte);
    } else {
      if (romName[myLength - 1] == 0x5F) myLength--;
      romName[myLength] = 0x5F;
    }
    myLength++;
  }

  // Find Game Serial
  cartID[0] = 0;
  if (sdBuffer[0x143] == 0x80 || sdBuffer[0x143] == 0xC0) {
    if ((romName[myLength - 4] == 'A' || romName[myLength - 4] == 'B' || romName[myLength - 4] == 'H' || romName[myLength - 4] == 'K' || romName[myLength - 4] == 'V') && (romName[myLength - 1] == 'A' || romName[myLength - 1] == 'B' || romName[myLength - 1] == 'D' || romName[myLength - 1] == 'E' || romName[myLength - 1] == 'F' || romName[myLength - 1] == 'I' || romName[myLength - 1] == 'J' || romName[myLength - 1] == 'K' || romName[myLength - 1] == 'P' || romName[myLength - 1] == 'S' || romName[myLength - 1] == 'U' || romName[myLength - 1] == 'X' || romName[myLength - 1] == 'Y')) {
      cartID[0] = romName[myLength - 4];
      cartID[1] = romName[myLength - 3];
      cartID[2] = romName[myLength - 2];
      cartID[3] = romName[myLength - 1];
      myLength -= 4;
      romName[myLength] = 0;
    }
  }

  // Strip trailing white space
  for (unsigned int i = myLength - 1; i > 0; i--) {
    if ((romName[i] != 0x5F) && (romName[i] != 0x20)) break;
    romName[i] = 0x00;
    myLength--;
  }

  // M161 (Mani 4 in 1)
  if ((strncmp(romName, "TETRIS SET", 10) == 0) && (sdBuffer[0x14D] == 0x3F)) {
    romType = 0x104;
  }

  // MMM01 (Mani 4 in 1)
  if (
    (strncmp(romName, "BOUKENJIMA2 SET", 15) == 0) && (sdBuffer[0x14D] == 0) ||
    (strncmp(romName, "BUBBLEBOBBLE SET", 16) == 0) && (sdBuffer[0x14D] == 0xC6) ||
    (strncmp(romName, "GANBARUGA SET", 13) == 0) && (sdBuffer[0x14D] == 0x90) ||
    (strncmp(romName, "RTYPE 2 SET", 11) == 0) && (sdBuffer[0x14D] == 0x32)) {
    romType = 0x0B;
  }

  // MBC1M
  if (
    (strncmp(romName, "MOMOCOL", 7) == 0) && (sdBuffer[0x14D] == 0x28) ||
    (strncmp(romName, "BOMCOL", 6) == 0) && (sdBuffer[0x14D] == 0x86) ||
    (strncmp(romName, "GENCOL", 6) == 0) && (sdBuffer[0x14D] == 0x8A) ||
    (strncmp(romName, "SUPERCHINESE 123", 16) == 0) && (sdBuffer[0x14D] == 0xE4) ||
    (strncmp(romName, "MORTALKOMBATI&II", 16) == 0) && (sdBuffer[0x14D] == 0xB9) ||
    (strncmp(romName, "MORTALKOMBAT DUO", 16) == 0) && (sdBuffer[0x14D] == 0xA7)) {
    romType += 0x100;
  }
  
  // ROM revision
  romVersion = sdBuffer[0x14C];
}

/******************************************
  ROM functions
*****************************************/
// Read ROM
void readROM_GB() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // create a new folder for the rom file
  EEPROM_readAnything(0, foldern);
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
  EEPROM_writeAnything(0, foldern);

  //open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("Can't create file on SD"), true);
  }

  int endAddress = 0x7FFF;
  word romAddress = 0;
  word startBank = 1;

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
  draw_progressbar(0, totalProgressBar);

  // M161 banks are double size and start with 0
  if (romType == 0x104) {
    startBank = 0;
    romBanks >>= 1;
    endAddress = 0x7FFF;
  }
  // MBC6 banks are half size
  else if (romType == 32) {
    romBanks <<= 1;
    endAddress = 0x3FFF;
  }

  for (word currBank = startBank; currBank < romBanks; currBank++) {
    // Second bank starts at 0x4000
    if (currBank > 1) {
      romAddress = 0x4000;

      // MBC6 banks are half size
      if (romType == 32) {
        endAddress = 0x5FFF;
      }
    }

    // Set ROM bank for M161
    if (romType == 0x104) {
      romAddress = 0;
      PORTH &= ~(1 << 0);
      delay(50);
      PORTH |= (1 << 0);
      writeByte_GB(0x4000, currBank & 0x7);
    }

    // Set ROM bank for MBC1M
    else if (romType == 0x101 || romType == 0x103) {
      if (currBank < 10) {
          writeByte_GB(0x4000, currBank >> 4);
          writeByte_GB(0x2000, (currBank & 0x1f));
      } else {
          writeByte_GB(0x4000, currBank >> 4);
          writeByte_GB(0x2000, 0x10 | (currBank & 0x1f));
      }
    }

    // Set ROM bank for MBC6
    else if (romType == 32) {
      writeByte_GB(0x2800, 0);
      writeByte_GB(0x3800, 0);
      writeByte_GB(0x2000, currBank);
      writeByte_GB(0x3000, currBank);
    }

    // Set ROM bank for TAMA5
    else if (romType == 0xFD) {
      writeByteSRAM_GB(0xA001, 0);
      writeByteSRAM_GB(0xA000, currBank & 0x0f);
      writeByteSRAM_GB(0xA001, 1);
      writeByteSRAM_GB(0xA000, (currBank >> 4) & 0x0f);
    }

    // Set ROM bank for MBC2/3/4/5
    else if (romType >= 5) {
      if (romType >= 11 && romType <= 13) {
        if ((currBank & 0x1f) == 0) {
          // reset MMM01
          PORTH &= ~(1 << 0);
          PORTH |= (1 << 0);

          // remap to higher 4Mbits ROM
          writeByte_GB(0x3fff, 0x20);
          writeByte_GB(0x5fff, 0x40);
          writeByte_GB(0x7fff, 0x01);
          writeByte_GB(0x1fff, 0x3a);
          writeByte_GB(0x1fff, 0x7a);

          // for every 4Mbits ROM, restart from 0x0000
          romAddress = 0x0000;
          currBank++;
        }
        else {
          writeByte_GB(0x6000, 0);
          writeByte_GB(0x2000, (currBank & 0x1f));
        }
      }
      else {
        if ((romType >= 0x19 && romType <= 0x1E) && (currBank == 0 || currBank == 256)) {
          writeByte_GB(0x3000, (currBank >> 8) & 0xFF);
        }
        writeByte_GB(0x2100, currBank & 0xFF);
      }
    }
    // Set ROM bank for MBC1
    else {
      writeByte_GB(0x6000, 0);
      writeByte_GB(0x4000, currBank >> 5);
      writeByte_GB(0x2000, currBank & 0x1F);
    }

    // Read banks and save to SD
    while (romAddress <= endAddress) {
      for (int i = 0; i < 512; i++) {
        sdBuffer[i] = readByte_GB(romAddress + i);
      }
      myFile.write(sdBuffer, 512);
      romAddress += 512;
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
  }

  // Close the file:
  myFile.close();
}

// Calculate checksum
unsigned int calc_checksum_GB (char* fileName, char* folder) {
  unsigned int calcChecksum = 0;
  //  int calcFilesize = 0; // unused
  unsigned long i = 0;
  int c = 0;

  // If file exists
  if (myFile.open(fileName, O_READ)) {
    //calcFilesize = myFile.fileSize() * 8 / 1024 / 1024; // unused
    for (i = 0; i < (myFile.fileSize() / 512); i++) {
      myFile.read(sdBuffer, 512);
      for (c = 0; c < 512; c++) {
        calcChecksum += sdBuffer[c];
      }
    }
    myFile.close();
    // Subtract checksum bytes
    calcChecksum -= eepbit[6];
    calcChecksum -= eepbit[7];

    // Return result
    return (calcChecksum);
  }
  // Else show error
  else {
    print_Error(F("DUMP ROM 1ST"), false);
    return 0;
  }
}

// Compare checksum
void compare_checksums_GB() {
  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // last used rom folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern - 1);

  if (strcmp(folder, "root") != 0)
    sd.chdir(folder);

  // Internal ROM checksum
  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum_GB(fileName, folder));

  print_Msg(F("Checksum: "));
  print_Msg(calcsumStr);
  if (strcmp(calcsumStr, checksumStr) == 0) {
    println_Msg(F(" -> OK"));
  }
  else {
    print_Msg(F(" != "));
    println_Msg(checksumStr);
    print_Error(F("Invalid Checksum"), false);
  }
  compareCRC("gb.txt", 0, 1, 0);
  display_Update();
  //go to root
  sd.chdir();
}

/******************************************
  SRAM functions
*****************************************/
// Read RAM
void readSRAM_GB() {
  // Does cartridge have RAM
  if (lastByte > 0) {

    // Get name, add extension and convert to char array for sd lib
    strcpy(fileName, romName);
    strcat(fileName, ".sav");

    // create a new folder for the save file
    EEPROM_readAnything(0, foldern);
    sprintf(folder, "GB/SAVE/%s/%d", romName, foldern);
    sd.mkdir(folder, true);
    sd.chdir(folder);

    // write new folder number back to eeprom
    foldern = foldern + 1;
    EEPROM_writeAnything(0, foldern);

    //open file on sd card
    if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
      print_Error(F("SD Error"), true);
    }

    // MBC2 Fix
    readByte_GB(0x0134);

    if (romType <= 4 || (romType >= 11 && romType <= 13)) {
      writeByte_GB(0x6000, 1);
    }

    // Initialise MBC
    writeByte_GB(0x0000, 0x0A);

    // Switch SRAM banks
    for (byte currBank = 0; currBank < sramBanks; currBank++) {
      writeByte_GB(0x4000, currBank);

      // Read SRAM
      for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress += 64) {
        for (byte i = 0; i < 64; i++) {
          sdBuffer[i] = readByteSRAM_GB(sramAddress + i);
        }
        myFile.write(sdBuffer, 64);
      }
    }

    // Disable SRAM
    writeByte_GB(0x0000, 0x00);

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
  if (lastByte > 0) {
    // Create filepath
    sprintf(filePath, "%s/%s", filePath, fileName);

    //open file on sd card
    if (myFile.open(filePath, O_READ)) {
      // MBC2 Fix
      readByte_GB(0x0134);

      // Enable SRAM for MBC1
      if (romType <= 4 || (romType >= 11 && romType <= 13)) {
        writeByte_GB(0x6000, 1);
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch RAM banks
      for (byte currBank = 0; currBank < sramBanks; currBank++) {
        writeByte_GB(0x4000, currBank);

        // Write RAM
        for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress++) {
          writeByteSRAM_GB(sramAddress, myFile.read());
        }
      }
      // Disable SRAM
      writeByte_GB(0x0000, 0x00);

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

    // MBC2 Fix
    readByte_GB(0x0134);

    // Check SRAM size
    if (lastByte > 0) {
      if (romType <= 4) { // MBC1
        writeByte_GB(0x6000, 1); // Set RAM Mode
      }

      // Initialise MBC
      writeByte_GB(0x0000, 0x0A);

      // Switch SRAM banks
      for (byte currBank = 0; currBank < sramBanks; currBank++) {
        writeByte_GB(0x4000, currBank);

        // Read SRAM
        for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress += 64) {
          //fill sdBuffer
          myFile.read(sdBuffer, 64);
          for (int c = 0; c < 64; c++) {
            if (readByteSRAM_GB(sramAddress + c) != sdBuffer[c]) {
              writeErrors++;
            }
          }
        }
      }
      // Disable RAM
      writeByte_GB(0x0000, 0x00);
    }
    // Close the file:
    myFile.close();
    return writeErrors;
  }
  else {
    print_Error(F("Can't open file"), true);
  }
}

// Read SRAM + FLASH save data of MBC6
void readSRAMFLASH_MBC6_GB() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".sav");

  // create a new folder for the save file
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GB/SAVE/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

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
    print_Error(F("SD Error"), true);
  }

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = 0x108000;
  draw_progressbar(0, totalProgressBar);
  
  // Enable Mapper and SRAM
  writeByte_GB(0x0000, 0x0A);

  // Switch SRAM banks
  for (byte currBank = 0; currBank < sramBanks; currBank++) {
    writeByte_GB(0x0400, currBank);
    writeByte_GB(0x0800, currBank);

    // Read SRAM
    for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress += 64) {
      for (byte i = 0; i < 64; i++) {
        sdBuffer[i] = readByteSRAM_GB(sramAddress + i);
      }
      myFile.write(sdBuffer, 64);
      processedProgressBar += 64;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
  }

  // Disable SRAM
  writeByte_GB(0x0000, 0x00);

  // Enable flash save memory (map to ROM)
  writeByte_GB(0x1000, 0x01);
  writeByte_GB(0x0C00, 0x01);
  writeByte_GB(0x1000, 0x00);
  writeByte_GB(0x2800, 0x08);
  writeByte_GB(0x3800, 0x08);

  // Switch FLASH banks
  for (byte currBank = 0; currBank < 128; currBank++) {
    word romAddress = 0x4000;
    
    writeByte_GB(0x2000, currBank);
    writeByte_GB(0x3000, currBank);

    // Read banks and save to SD
    while (romAddress <= 0x5FFF) {
      for (int i = 0; i < 512; i++) {
        sdBuffer[i] = readByte_GB(romAddress + i);
      }
      myFile.write(sdBuffer, 512);
      romAddress += 512;
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
  }

  // Disable flash save memory
  writeByte_GB(0x1000, 0x01);
  writeByte_GB(0x0C00, 0x00);
  writeByte_GB(0x1000, 0x00);
  writeByte_GB(0x2800, 0x00);
  writeByte_GB(0x3800, 0x00);
  
  // Close the file:
  myFile.close();

  // Signal end of process
  println_Msg(F("OK"));
  display_Update();
}

// Write RAM
void writeSRAMFLASH_MBC6_GB() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);

  //open file on sd card
  if (myFile.open(filePath, O_READ)) {
    display_Clear();
    println_Msg(F("Writing MBC6 save..."));
    display_Update();
    
    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = 0x108000;
    draw_progressbar(0, totalProgressBar);

    // Enable Mapper and SRAM
    writeByte_GB(0x0000, 0x0A);
  
    // Switch SRAM banks
    for (byte currBank = 0; currBank < sramBanks; currBank++) {
      writeByte_GB(0x0400, currBank);
      writeByte_GB(0x0800, currBank);

      // Write SRAM
      for (word sramAddress = 0xA000; sramAddress <= lastByte; sramAddress++) {
        writeByteSRAM_GB(sramAddress, myFile.read());
      }

      processedProgressBar += (lastByte + 1) - 0xA000;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    
    // Disable SRAM
    writeByte_GB(0x0000, 0x00);
  
    // Enable flash save memory (map to ROM)
    writeByte_GB(0x1000, 0x01);
    writeByte_GB(0x0C00, 0x01);
    writeByte_GB(0x1000, 0x01);
    writeByte_GB(0x2800, 0x08);
    writeByte_GB(0x3800, 0x08);
    
    for (byte currBank = 0; currBank < 128; currBank++) {
      word romAddress = 0x4000;
      
      // Erase FLASH sector
      if (((processedProgressBar - 0x8000) % 0x20000) == 0) {
        writeByte_GB(0x2800, 0x08);
        writeByte_GB(0x3800, 0x08);
        writeByte_GB(0x2000, 0x01);
        writeByte_GB(0x3000, 0x02);
        writeByte_GB(0x7555, 0xAA);
        writeByte_GB(0x4AAA, 0x55);
        writeByte_GB(0x7555, 0x80);
        writeByte_GB(0x7555, 0xAA);
        writeByte_GB(0x4AAA, 0x55);
        writeByte_GB(0x2800, 0x08);
        writeByte_GB(0x3800, 0x08);
        writeByte_GB(0x2000, currBank);
        writeByte_GB(0x3000, currBank);
        writeByte_GB(0x4000, 0x30);
        byte lives = 100;
        while (1) {
          byte sr = readByte_GB(0x4000);
          if (sr == 0x80) break;
          delay(1);
          if (lives-- <= 0) {
            // Disable flash save memory
            writeByte_GB(0x1000, 0x01);
            writeByte_GB(0x0C00, 0x00);
            writeByte_GB(0x1000, 0x00);
            writeByte_GB(0x2800, 0x00);
            writeByte_GB(0x3800, 0x00);
            myFile.close();
            display_Clear();
            print_Error(F("Error erasing FLASH sector."), true);
          }
        }
      }
      else {
        writeByte_GB(0x2800, 0x08);
        writeByte_GB(0x3800, 0x08);
        writeByte_GB(0x2000, currBank);
        writeByte_GB(0x3000, currBank);
      }
      
      // Write to FLASH
      while (romAddress <= 0x5FFF) {
        writeByte_GB(0x2000, 0x01);
        writeByte_GB(0x3000, 0x02);
        writeByte_GB(0x7555, 0xAA);
        writeByte_GB(0x4AAA, 0x55);
        writeByte_GB(0x7555, 0xA0);
        writeByte_GB(0x2800, 0x08);
        writeByte_GB(0x3800, 0x08);
        writeByte_GB(0x2000, currBank);
        writeByte_GB(0x3000, currBank);
        for (int i = 0; i < 128; i++) {
          writeByte_GB(romAddress++, myFile.read());
        }
        writeByte_GB(romAddress - 1, 0x00);
        byte lives = 100;
        while (1) {
          byte sr = readByte_GB(romAddress - 1);
          if (sr == 0x80) break;
          delay(1);
          if (lives-- <= 0) {
            // Disable flash save memory
            writeByte_GB(0x1000, 0x01);
            writeByte_GB(0x0C00, 0x00);
            writeByte_GB(0x1000, 0x00);
            writeByte_GB(0x2800, 0x00);
            writeByte_GB(0x3800, 0x00);
            myFile.close();
            display_Clear();
            print_Error(F("Error writing to FLASH."), true);
          }
        }
        writeByte_GB(romAddress - 1, 0xF0);
        processedProgressBar += 128;
        draw_progressbar(processedProgressBar, totalProgressBar);
      }
    }

    // Disable flash save memory
    writeByte_GB(0x1000, 0x01);
    writeByte_GB(0x0C00, 0x00);
    writeByte_GB(0x1000, 0x00);
    writeByte_GB(0x2800, 0x00);
    writeByte_GB(0x3800, 0x00);

    // Close the file:
    myFile.close();
    println_Msg(F("Save writing finished"));
    display_Update();
  }
  else {
    print_Error(F("File doesnt exist"), false);
  }
}

/******************************************
  29F016/29F032/29F033 flashrom functions
*****************************************/
// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
void writeFlash29F_GB(byte MBC, boolean flashErase) {
  // Launch filebrowser
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select file"));
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
    switch (romSize) {
      case 0x00:
        romBanks = 2;
        break;
      case 0x01:
        romBanks = 4;
        break;
      case 0x02:
        romBanks = 8;
        break;
      case 0x03:
        romBanks = 16;
        break;
      case 0x04:
        romBanks = 32;
        break;
      case 0x05:
        romBanks = 64;
        break;
      case 0x06:
        romBanks = 128;
        break;
      case 0x07:
        romBanks = 256;
        break;
      default:
        romBanks = 2;
    }

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
    else if (strcmp(flashid, "04AD") == 0) {
      println_Msg(F("AM29F016D"));
      print_Msg(F("Banks: "));
      print_Msg(romBanks);
      println_Msg(F("/128"));
      display_Update();
    }
    else if (strcmp(flashid, "01D5") == 0) {
      println_Msg(F("AM29F080B"));
      print_Msg(F("Banks: "));
      print_Msg(romBanks);
      println_Msg(F("/64"));
      display_Update();
    }
    else {
      print_Msg(F("Flash ID: "));
      println_Msg(flashid);
      display_Update();
      print_Error(F("Unknown flashrom"), true);
    }

    // Reset flash
    writeByte_GB(0x555, 0xf0);

    delay(100);

    if (flashErase) {
      println_Msg(F("Erasing flash"));
      display_Update();

      // Erase flash
      writeByte_GB(0x555, 0xaa);
      writeByte_GB(0x2aa, 0x55);
      writeByte_GB(0x555, 0x80);
      writeByte_GB(0x555, 0xaa);
      writeByte_GB(0x2aa, 0x55);
      writeByte_GB(0x555, 0x10);

      // Read the status register
      byte statusReg = readByte_GB(0);
      // After a completed erase D7 will output 1
      while ((statusReg & 0x80) != 0x80) {
        // Update Status
        statusReg = readByte_GB(0);
      }

      // Blankcheck
      println_Msg(F("Blankcheck"));
      display_Update();

      // Read x number of banks
      for (int currBank = 0; currBank < romBanks; currBank++) {
        // Blink led
        blinkLED();

        // Set ROM bank
        writeByte_GB(0x2000, currBank);

        for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
          for (int currByte = 0; currByte < 512; currByte++) {
            sdBuffer[currByte] = readByte_GB(currAddr + currByte);
          }
          for (int j = 0; j < 512; j++) {
            if (sdBuffer[j] != 0xFF) {
              println_Msg(F("Not empty"));
              print_Error(F("Erase failed"), true);
            }
          }
        }
      }
    }

    if (MBC == 3) {
      println_Msg(F("Writing flash MBC3"));
      display_Update();

      // Write flash
      word currAddr = 0;
      word endAddr = 0x3FFF;

      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
      draw_progressbar(0, totalProgressBar);

      for (int currBank = 0; currBank < romBanks; currBank++) {
        // Blink led
        blinkLED();

        // Set ROM bank
        writeByte_GB(0x2100, currBank);

        if (currBank > 0) {
          currAddr = 0x4000;
          endAddr = 0x7FFF;
        }

        while (currAddr <= endAddr) {
          myFile.read(sdBuffer, 512);

          for (int currByte = 0; currByte < 512; currByte++) {
            // Write command sequence
            writeByte_GB(0x555, 0xaa);
            writeByte_GB(0x2aa, 0x55);
            writeByte_GB(0x555, 0xa0);
            // Write current byte
            writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

            // Set OE/RD(PH6) LOW
            PORTH &= ~(1 << 6);

            // Busy check
            while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
            }

            // Switch OE/RD(PH6) to HIGH
            PORTH |= (1 << 6);
          }
          currAddr += 512;
          processedProgressBar += 512;
          draw_progressbar(processedProgressBar, totalProgressBar);
        }
      }
    }

    else if (MBC == 5) {
      println_Msg(F("Writing flash MBC5"));
      display_Update();

      // Write flash
      //Initialize progress bar
      uint32_t processedProgressBar = 0;
      uint32_t totalProgressBar = (uint32_t)(romBanks) * 16384;
      draw_progressbar(0, totalProgressBar);

      for (int currBank = 0; currBank < romBanks; currBank++) {
        // Blink led
        blinkLED();

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

            // Set OE/RD(PH6) LOW
            PORTH &= ~(1 << 6);

            // Busy check
            while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
            }

            // Switch OE/RD(PH6) to HIGH
            PORTH |= (1 << 6);
          }
          processedProgressBar += 512;
          draw_progressbar(processedProgressBar, totalProgressBar);
        }
      }
    }

    print_Msg(F("Verifying..."));
    display_Update();

    // Go back to file beginning
    myFile.seekSet(0);
    //unsigned int addr = 0;  // unused
    writeErrors = 0;

    // Verify flashrom
    word romAddress = 0;

    // Read number of banks and switch banks
    for (word bank = 1; bank < romBanks; bank++) {
      if (romType >= 5) { // MBC2 and above
        writeByte_GB(0x2100, bank); // Set ROM bank
      }
      else { // MBC1
        writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }

      if (bank > 1) {
        romAddress = 0x4000;
      }
      // Blink led
      blinkLED();

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
      println_Msg(F("Error"));
      print_Msg(writeErrors);
      println_Msg(F(" bytes "));
      print_Error(F("did not verify."), true);
    }
  }
  else {
    println_Msg(F("Can't open file"));
    display_Update();
  }
}

/******************************************
  CFU flashrom functions
*****************************************/

/*
   Flash chips can either be in x8 mode or x16 mode and sometimes the two
   least significant bits on flash cartridges' data lines are swapped.
   This function reads a byte and compensates for the differences.
   This is only necessary for commands to the flash, not for data read from the flash, the MBC or SRAM.

   address needs to be the x8 mode address of the flash register that should be read.
*/
byte readByteCompensated(int address) {
  byte data = readByte_GB(address >> (flashX16Mode ? 1 : 0));
  if (flashSwitchLastBits) {
    return (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  return data;
}

/*
   Flash chips can either be in x8 mode or x16 mode and sometimes the two
   least significant bits on flash cartridges' data lines are swapped.
   This function writes a byte and compensates for the differences.
   This is only necessary for commands to the flash, not for data written to the flash, the MBC or SRAM.
   .
   address needs to be the x8 mode address of the flash register that should be read.
*/
byte writeByteCompensated(int address, byte data) {
  if (flashSwitchLastBits) {
    data = (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  writeByte_GB(address >> (flashX16Mode ? 1 : 0), data);
}

void startCFIMode(boolean x16Mode) {
  if (x16Mode) {
    writeByte_GB(0x555, 0xf0); //x16 mode reset command
    delay(500);
    writeByte_GB(0x555, 0xf0); //Double reset to get out of possible Autoselect + CFI mode
    delay(500);
    writeByte_GB(0x55, 0x98);  //x16 CFI Query command
  } else {
    writeByte_GB(0xAAA, 0xf0); //x8  mode reset command
    delay(100);
    writeByte_GB(0xAAA, 0xf0); //Double reset to get out of possible Autoselect + CFI mode
    delay(100);
    writeByte_GB(0xAA, 0x98);  //x8 CFI Query command
  }
}

/* Identify the different flash chips.
   Sets the global variables flashBanks, flashX16Mode and flashSwitchLastBits
*/
void identifyCFI_GB() {
  // Reset flash
  display_Clear();
  writeByte_GB(0x6000, 0); // Set ROM Mode
  writeByte_GB(0x2000, 0); // Set Bank to 0
  writeByte_GB(0x3000, 0);

  startCFIMode(false); // Trying x8 mode first

  display_Clear();
  // Try x8 mode first
  char cfiQRYx8[7];
  char cfiQRYx16[7];
  sprintf(cfiQRYx8, "%02X%02X%02X", readByte_GB(0x20), readByte_GB(0x22), readByte_GB(0x24));
  sprintf(cfiQRYx16, "%02X%02X%02X", readByte_GB(0x10), readByte_GB(0x11), readByte_GB(0x12)); // some devices use x8-style CFI Query command even though they are in x16 command mode
  if (strcmp(cfiQRYx8, "515259") == 0) { // QRY in x8 mode
    println_Msg(F("Normal CFI x8 Mode"));
    flashX16Mode = false;
    flashSwitchLastBits = false;
  } else if (strcmp(cfiQRYx8, "52515A") == 0) { // QRY in x8 mode with switched last bit
    println_Msg(F("Switched CFI x8 Mode"));
    flashX16Mode = false;
    flashSwitchLastBits = true;
  } else if (strcmp(cfiQRYx16, "515259") == 0) { // QRY in x16 mode
    println_Msg(F("Normal CFI x16 Mode"));
    flashX16Mode = true;
    flashSwitchLastBits = false;
  } else if (strcmp(cfiQRYx16, "52515A") == 0) { // QRY in x16 mode with switched last bit
    println_Msg(F("Switched CFI x16 Mode"));
    flashX16Mode = true;
    flashSwitchLastBits = true;
  } else {
    startCFIMode(true); // Try x16 mode next
    sprintf(cfiQRYx16, "%02X%02X%02X", readByte_GB(0x10), readByte_GB(0x11), readByte_GB(0x12));
    if (strcmp(cfiQRYx16, "515259") == 0) { // QRY in x16 mode
      println_Msg(F("Normal CFI x16 Mode"));
      flashX16Mode = true;
      flashSwitchLastBits = false;
    } else if (strcmp(cfiQRYx16, "52515A") == 0) { // QRY in x16 mode with switched last bit
      println_Msg(F("Switched CFI x16 Mode"));
      flashX16Mode = true;
      flashSwitchLastBits = true;
    } else {
      println_Msg(F("CFI Query failed!"));
      display_Update();
      wait();
      return;
    }
  }
  flashBanks = 1 << (readByteCompensated(0x4E) - 14); // - flashX16Mode);

  // Reset flash
  writeByteCompensated(0xAAA, 0xf0);
  delay(100);
}

// Write 29F032 flashrom
// A0-A13 directly connected to cart edge -> 16384(0x0-0x3FFF) bytes per bank -> 256(0x0-0xFF) banks
// A14-A21 connected to MBC5
// identifyFlash_GB() needs to be run before this!
bool writeCFI_GB() {
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
    switch (romSize) {
      case 0x00:
        romBanks = 2;
        break;
      case 0x01:
        romBanks = 4;
        break;
      case 0x02:
        romBanks = 8;
        break;
      case 0x03:
        romBanks = 16;
        break;
      case 0x04:
        romBanks = 32;
        break;
      case 0x05:
        romBanks = 64;
        break;
      case 0x06:
        romBanks = 128;
        break;
      case 0x07:
        romBanks = 256;
        break;
      default:
        romBanks = 2;
    }

    if (romBanks <= flashBanks) {
      print_Msg(F("Using "));
      print_Msg(romBanks);
      print_Msg(F("/"));
      print_Msg(flashBanks);
      println_Msg(F(" Banks"));
      display_Update();
    } else {
      println_Msg(F("Error: Flash has too few banks!"));
      print_Msg(F("Has "));
      print_Msg(flashBanks);
      println_Msg(F(" banks,"));
      print_Msg(F("but needs "));
      print_Msg(romBanks);
      println_Msg(F("."));
      println_Msg(F("Press Button..."));
      display_Update();
      wait();
      resetArduino();
    }

    // Set ROM bank hi 0
    writeByte_GB(0x3000, 0);
    // Set ROM bank low 0
    writeByte_GB(0x2000, 0);
    delay(100);

    // Reset flash
    writeByteCompensated(0xAAA, 0xf0);
    delay(100);

    // Reset flash
    writeByte_GB(0x555, 0xf0);

    delay(100);
    println_Msg(F("Erasing flash"));
    display_Update();

    // Erase flash
    writeByteCompensated(0xAAA, 0xaa);
    writeByteCompensated(0x555, 0x55);
    writeByteCompensated(0xAAA, 0x80);
    writeByteCompensated(0xAAA, 0xaa);
    writeByteCompensated(0x555, 0x55);
    writeByteCompensated(0xAAA, 0x10);

    // Read the status register
    byte statusReg = readByte_GB(0);

    // After a completed erase D7 will output 1
    while ((statusReg & 0x80) != 0x80) {
      // Blink led
      blinkLED();
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
      blinkLED();

      // Set ROM bank
      writeByte_GB(0x2000, currBank);

      for (unsigned int currAddr = 0x4000; currAddr < 0x7FFF; currAddr += 512) {
        for (int currByte = 0; currByte < 512; currByte++) {
          sdBuffer[currByte] = readByte_GB(currAddr + currByte);
        }
        for (int j = 0; j < 512; j++) {
          if (sdBuffer[j] != 0xFF) {
            println_Msg(F("Not empty"));
            print_Error(F("Erase failed"), true);
          }
        }
      }
    }

    println_Msg(F("Writing flash MBC3/5"));
    display_Update();

    // Write flash
    word currAddr = 0;
    word endAddr = 0x3FFF;

    for (int currBank = 0; currBank < romBanks; currBank++) {
      // Blink led
      blinkLED();

      // Set ROM bank
      writeByte_GB(0x2100, currBank);
      // 0x2A8000 fix
      writeByte_GB(0x4000, 0x0);

      if (currBank > 0) {
        currAddr = 0x4000;
        endAddr = 0x7FFF;
      }

      while (currAddr <= endAddr) {
        myFile.read(sdBuffer, 512);

        for (int currByte = 0; currByte < 512; currByte++) {
          // Write command sequence
          writeByteCompensated(0xAAA, 0xaa);
          writeByteCompensated(0x555, 0x55);
          writeByteCompensated(0xAAA, 0xa0);

          // Write current byte
          writeByte_GB(currAddr + currByte, sdBuffer[currByte]);

          // Setting CS(PH3) and OE/RD(PH6) LOW
          PORTH &= ~((1 << 3) | (1 << 6));

          // Busy check
          short i = 0;
          while ((PINC & 0x80) != (sdBuffer[currByte] & 0x80)) {
            i++;
            if (i > 500) {
              if (currAddr < 0x4000) { // This happens when trying to flash an MBC5 as if it was an MBC3. Retry to flash as MBC5, starting from last successfull byte.
                currByte--;
                currAddr += 0x4000;
                endAddr = 0x7FFF;
                break;
              } else { // If a timeout happens while trying to flash MBC5-style, flashing failed.
                return false;
              }
            }
          }

          // Switch CS(PH3) and OE/RD(PH6) to HIGH
          PORTH |= (1 << 3) | (1 << 6);
          __asm__("nop\n\tnop\n\tnop\n\t"); // Waste a few CPU cycles to remove write errors

        }
        currAddr += 512;
      }
    }

    display_Clear();
    println_Msg(F("Verifying"));
    display_Update();

    // Go back to file beginning
    myFile.seekSet(0);
    //unsigned int addr = 0;  // unused
    writeErrors = 0;

    // Verify flashrom
    word romAddress = 0;

    // Read number of banks and switch banks
    for (word bank = 1; bank < romBanks; bank++) {
      if (romType >= 5) { // MBC2 and above
        writeByte_GB(0x2100, bank); // Set ROM bank
      }
      else { // MBC1
        writeByte_GB(0x6000, 0); // Set ROM Mode
        writeByte_GB(0x4000, bank >> 5); // Set bits 5 & 6 (01100000) of ROM bank
        writeByte_GB(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
      }

      if (bank > 1) {
        romAddress = 0x4000;
      }
      // Blink led
      blinkLED();

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
  return true;
}

#endif

//******************************************
// End of File
//******************************************
