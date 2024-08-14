//***********************************************************
// SEGA MASTER SYSTEM / MARK III / GAME GEAR / SG-1000 MODULE
//***********************************************************
#ifdef ENABLE_SMS

//******************************************
//   Menus
//******************************************
// Adapters menu
static const char SMSAdapterItem1[] PROGMEM = "SMS/MarkIII raphnet";
static const char SMSAdapterItem2[] PROGMEM = "SMS Retrode";
static const char SMSAdapterItem3[] PROGMEM = "SMS Retron3in1";
static const char SMSAdapterItem4[] PROGMEM = "GameGear Retrode";
static const char SMSAdapterItem5[] PROGMEM = "GameGear Retron3in1";
static const char SMSAdapterItem6[] PROGMEM = "SG-1000 raphnet";
static const char* const SMSAdapterMenu[] PROGMEM = { SMSAdapterItem1, SMSAdapterItem2, SMSAdapterItem3, SMSAdapterItem4, SMSAdapterItem5, SMSAdapterItem6 };

// Operations menu
static const char SMSOperationMenuItem4[] PROGMEM = "Set ROM Size";
static const char* const SMSOperationMenu[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, SMSOperationMenuItem4, FSTRING_RESET };

// SG Operations menu
static const char* const SGOperationsMenu[] PROGMEM = { FSTRING_READ_ROM, FSTRING_RESET };

// Rom sizes menu
static const char SMSRomSizeItem1[] PROGMEM = "8 KB";
static const char SMSRomSizeItem2[] PROGMEM = "16 KB";
static const char SMSRomSizeItem3[] PROGMEM = "24 KB";
static const char SMSRomSizeItem4[] PROGMEM = "32 KB";
static const char SMSRomSizeItem5[] PROGMEM = "40 KB";  //SG-1000 40k mapping not yet supported
static const char SMSRomSizeItem6[] PROGMEM = "48 KB";  //SG-1000 40k mapping not yet supported
static const char SMSRomSizeItem7[] PROGMEM = "64 KB";
static const char SMSRomSizeItem8[] PROGMEM = "128 KB";
static const char SMSRomSizeItem9[] PROGMEM = "256 KB";
static const char SMSRomSizeItem10[] PROGMEM = "512 KB";
static const char SMSRomSizeItem11[] PROGMEM = "1024 KB";
static const char* const SG1RomSizeMenu[] PROGMEM = { SMSRomSizeItem1, SMSRomSizeItem2, SMSRomSizeItem3, SMSRomSizeItem4 };                                      // Rom sizes for SG-1000
static const char* const SMSRomSizeMenu[] PROGMEM = { SMSRomSizeItem4, SMSRomSizeItem7, SMSRomSizeItem8, SMSRomSizeItem9, SMSRomSizeItem10, SMSRomSizeItem11 };  // Rom sizes for SMS and GG

// Init systems
static bool system_sms = false;     // SMS or MarkIII
static bool system_gg = false;      // GameGear
static bool system_sg1000 = false;  // SG-1000

// Init adapters
static bool adapter_raphnet = false;  // raphet adapater (SMS-to-MD or MIII-to-MD)
static bool adapter_retrode = false;  // Retrode adapter (SMS-to-MD or GG-to-MD)
static bool adapter_retron = false;   // Retron 3in1 adapter (SMS-to-MD or GG-to-MD)

// Manual ROM Size Selection Flag
bool manRomSizeSelected = false;

//*********************************************************
//  Main menu with systems/adapters setups to choose from
//*********************************************************
void smsMenu() {
  unsigned char SMSSetup;
  convertPgm(SMSAdapterMenu, 6);
  SMSSetup = question_box(F("Select your setup"), menuOptions, 6, 0);

  switch (SMSSetup) {
    case 0:
      // SMS or MarkIII with raphnet adapter
      system_sms = true;
      adapter_raphnet = true;
      break;
    case 1:
      // SMS with Retrode adapter
      system_sms = true;
      adapter_retrode = true;
      break;
    case 2:
      // SMS with Retron 3in1 adapter
      system_sms = true;
      adapter_retron = true;
      break;
    case 3:
      // GameGear with Retrode adapter
      system_gg = true;
      adapter_retrode = true;
      break;
    case 4:
      // GameGear with Retron 3in1 adapter
      system_gg = true;
      adapter_retron = true;
      break;
    case 5:
      // SG-1000 with raphnet adapter
      system_sg1000 = true;
      adapter_raphnet = true;
      break;
  }
  for (;;) operationsMenu();
}

//****************************************************
//  Create custom menu depending on selected setup
//****************************************************
void operationsMenu() {
  unsigned char SMSOperation = '3';

  if (system_sg1000) {
    convertPgm(SGOperationsMenu, 2);
    SMSOperation = question_box(FS(SMSAdapterItem6), menuOptions, 2, 0);
    switch (SMSOperation) {
      case 0: // Read ROM
        mode = CORE_SMS;
        setup_SMS();
        readROM_SMS();
        break;
      default:
      case 1:
        // Reset
        resetArduino();
        break;
    }
  } else {
    convertPgm(SMSOperationMenu, 5);
    if (system_sms) {
      if (adapter_raphnet) {
        SMSOperation = question_box(FS(SMSAdapterItem1), menuOptions, 5, 0);
      } else if (adapter_retrode) {
        SMSOperation = question_box(FS(SMSAdapterItem2), menuOptions, 5, 0);
      } else if (adapter_retron) {
        SMSOperation = question_box(FS(SMSAdapterItem3), menuOptions, 5, 0);
      }
    } else if (system_gg) {
      if (adapter_retrode) {
        SMSOperation = question_box(FS(SMSAdapterItem4), menuOptions, 5, 0);
      } else if (adapter_retron) {
        SMSOperation = question_box(FS(SMSAdapterItem5), menuOptions, 5, 0);
      }
    }

    switch (SMSOperation) {
      case 0: // Read ROM
        mode = CORE_SMS;
        setup_SMS();
        readROM_SMS();
        break;
      case 1: // Read SRAM
        mode = CORE_SMS;
        setup_SMS();
        readSRAM_SMS();
        break;
      case 2: // Write SRAM
        mode = CORE_SMS;
        setup_SMS();
        writeSRAM_SMS();
        break;
      case 3: // Select ROM Size
        manual_selectRomSize();
        break;
      default:
      case 4:
        // Reset
        resetArduino();
        break;
    }
  }

  display_Update();
  wait();
}

//********************************
//   Setup I/O
//********************************
void setup_SMS() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A14
  DDRK = 0xFF;
  //A15
  DDRH |= (1 << 3);

  //For Retrode adapter
  if (adapter_retrode) {
    PORTH &= ~((1 << 0) | (1 << 3) | (1 << 5));
    PORTL &= ~(1 << 1);
    DDRH &= ~((1 << 0) | (1 << 5));
    DDRL &= ~((1 << 1));
    // Set Control Pins to Output OE(PH6)
    DDRH |= (1 << 6);
    // WR(PL5) and RD(PL6)
    DDRL |= (1 << 5) | (1 << 6);
    // Setting OE(PH6) HIGH
    PORTH |= (1 << 6);
    // Setting WR(PL5) and RD(PL6) HIGH
    PORTL |= (1 << 5) | (1 << 6);
  }

  // For Raphnet and Retron adapters
  else {
    // Set Control Pins to Output RST(PH0) WR(PH5) OE(PH6)
    DDRH |= (1 << 0) | (1 << 5) | (1 << 6);
    // CE(PL1)
    DDRL |= (1 << 1);
    // Setting RST(PH0) WR(PH5) OE(PH6) HIGH
    PORTH |= (1 << 0) | (1 << 5) | (1 << 6);
    // CE(PL1)
    PORTL |= (1 << 1);
  }

  if (!system_sg1000) {
    // SMS/GG ROM has 16KB banks which can be mapped to one of three slots via register writes
    // Register Slot Address space
    // $fffd    0    $0000-$3fff
    // $fffe    1    $4000-$7fff
    // $ffff    2    $8000-$bfff

    // Disable sram
    writeByte_SMS(0xFFFC, 0);

    // Map first 3 banks so we can read-out the header info
    writeByte_SMS(0xFFFD, 0);
    writeByte_SMS(0xFFFE, 1);
    writeByte_SMS(0xFFFF, 2);
  }

  delay(400);

  // Read and print cart info only if ROM size not manually selected
  if (manRomSizeSelected == false) {
    getCartInfo_SMS();
  }
}

//*****************************************
//  Low level functions
//*****************************************
void writeByte_SMS(word myAddress, byte myData) {
  if (adapter_retrode && system_gg) {
    // Set Data Pins (D8-D15) to Output
    DDRA = 0xFF;
  } else {
    // Set Data Pins (D0-D7) to Output
    DDRC = 0xFF;
  }

  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  if (!adapter_retrode) {
    // A15/M0-7(PH3) and OE(PH6) are connected
    PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
  }

  // Output data
  if (adapter_retrode && system_gg) {
    PORTA = myData;
  } else {
    PORTC = myData;
  }

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  if (adapter_retrode) {
    // Switch WR(PL5) and OE/CE(PH6) to LOW
    PORTL &= ~(1 << 5);
    PORTH &= ~(1 << 6);
  } else {
    // Switch CE(PL1) and WR(PH5) to LOW
    PORTL &= ~(1 << 1);
    PORTH &= ~(1 << 5);
  }

  // Leave WR low for at least 60ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  if (adapter_retrode) {
    // Switch WR(PL5) and OE/CE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 5);
  } else {
    // Switch CE(PL1) and WR(PH5) to HIGH
    PORTH |= (1 << 5);
    PORTL |= (1 << 1);
  }

  // Leave WR high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  if (adapter_retrode && system_gg) {
    // Set Data Pins (D8-D15) to Input
    DDRA = 0x00;
  } else {
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
  }
}

byte readByte_SMS(word myAddress) {
  if (adapter_retrode && system_gg) {
    // Set Data Pins (D8-D15) to Input
    DDRA = 0x00;
  } else {
    // Set Data Pins (D0-D7) to Input
    DDRC = 0x00;
  }

  // Set Address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  if (!adapter_retrode) {
    // A15/M0-7(PH3) and OE(PH6) are connected
    PORTH = (PORTH & 0b11110111) | ((myAddress >> 12) & 0b00001000);
  }

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  if (adapter_retrode) {
    // Switch RD(PL6) and OE(PH6) to LOW
    PORTL &= ~(1 << 6);
    PORTH &= ~(1 << 6);
  } else {
    // Switch CE(PL1) and OE(PH6) to LOW
    PORTL &= ~(1 << 1);
    PORTH &= ~(1 << 6);
  }

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = (adapter_retrode && system_gg) ? PINA : PINC;

  if (adapter_retrode) {
    // Switch RD(PL6) and OE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 6);
  } else {
    // Switch CE(PL1) and OE(PH6) to HIGH
    PORTH |= (1 << 6);
    PORTL |= (1 << 1);
  }

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

byte readNibble(byte data, byte number) {
  return ((data >> (number * 4)) & 0xF);
}

//*****************************************
//  Cartridges functions
//*****************************************
void getCartInfo_SMS() {
  byte cartNib = readNibble(readByte_SMS(0x7FFF), 0);

  // Get rom size from header
  //    Note: Common for this value to be smaller than the actual value.
  //          Normally used for BIOS checksum calculations on export hardware (non JP).
  switch (cartNib) {
    case 0xa:
      cartSize = 8192; // 8 KiB
      break;
    case 0xb:
      cartSize = 16384; // 16 KiB
      break;
    case 0xc:
      cartSize = 32768; // 32 KiB
      break;
    case 0xd:
      cartSize = 49152; // 48 KiB
      break;
    case 0xe:
      cartSize = 65536; // 64 KiB
      break;
    case 0xf:
      cartSize = 131072; // 128 KiB
      break;
    case 0x0:
      cartSize = 262144; // 256 KiB
      break;
    case 0x1:
    case 0x2:
      cartSize = 524288; // 512 KiB
      break;
    case 0x3:
      // 0x3 is (only?) used in The Pro Yakyuu '91 (Game Gear)
      cartSize = 131072; // 128 KiB
      break;
    default:
      cartSize = 49152; // 48 KiB
      // LED Error
      rgbLed(blue_color);
      break;
  }

  // Get rom name (expecting "TMR SEGA" string)
  for (byte i = 0; i < 8; i++) {
    romName[i] = char(readByte_SMS(0x7FF0 + i));
  }
  romName[8] = '\0';

  // Attempt to detect cart size by checking if "TMR SEGA" is mirrored
  // Based on https://www.raphnet.net/electronique/sms_cartridge_programmer/index_en.php#6
  // Note: Logic does not work on US CloudMaster (256K) and Penquin Land (128K) SMS carts.
  //       Both detect as 512 KB based on the logic below.
  if (strcmp(romName, "TMR SEGA") == 0) {
    byte bank = 1;
    char romNameBuf[9];
    while (bank < 64) { // Total number of possible banks: 1 MiB / 16 (KiB/Bank)
      bank++; // Increment the bank
      writeByte_SMS(0xfffe, bank); // Load bank into slot 1
      for (byte i = 0; i < 8; i++) {
        romNameBuf[i] = char(readByte_SMS(0x7FF0 + i));
      }
      romNameBuf[8] = '\0';

      // Debug info:
      // print_Msg(F("Bank: "));
      // println_Msg(bank);
      // print_Msg(F("Parsed ROM name: "));
      // println_Msg(romNameBuf);
      // println_Msg(FS(FSTRING_EMPTY));

      if (strcmp(romNameBuf, romName) == 0) {
        break;
      }
    }
    if (bank > 2) { // 32 KiB is the smallest SMS/GG ROM size
      cartSize = (bank - 1) * 16384UL;
    }

    // Debug info:
    // print_Msg(F("Calculated ROM Size: "));
    // println_Msg(cartSize);

    // Reset Bank Slot 1
    writeByte_SMS(0xFFFE, 1);

    // Display header info
    display_Clear();
    if (system_sms) {
      println_Msg(F("SMS Header:"));
    } else {
      println_Msg(F("GG Header:"));
    }
  } else { // romName != "TMR SEGA"
    // Fix for "Fantasy Zone (J) (V1.0)" that has not the normal header, but "COPYRIGHT SEGAPRG. BY T.ASAI".
    char headerFZ[29];
    if (strcmp(romName, "G. BY T.A") != 0) {
      for (byte i = 0; i < 28; i++) {
        headerFZ[i] = char(readByte_SMS(0x7FE0 + i));
      }
      headerFZ[28] = '\0';

      if (strcmp(headerFZ, "COPYRIGHT SEGAPRG. BY T.ASAI") == 0) {
        strcpy(romName, "TMR SEGA");
        cartSize = 131072; // 128 KiB
      }
    }

    manual_selectRomSize();

    // Display cart info
    display_Clear();
    println_Msg(F("SMS/GG header not found"));
  }

  println_Msg(FS(FSTRING_SPACE));
  print_Msg(FS(FSTRING_NAME));
  println_Msg(romName);
  print_Msg(cartSize / 1024);
  println_Msg(F("KB"));
  println_Msg(FS(FSTRING_SPACE));

// Wait for user input
#if (defined(ENABLE_LCD) || defined(ENABLE_OLED))
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif

  // Turn off LED
  rgbLed(black_color);
}

void manual_selectRomSize() {
  // Set rom size manually
  unsigned char SMSRomSize;

  if (system_sg1000) {
    // Rom sizes for SG-1000
    convertPgm(SG1RomSizeMenu, 4);
    SMSRomSize = question_box(F("Select ROM size"), menuOptions, 4, 0);
    switch (SMSRomSize) {
      case 0:
        cartSize = 8192;   // 8 KiB
        break;
      case 1:
        cartSize = 16384;  // 16 KiB
        break;
      case 2:
        cartSize = 24576;  // 24 KiB
        break;
      case 3:
        cartSize = 32768;  // 32 KiB
        break;
        //case 4:
        //  cartSize = 40960; // 40KB
        //  break;
        //case 5:
        //  cartSize = 49152; // 48KB
        //  break;
    }
  } else {
    // Rom sizes for SMS and GG
    convertPgm(SMSRomSizeMenu, 6);
    SMSRomSize = question_box(F("Select ROM size"), menuOptions, 6, 0);
    switch (SMSRomSize) {
      case 0:
        cartSize = 32768;    // 32 KiB
        break;
      case 1:
        cartSize = 65536;    // 64 KiB
        break;
      case 2:
        cartSize = 131072;   // 128 KiB
        break;
      case 3:
        cartSize = 262144;   // 256 KiB
        break;
      case 4:
        cartSize = 524288;   // 512 KiB
        break;
      case 5:
        cartSize = 1048576;  // 1 MiB
        break;
    }
  }
  strcpy(romName, "UNKNOWN"); // Use default UNKNOWN rom name upon manual selection
  manRomSizeSelected = true; // This ensures manually selected value is used upon read from the menu

  // Display info
  display_Clear();
  print_Msg(F("Selected Size: "));
  print_Msg(cartSize / 1024);
  println_Msg(F("KB"));

  print_STR(press_button_STR, 1);
}

//******************************************
//  Read ROM and save it to the SD card
//******************************************
void readROM_SMS() {
  sd.chdir("/");
  // Get name, add extension depending on the system and convert to char array for sd lib
  if (system_sms) {
    createFolder("SMS", "ROM", romName, "sms");
  } else if (system_gg) {
    createFolder("GG", "ROM", romName, "gg");
  } else {
    createFolder("SG1000", "ROM", romName, "sg");
  }
  printAndIncrementFolder(true);

  print_Msg(F("ROM Size: "));
  print_Msg(cartSize / 1024);
  println_Msg(F("KB"));

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }

  // Set default bank size to 16 KiB
  word bankSize = 16384;

  // For carts not using mappers (SG1000 or SMS/GG 32 KiB)
  if ((system_sg1000) || (cartSize == 32768)) {
    bankSize = cartSize;
  }

  // Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)(cartSize);
  draw_progressbar(0, totalProgressBar);

  for (byte currBank = 0x0; currBank < (cartSize / bankSize); currBank++) {
    // Write current 16KB bank to slot 2 register 0xFFFF
    if (!system_sg1000) {
      writeByte_SMS(0xFFFF, currBank);
    }

    // Blink led
    blinkLED();

    // Read 16KB from slot 2 which starts at 0x8000
    for (word currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(((system_sg1000) || (cartSize == 32768) ? 0 : 0x8000) + currBuffer + currByte);
      }
      // hexdump for debugging:
      // if (currBank == 0 && currBuffer == 0) {
      //   for (word xi = 0; xi < 0x100; xi++) {
      //     if (xi%16==0) {
      //       print_Msg_PaddedHex16(xi);
      //       print_Msg(FS(FSTRING_SPACE));
      //     }
      //     print_Msg_PaddedHexByte(sdBuffer[xi]);
      //     if (xi>0&&((xi+1)%16)==0) {
      //       println_Msg(FS(FSTRING_EMPTY));
      //     } else {
      //       print_Msg(FS(FSTRING_SPACE));
      //     }
      //   }
      // }
      myFile.write(sdBuffer, 512);
    }

    // Update progress bar
    processedProgressBar += bankSize;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

  // Close file
  myFile.close();

  // Compare dump checksum with database values
  if (system_sms) {
    compareCRC("sms.txt", 0, 1, 0);
  } else if (system_gg) {
    compareCRC("gg.txt", 0, 1, 0);
  } else {
    compareCRC("sg1000.txt", 0, 1, 0);
  }

#ifdef ENABLE_GLOBAL_LOG
  save_log();
#endif

  print_STR(press_button_STR, 1);
}

//******************************************
//  Read SRAM and save to the SD card
///*****************************************
void readSRAM_SMS() {
  // Get name, add extension and convert to char array for sd lib
  const char* system;

  if (system_gg) {
    system = "GG";
  } else {
    system = "SMS";
  }
  createFolderAndOpenFile(system, "SAVE", romName, "sav");

  // Write the whole 32 KiB
  // When there is only 8 KiB of SRAM, the contents should be duplicated
  word bankSize = 16384;
  for (byte currBank = 0x0; currBank < 2; currBank++) {
    writeByte_SMS(0xFFFC, 0x08 | (currBank << 2));

    // Blink led
    blinkLED();

    // Read 16 KiB from slot 2 which starts at 0x8000
    for (word currBuffer = 0; currBuffer < bankSize; currBuffer += 512) {
      // Fill SD buffer
      for (int currByte = 0; currByte < 512; currByte++) {
        sdBuffer[currByte] = readByte_SMS(0x8000 + currBuffer + currByte);
      }
      myFile.write(sdBuffer, 512);
    }
  }
  // Close file
  myFile.close();
  print_STR(press_button_STR, 1);
  display_Update();
}

//**********************************************
//  Read file from SD card and write it to SRAM
//**********************************************
void writeSRAM_SMS() {
  if (false) {
    print_Error(F("DISABLED"));
  } else {
    fileBrowser(FS(FSTRING_SELECT_FILE));

    sd.chdir();
    sprintf(filePath, "%s/%s", filePath, fileName);

    display_Clear();
    println_Msg(F("Restoring from "));
    println_Msg(filePath);
    display_Update();

    if (myFile.open(filePath, O_READ)) {
      // Get SRAM size from file, with a maximum of 32KB
      uint32_t sramSize = myFile.fileSize();
      if (sramSize > ((uint32_t)32 * (uint32_t)1024)) {
        sramSize = (uint32_t)32 * (uint32_t)1024;
      }
      print_Msg(F("sramSize: "));
      print_Msg(sramSize);
      println_Msg(FS(FSTRING_EMPTY));
      word bankSize = 16384;
      for (word address = 0x0; address < sramSize; address += 512) {
        byte currBank = address >= bankSize ? 1 : 0;
        word page_address = address - (currBank * bankSize);
        writeByte_SMS(0xFFFC, 0x08 | (currBank << 2));
        // Blink led
        blinkLED();
        myFile.read(sdBuffer, 512);
        for (int x = 0; x < 512; x++) {
          writeByte_SMS(0x8000 + page_address + x, sdBuffer[x]);
        }
      }

      // Close file
      myFile.close();
      print_STR(press_button_STR, 1);
      display_Update();
    } else {
      print_FatalError(sd_error_STR);
    }
  }
  sd.chdir();          // root
  filePath[0] = '\0';  // Reset filePath
}
#endif

//******************************************
// End of File
//******************************************
