//******************************************
// FLASHROM MODULE
// (also includes SNES repro functions)
//******************************************
#ifdef enable_FLASH

/******************************************
   Variables
 *****************************************/
// Flashrom
unsigned long flashSize;
byte flashromType;
byte secondID = 1;
unsigned long time;
unsigned long blank;
unsigned long sectorSize;
uint16_t bufferSize;
byte mapping = 1;

/******************************************
   Menu
 *****************************************/
// 8bit Flash menu items
static const char flash8MenuItem1[] PROGMEM = "Blankcheck";
static const char flash8MenuItem2[] PROGMEM = "Erase";
static const char flash8MenuItem3[] PROGMEM = "Read";
static const char flash8MenuItem4[] PROGMEM = "Write";
static const char flash8MenuItem5[] PROGMEM = "ID";
static const char flash8MenuItem6[] PROGMEM = "Print";
//static const char flash8MenuItem7[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsFLASH8[] PROGMEM = { flash8MenuItem1, flash8MenuItem2, flash8MenuItem3, flash8MenuItem4, flash8MenuItem5, flash8MenuItem6, string_reset2 };

#ifdef enable_FLASH16
// Flash start menu
static const char flashMenuItem1[] PROGMEM = "8bit Flash adapter";
static const char flashMenuItem2[] PROGMEM = "Eprom adapter";
static const char flashMenuItem3[] PROGMEM = "16bit Flash adapter";
// static const char flashMenuItem4[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsFlash[] PROGMEM = { flashMenuItem1, flashMenuItem2, flashMenuItem3, string_reset2 };

// 16bit Flash menu items
static const char flash16MenuItem1[] PROGMEM = "Blankcheck";
static const char flash16MenuItem2[] PROGMEM = "Erase";
static const char flash16MenuItem3[] PROGMEM = "Read";
static const char flash16MenuItem4[] PROGMEM = "Write";
static const char flash16MenuItem5[] PROGMEM = "ID";
static const char flash16MenuItem6[] PROGMEM = "Print";
//static const char flash16MenuItem7[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsFLASH16[] PROGMEM = { flash16MenuItem1, flash16MenuItem2, flash16MenuItem3, flash16MenuItem4, flash16MenuItem5, flash16MenuItem6, string_reset2 };

// Eprom menu items
static const char epromMenuItem1[] PROGMEM = "Blankcheck";
static const char epromMenuItem2[] PROGMEM = "Read";
static const char epromMenuItem3[] PROGMEM = "Write";
static const char epromMenuItem4[] PROGMEM = "Verify";
static const char epromMenuItem5[] PROGMEM = "Print";
// static const char epromMenuItem6[] PROGMEM = "Reset"; (stored in common strings array)
static const char* const menuOptionsEprom[] PROGMEM = { epromMenuItem1, epromMenuItem2, epromMenuItem3, epromMenuItem4, epromMenuItem5, string_reset2 };

void flashMenu() {
  // create menu with title and 3 options to choose from
  unsigned char flashSlot;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFlash, 4);
  flashSlot = question_box(F("Select adapter PCB"), menuOptions, 4, 0);

  // wait for user choice to come back from the question box menu
  switch (flashSlot) {
    case 0:
      display_Clear();
      display_Update();
      mapping = 1;
      setup_Flash8();
      id_Flash8();
      wait();
      mode = mode_FLASH8;
      break;

    case 1:
      display_Clear();
      display_Update();
      setup_Eprom();
      mode = mode_EPROM;
      break;

    case 2:
      display_Clear();
      display_Update();
      setup_Flash16();
      id_Flash16();
      wait();
      mode = mode_FLASH16;
      break;

    case 3:
      resetArduino();
      break;
  }
}
#endif

void flashromMenu8() {
  // create menu with title and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH8, 7);
  mainMenu = question_box(F("Flashrom Writer 8"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetFlash8();
      blankcheck_Flash();
      break;

    case 1:
      display_Clear();
      println_Msg(F("Erasing Flashrom"));
      println_Msg(F("Please wait..."));
      display_Update();
      time = millis();

      switch (flashromType) {
        case 1: eraseFlash29F032(); break;
        case 2: eraseFlash29F1610(); break;
        case 3: eraseFlash28FXXX(); break;
      }

      println_Msg(F("Flashrom erased"));
      display_Update();
      resetFlash8();
      break;

    case 2:
      time = millis();
      resetFlash8();
      readFlash();
      break;

    case 3:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser(F("Select file"));
      display_Clear();
      time = millis();

      switch (flashromType) {
        case 1:
          writeFlash29F032();
          break;
        case 2:
          if (flashid == 0xC2F3)
            writeFlash29F1601();
          else if ((flashid == 0xC2F1) || (flashid == 0xC2F9))
            writeFlash29F1610();
          else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB))
            writeFlash29LV640();
          else if (flashid == 0x017E) {
            // sector size, write buffer size
            writeFlash29GL(sectorSize, bufferSize);
          } else if ((flashid == 0x0458) || (flashid == 0x0158) || (flashid == 0x01AB))
            writeFlash29F800();

          break;
        case 3:
          writeFlash28FXXX();
          break;
      }

      delay(100);

      // Reset twice just to be sure
      resetFlash8();
      resetFlash8();

      verifyFlash();
      break;

    case 4:
      time = 0;
      display_Clear();
      resetFlash8();
      println_Msg(F("ID Flashrom"));
      switch (flashromType) {
        case 1: idFlash29F032(); break;
        case 2: idFlash29F1610(); break;
        case 3: idFlash28FXXX(); break;
      }

      println_Msg(F(""));
      printFlash(40);
      println_Msg(F(""));
      display_Update();

      resetFlash8();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetFlash8();
      printFlash(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetFlash8();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took : "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 0);
  display_Update();
  wait();
}

#ifdef enable_FLASH16
void flashromMenu16() {
  // create menu with title "Flashrom Writer 16" and 7 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFLASH16, 7);
  mainMenu = question_box(F("Flashrom Writer 16"), menuOptions, 7, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      resetFlash16();
      blankcheck16();
      break;

    case 1:
      display_Clear();
      println_Msg(F("Erase Flashrom"));
      display_Update();
      time = millis();
      resetFlash16();
      eraseFlash16();
      println_Msg(F("Flashrom erased."));
      display_Update();
      break;

    case 2:
      display_Clear();
      time = millis();
      resetFlash16();
      readFlash16();
      break;

    case 3:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser(F("Select file"));
      display_Clear();
      time = millis();
      if (flashid == 0xC2F3) {
        writeFlash16_29F1601();
      } else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB) || (flashid == 0xC2FC)) {
        writeFlash16_29LV640();
      } else {
        writeFlash16();
      }
      delay(100);
      resetFlash16();
      delay(100);
      verifyFlash16();
      break;

    case 4:
      time = 0;
      display_Clear();
      println_Msg(F("ID Flashrom"));
      idFlash16();
      println_Msg(F(""));
      printFlash16(40);
      println_Msg(F(""));
      display_Update();
      resetFlash16();
      break;

    case 5:
      time = 0;
      display_Clear();
      println_Msg(F("Print first 70Bytes"));
      display_Update();
      resetFlash16();
      printFlash16(70);
      break;

    case 6:
      time = 0;
      display_Clear();
      display_Update();
      resetFlash16();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took: "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  wait();
}

void epromMenu() {
  // create menu with title "Eprom Writer" and 4 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsEprom, 6);
  mainMenu = question_box(F("Eprom Writer"), menuOptions, 6, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      display_Clear();
      println_Msg(F("Blankcheck"));
      display_Update();
      time = millis();
      blankcheck_Eprom();
      break;

    case 1:
      display_Clear();
      time = millis();
      read_Eprom();
      break;

    case 2:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser(F("Select file"));
      display_Clear();
      time = millis();
      write_Eprom();
      delay(1000);
      verify_Eprom();
      break;

    case 3:
      filePath[0] = '\0';
      sd.chdir("/");
      fileBrowser(F("Verify against"));
      sprintf(filePath, "%s/%s", filePath, fileName);
      display_Clear();
      time = millis();
      verify_Eprom();
      break;

    case 4:
      display_Clear();
      time = millis();
      print_Eprom(80);
      break;

    case 5:
      time = 0;
      display_Clear();
      display_Update();
      resetArduino();
      break;
  }
  if (time != 0) {
    print_Msg(F("Operation took: "));
    print_Msg((millis() - time) / 1000, DEC);
    println_Msg(F("s"));
    display_Update();
  }
  wait();
}
#endif

/******************************************
   Flash IDs
 *****************************************/
void id_Flash8() {
  // Test if 28FXXX series flash (type 3 flashrom)
  idFlash28FXXX();

  // Print start screen
idtheflash:
  display_Clear();
  display_Update();
  println_Msg(F("Flashrom Writer 8bit"));
  println_Msg("");
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);

  if (flashid == 0xC2F1) {
    println_Msg(F("MX29F1610 detected"));
    flashSize = 2097152;
    flashromType = 2;
  } else if (flashid == 0xC2F3) {
    println_Msg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  } else if (flashid == 0xC2F9) {
    println_Msg(F("MX29L3211 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C4) || (flashid == 0xC249)) {
    println_Msg(F("MX29LV160 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0xC2A7) || (flashid == 0xC2A8)) {
    println_Msg(F("MX29LV320 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C9) || (flashid == 0xC2CB)) {
    println_Msg(F("MX29LV640 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  } else if (flashid == 0x0141) {
    println_Msg(F("AM29F032B detected"));
    flashSize = 4194304;
    flashromType = 1;
  } else if (flashid == 0x01AD) {
    println_Msg(F("AM29F016B detected"));
    flashSize = 2097152;
    flashromType = 1;
  } else if (flashid == 0x20AD) {
    println_Msg(F("AM29F016D detected"));
    flashSize = 2097152;
    flashromType = 1;
  } else if (flashid == 0x04AD) {
    println_Msg(F("AM29F016D detected"));
    flashSize = 2097152;
    flashromType = 1;
  } else if (flashid == 0x04D4) {
    println_Msg(F("MBM29F033C detected"));
    flashSize = 4194304;
    flashromType = 1;
  } else if (flashid == 0x04D5) {
    println_Msg(F("MBM29F080C detected"));
    flashSize = 1048576;
    flashromType = 1;
  } else if (flashid == 0x0458) {
    println_Msg(F("MBM29F800BA detected"));
    flashSize = 1048576;
    flashromType = 2;
  } else if (flashid == 0x01AB) {
    println_Msg(F("AM29F400AB detected"));
    flashSize = 131072 * 4;
    flashromType = 2;
  } else if (flashid == 0x0158) {
    println_Msg(F("AM29F800BB detected"));
    flashSize = 1048576;
    flashromType = 2;
  } else if (flashid == 0x01A3) {
    println_Msg(F("AM29LV033C detected"));
    flashSize = 131072 * 32;
    flashromType = 1;
  } else if (flashid == 0x017E) {
    // S29GL032M
    if (readByte_Flash(28) == 0x1A) {
      println_Msg(F("S29GL032M detected"));
      flashSize = 4194304;
      sectorSize = 65536;
      bufferSize = 32;
    }
    // S29GL064M
    else if (readByte_Flash(28) == 0x10) {
      println_Msg(F("S29GL064M detected"));
      flashSize = 8388608;
      sectorSize = 65536;
      bufferSize = 32;
    }
    // Unknown S29GL type
    else {
      println_Msg(F("Unknown S29GL Type"));
      flashSize = 4194304;
      sectorSize = 65536;
      bufferSize = 32;
    }
    println_Msg(F("ATTENTION 3.3V"));
    flashromType = 2;
  } else if (flashid == 0xB088) {
    // LH28F016SUT
    println_Msg(F("LH28F016SUT detected"));
    println_Msg(F("ATTENTION 3/5 setting"));
    flashSize = 2097152;
    sectorSize = 65536;
    bufferSize = 256;
    flashromType = 3;
  } else if ((flashid == 0x8916) || (flashid == 0x8917) || (flashid == 0x8918)) {
    // E28FXXXJ3A
    print_Msg(F("E28F"));

    switch (flashid & 0x00f0) {
      case 0x60:
        flashSize = 131072 * 32;
        print_Msg(F("320"));
        break;
      case 0x70:
        flashSize = 131072 * 64;
        print_Msg(F("640"));
        break;
      case 0x80:
        flashSize = 131072 * 128;
        print_Msg(F("128"));
        break;
    }

    println_Msg(F("J3A detected"));
    sectorSize = 131072;
    bufferSize = 32;
    flashromType = 3;
  } else if (secondID == 1) {
    // Read ID a second time using a different command (type 1 flashrom)
    resetFlash8();
    idFlash29F032();
    secondID = 2;
    goto idtheflash;
  } else if (secondID == 2) {
    // Backup first ID read-out
    strncpy(vendorID, flashid_str, 5);

    // Read ID a third time using a different command (type 2 flashrom)
    resetFlash8();
    idFlash29F1610();
    secondID = 0;
    goto idtheflash;
  } else {
    // ID not found
    display_Clear();
    println_Msg(F("Flashrom Writer 8bit"));
    println_Msg("");
    print_Msg(F("ID Type 1: "));
    println_Msg(vendorID);
    print_Msg(F("ID Type 2: "));
    println_Msg(flashid_str);
    println_Msg("");
    println_Msg(F("UNKNOWN FLASHROM"));
    println_Msg("");
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();

    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(F(""));
    printFlash(40);
    println_Msg(F(""));
    display_Update();
    resetFlash8();
    print_FatalError(F("Press Button to reset"));
  }
  println_Msg("");
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();

  resetFlash8();
}

#ifdef enable_FLASH16
void id_Flash16() {
  // ID flash
  idFlash16();
  resetFlash16();

  println_Msg(F("Flashrom Writer 16bit"));
  println_Msg("");
  print_Msg(F("Flash ID: "));
  println_Msg(flashid_str);
  if (flashid == 0xC2F1) {
    println_Msg(F("MX29F1610 detected"));
    println_Msg("");
    flashSize = 2097152;
    flashromType = 2;
  } else if (flashid == 0xC2F3) {
    println_Msg(F("MX29F1601 detected"));
    flashSize = 2097152;
    flashromType = 2;
  } else if (flashid == 0xC2F9) {
    println_Msg(F("MX29L3211 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C4) || (flashid == 0xC249)) {
    println_Msg(F("MX29LV160 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0xC2A7) || (flashid == 0xC2A8)) {
    println_Msg(F("MX29LV320 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C9) || (flashid == 0xC2CB)) {
    println_Msg(F("MX29LV640 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  } else if (flashid == 0xC2FC) {
    println_Msg(F("MX26L6420 detected"));
    println_Msg(F("ATTENTION 3.3V"));
    flashSize = 8388608;
    flashromType = 2;
  } else {
    print_FatalError(F("Unknown flashrom"));
    println_Msg("");
  }
  println_Msg("");
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
}
#endif

/******************************************
   Setup
 *****************************************/
void setup_Flash8() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) CE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // Setting RST(PH0) OE(PH1) OE_SNS(PH3) WE(PH4) WE_SNS(PH5) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
}

#ifdef enable_FLASH16
void setup_Flash16() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) CE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Setting RST(PH0) OE(PH1) BYTE(PH3) WE(PH4) WP(PH5) HIGH
  PORTH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  delay(100);
}

void setup_Eprom() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;
  // Disable Internal Pullups
  PORTC = 0x00;
  PORTA = 0x00;

  // Set Control Pins to Output VPP/OE(PH5) CE(PH6)
  DDRH |= (1 << 5) | (1 << 6);

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);
  // Setting VPP/OE(PH5) LOW
  PORTH &= ~(1 << 5);

  // 27C322 is a 4MB eprom
  flashSize = 4194304;
}
#endif

/******************************************
   I/O Functions
 *****************************************/
// Switch data pins to read
void dataIn8() {
  // Set to Input
  DDRC = 0x00;
}

#ifdef enable_FLASH16
// Switch data pins to write
void dataOut16() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn16() {
  DDRC = 0x00;
  DDRA = 0x00;
}
#endif

/******************************************
   Low level functions
 *****************************************/
void writeByte_Flash(unsigned long myAddress, byte myData) {
  // A0-A7
  PORTF = myAddress & 0xFF;

  // standard for flash adapter and SNES HiRom
  if (mapping == 1) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
  }
  // for SNES LoRom
  else if (mapping == 0) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
  }
  // for SNES ExLoRom repro
  else if (mapping == 2) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << PL7);
  }
  // for SNES ExHiRom repro
  else if (mapping == 3) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A22
    PORTL = (myAddress >> 16) & 0xFF;
    // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
    if (!(((myAddress >> 16) & 0xFF) & 0x40)) {
      // if PL6 is 0 set PL7 to 1
      PORTL |= (1 << 7);
    } else if (((myAddress >> 16) & 0xFF) & 0x40) {
      // if PL6 is 1 set PL7 to 0
      PORTL &= ~(1 << 7);
    }
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }

  // Data
  PORTC = myData;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WE(PH4) WE_SNS(PH5) to LOW
  PORTH &= ~((1 << 4) | (1 << 5));

  // Leave WE low for at least 60ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WE(PH4) WE_SNS(PH5) to HIGH
  PORTH |= (1 << 4) | (1 << 5);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

byte readByte_Flash(unsigned long myAddress) {
  // A0-A7
  PORTF = myAddress & 0xFF;

  // standard for flash adapter and SNES HiRom
  if (mapping == 1) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
  }
  // for SNES LoRom
  else if (mapping == 0) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
  }
  // for SNES ExLoRom repro
  else if (mapping == 2) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << PL7);
  }
  // for SNES ExHiRom repro
  else if (mapping == 3) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A22
    PORTL = (myAddress >> 16) & 0xFF;
    // Set PL7 to inverse of PL6 to reverse P0 and P1 roms
    if (!(((myAddress >> 16) & 0xFF) & 0x40)) {
      // if PL6 is 0 set PL7 to 1
      PORTL |= (1 << 7);
    } else if (((myAddress >> 16) & 0xFF) & 0x40) {
      // if PL6 is 1 set PL7 to 0
      PORTL &= ~(1 << 7);
    }
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting OE(PH1) OE_SNS(PH3) LOW
  PORTH &= ~((1 << 1) | (1 << 3));

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINC;

  // Setting OE(PH1) OE_SNS(PH3) HIGH
  PORTH |= (1 << 1) | (1 << 3);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

#ifdef enable_FLASH16
void writeWord_Flash(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t");

  // Switch WE(PH4) to LOW
  PORTH &= ~(1 << 4);

  // Leave WE low for at least 60ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch WE(PH4) to HIGH
  PORTH |= (1 << 4);

  // Leave WE high for at least 50ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

word readWord_Flash(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting OE(PH1) LOW
  PORTH &= ~(1 << 1);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  __asm__("nop\n\t");

  // Setting OE(PH1) HIGH
  PORTH |= (1 << 1);
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempWord;
}
#endif

/******************************************
  29F032 flashrom functions
*****************************************/
void resetFlash29F032() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x555, 0xf0);

  // Set data pins to input again
  dataIn8();

  delay(500);
}

void idFlash29F032() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x90);

  // Set data pins to input again
  dataIn8();

  // Read the two id bytes into a string
  flashid = readByte_Flash(0) << 8;
  flashid |= readByte_Flash(1);
  sprintf(flashid_str, "%04X", flashid);
}

void eraseFlash29F032() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x80);
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, 0x10);

  // Set data pins to input again
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  // After a completed erase D7 will output 1
  while ((statusReg & 0x80) != 0x80) {
    // Blink led
    blinkLED();
    delay(100);
    // Update Status
    statusReg = readByte_Flash(0);
  }
}

void writeFlash29F032() {
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
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    // Retry writing, for when /RESET is not connected (floating)
    int dq5failcnt = 0;
    int noread = 0;
    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      // if (currByte >= 0) {
      //   print_Msg(currByte);
      //   print_Msg(F(" "));
      //   print_Msg(dq5failcnt);
      //   println_Msg(F(""));
      // }
      if (!noread) {
        myFile.read(sdBuffer, 512);
      }
      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      noInterrupts();
      int blockfailcnt = 0;
      for (int c = 0; c < 512; c++) {
        uint8_t datum = sdBuffer[c];
        dataIn8();
        uint8_t d = readByte_Flash(currByte + c);
        dataOut();
        if (d == datum || datum == 0xFF) {
          continue;
        }
        // Write command sequence
        writeByte_Flash(0x555, 0xaa);
        writeByte_Flash(0x2aa, 0x55);
        writeByte_Flash(0x555, 0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, datum);
        if (busyCheck29F032(currByte + c, datum)) {
          dq5failcnt++;
          blockfailcnt++;
        }
      }
      interrupts();
      if (blockfailcnt > 0) {
        print_Msg(F("Failures at "));
        print_Msg(currByte);
        print_Msg(F(": "));
        print_Msg(blockfailcnt);
        println_Msg(F(""));
        dq5failcnt -= blockfailcnt;
        currByte -= 512;
        delay(100);
        noread = 1;
      } else {
        noread = 0;
      }
    }
    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

int busyCheck29F032(uint32_t addr, byte c) {
  int ret = 0;
  // Set data pins to input
  dataIn8();

  // Setting OE(PH1) OE_SNS(PH3) CE(PH6)LOW
  PORTH &= ~((1 << 1) | (1 << 3) | (1 << 6));
  // Setting WE(PH4) WE_SNES(PH5) HIGH
  PORTH |= (1 << 4) | (1 << 5);

  //When the Embedded Program algorithm is complete, the device outputs the datum programmed to D7
  for (;;) {
    uint8_t d = readByte_Flash(addr);
    if ((d & 0x80) == (c & 0x80)) {
      break;
    }
    if ((d & 0x20) == 0x20) {
      // From the datasheet:
      // DQ 5 will indicate if the program or erase time has exceeded the specified limits (internal pulse count).
      // Under these conditions DQ 5 will produce a “1”.
      // This is a failure condition which indicates that the program or erase cycle was not successfully completed.
      // Note : DQ 7 is rechecked even if DQ 5 = “1” because DQ 7 may change simultaneously with DQ 5 .
      d = readByte_Flash(addr);
      if ((d & 0x80) == (c & 0x80)) {
        break;
      } else {
        ret = 1;
        break;
      }
    }
  }

  // Set data pins to output
  dataOut();

  // Setting OE(PH1) OE_SNS(PH3) HIGH
  PORTH |= (1 << 1) | (1 << 3);
  return ret;
}
/******************************************
  29F1610 flashrom functions
*****************************************/

void resetFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Reset command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0xf0);

  // Set data pins to input again
  dataIn8();

  delay(500);
}

void writeFlash29F1610() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
      // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 3072 == 0)
        blinkLED();

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck29F1610();

      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);
      }
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

void writeFlash29F1601() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 128) {
      // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 3072 == 0)
        blinkLED();

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck29F1610();

      // Write command sequence
      writeByte_Flash(0x5555 << 1, 0xaa);
      writeByte_Flash(0x2aaa << 1, 0x55);
      writeByte_Flash(0x5555 << 1, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);

        if (c == 127) {
          // Write the last byte twice or else it won't write at all
          writeByte_Flash(currByte + c, sdBuffer[c]);
        }
      }
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

void idFlash29F1610() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x90);

  // Set data pins to input again
  dataIn8();

  // Read the two id bytes into a string
  flashid = readByte_Flash(0) << 8;
  flashid |= readByte_Flash(2);
  sprintf(flashid_str, "%04X", flashid);
}

byte readStatusReg() {
  // Set data pins to output
  dataOut();

  // Status reg command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x70);

  // Set data pins to input again
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);
  return statusReg;
}

void eraseFlash29F1610() {
  // Set data pins to output
  dataOut();

  // Erase command sequence
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x80);
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, 0x10);

  // Set data pins to input again
  dataIn8();

  busyCheck29F1610();
}

// Delay between write operations based on status register
void busyCheck29F1610() {
  // Set data pins to input
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  while ((statusReg & 0x80) != 0x80) {
    statusReg = readByte_Flash(0);
  }

  // Set data pins to output
  dataOut();
}

/******************************************
  MX29LV flashrom functions
*****************************************/
void busyCheck29LV640(unsigned long myAddress, byte myData) {
  // Set data pins to input
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(myAddress);
  while ((statusReg & 0x80) != (myData & 0x80)) {
    statusReg = readByte_Flash(myAddress);
  }

  // Set data pins to output
  dataOut();
}

void writeFlash29LV640() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      // Fill sdBuffer
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 4096 == 0)
        blinkLED();
      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByte_Flash(0x555 << 1, 0xaa);
        writeByte_Flash(0x2aa << 1, 0x55);
        writeByte_Flash(0x555 << 1, 0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        // Check if write is complete
        busyCheck29LV640(currByte + c, sdBuffer[c]);
      }
    }
    // Set data pins to input again
    dataIn8();
    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

/******************************************
  S29GL flashrom functions
*****************************************/
void writeFlash29GL(unsigned long sectorSize, byte bufferSize) {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    for (unsigned long currSector = 0; currSector < fileSize; currSector += sectorSize) {
      // Blink led
      blinkLED();

      // Write to flashrom
      for (unsigned long currSdBuffer = 0; currSdBuffer < sectorSize; currSdBuffer += 512) {
        // Fill SD buffer
        myFile.read(sdBuffer, 512);

        // Write bufferSize bytes at a time
        for (int currWriteBuffer = 0; currWriteBuffer < 512; currWriteBuffer += bufferSize) {
          // 2 unlock commands
          writeByte_Flash(0x555 << 1, 0xaa);
          writeByte_Flash(0x2aa << 1, 0x55);
          // Write buffer load command at sector address
          writeByte_Flash(currSector + currSdBuffer + currWriteBuffer, 0x25);
          // Write byte count (minus 1) at sector address
          writeByte_Flash(currSector + currSdBuffer + currWriteBuffer, bufferSize - 1);

          // Load bytes into buffer
          for (byte currByte = 0; currByte < bufferSize; currByte++) {
            writeByte_Flash(currSector + currSdBuffer + currWriteBuffer + currByte, sdBuffer[currWriteBuffer + currByte]);
          }

          // Write Buffer to Flash
          writeByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1, 0x29);

          // Read the status register at last written address
          dataIn8();
          byte statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          while ((statusReg & 0x80) != (sdBuffer[currWriteBuffer + bufferSize - 1] & 0x80)) {
            statusReg = readByte_Flash(currSector + currSdBuffer + currWriteBuffer + bufferSize - 1);
          }
          dataOut();
        }
      }
    }
    // Set data pins to input again
    dataIn8();
    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

/******************************************
  29F800 functions
*****************************************/
void writeFlash29F800() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut();

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByte_Flash(0x5555 << 1, 0xaa);
        writeByte_Flash(0x2aaa << 1, 0x55);
        writeByte_Flash(0x5555 << 1, 0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        busyCheck29F032(currByte + c, sdBuffer[c]);
      }
    }

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

/******************************************
  28FXXX series flashrom functions
*****************************************/
void idFlash28FXXX() {
  dataOut();
  writeByte_Flash(0x0, 0x90);

  dataIn8();

  // Read the two id bytes into a string
  flashid = readByte_Flash(0) << 8;
  flashid |= readByte_Flash(1);
  sprintf(flashid_str, "%04X", flashid);
}

void resetFlash28FXXX() {
  dataOut();
  writeByte_Flash(0x0, 0xff);

  dataIn();
  delay(500);
}

uint8_t statusFlash28FXXX() {
  dataOut();

  writeByte_Flash(0x0, 0x70);
  dataIn8();
  return readByte_Flash(0x0);
}

void eraseFlash28FXXX() {
  // only can erase block by block
  for (uint32_t ba = 0; ba < flashSize; ba += sectorSize) {
    dataOut();
    writeByte_Flash(ba, 0x20);
    writeByte_Flash(ba, 0xd0);

    dataIn8();
    while ((readByte_Flash(ba) & 0x80) == 0x00)
      ;

    // blink LED
    blinkLED();
  }
}

void writeFlash28FXXX() {
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 0);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    if ((flashid == 0xB088))
      writeFlashLH28F0XX();
    else if ((flashid == 0x8916) || (flashid == 0x8917) || (flashid == 0x8918)) {
      writeFlashE28FXXXJ3A();
    }

    myFile.close();
  } else {
    print_STR(open_file_STR, 1);
    display_Update();
  }
}

void writeFlashE28FXXXJ3A() {
  fileSize = myFile.fileSize();
  if (fileSize > flashSize) {
    print_Error(file_too_big_STR);
    return;
  }

  uint32_t block_addr;
  uint32_t block_addr_mask = ~(sectorSize - 1);

  // Fill sdBuffer
  for (uint32_t currByte = 0; currByte < fileSize; currByte += 512) {
    myFile.read(sdBuffer, 512);

    // Blink led
    if (currByte % 2048 == 0)
      blinkLED();

    block_addr = currByte & block_addr_mask;

    for (uint32_t c = 0; c < 512; c += bufferSize) {
      // write to buffer start
      dataOut();
      writeByte_Flash(block_addr, 0xe8);

      // waiting for buffer available
      dataIn8();
      while ((readByte_Flash(block_addr) & 0x80) == 0x00)
        ;
      dataOut();

      // set write byte count
      writeByte_Flash(block_addr, bufferSize - 1);

      // filling buffer
      for (uint32_t d = 0; d < bufferSize; d++)
        writeByte_Flash(currByte + c + d, sdBuffer[c + d]);

      // start flashing page
      writeByte_Flash(block_addr, 0xd0);

      // waiting for finishing
      dataIn8();
      while ((readByte_Flash(block_addr) & 0x80) == 0x00)
        ;
    }
  }

  dataIn8();
}

void writeFlashLH28F0XX() {
  fileSize = myFile.fileSize();
  if (fileSize > flashSize) {
    print_Error(file_too_big_STR);
    return;
  }

  // Fill sdBuffer
  for (uint32_t currByte = 0; currByte < fileSize; currByte += 512) {
    myFile.read(sdBuffer, 512);
    // Blink led
    if (currByte % 2048 == 0)
      blinkLED();

    for (uint32_t c = 0; c < 512; c += bufferSize) {
      // sequence load to page
      dataOut();
      writeByte_Flash(0x0, 0xe0);
      writeByte_Flash(0x0, bufferSize - 1);  // BCL
      writeByte_Flash(0x0, 0x00);            // BCH should be 0x00

      for (uint32_t d = 0; d < bufferSize; d++)
        writeByte_Flash(d, sdBuffer[c + d]);

      // start flashing page
      writeByte_Flash(0x0, 0x0c);
      writeByte_Flash(0x0, bufferSize - 1);  // BCL
      writeByte_Flash(currByte + c, 0x00);   // BCH should be 0x00

      // waiting for finishing
      dataIn8();
      while ((readByte_Flash(currByte + c) & 0x80) == 0x00)
        ;
    }
  }

  dataIn8();
}

/******************************************
  Common flashrom functions
*****************************************/
void blankcheck_Flash() {
  println_Msg(F("Please wait..."));
  display_Update();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize; currByte++) {
    // Check if all bytes are 0xFF
    if (readByte_Flash(currByte) != 0xFF) {
      currByte = flashSize;
      blank = 0;
    }
  }
  if (blank) {
    println_Msg(F("Flashrom is empty"));
    display_Update();
  } else {
    print_Error(F("Error: Not blank"));
  }
}

void verifyFlash() {
  print_STR(verifying_STR, 1);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    blank = 0;
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
        if (readByte_Flash(currByte + c) != sdBuffer[c]) {
          blank++;
        }
      }
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

void readFlash() {
  // Reset to root directory
  sd.chdir("/");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  display_Clear();
  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(create_file_STR);
  }
  for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
    for (int c = 0; c < 512; c++) {
      sdBuffer[c] = readByte_Flash(currByte + c);
    }
    myFile.write(sdBuffer, 512);
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading"));
  display_Update();
}

void printFlash(int numBytes) {
  char myBuffer[3];

  for (int currByte = 0; currByte < numBytes; currByte += 10) {
    for (int c = 0; c < 10; c++) {
      itoa(readByte_Flash(currByte + c), myBuffer, 16);
      for (size_t i = 0; i < 2 - strlen(myBuffer); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(myBuffer);
    }
    println_Msg("");
  }
  display_Update();
}

void resetFlash8() {
  switch (flashromType) {
    case 1: resetFlash29F032(); break;
    case 2: resetFlash29F1610(); break;
    case 3: resetFlash28FXXX(); break;
  }
}

#ifdef enable_FLASH16
/******************************************
  29L3211 16bit flashrom functions
*****************************************/
void resetFlash16() {
  // Set data pins to output
  dataOut16();

  // Reset command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0xf0);

  // Set data pins to input again
  dataIn16();

  delay(500);
}

void writeFlash16() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut16();

    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    int d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWord_Flash(0x5555, 0xaa);
      writeWord_Flash(0x2aaa, 0x55);
      writeWord_Flash(0x5555, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 64; c++) {
        word currWord = ((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF);
        writeWord_Flash(currByte + c, currWord);
        d += 2;
      }
      d = 0;
    }

    // Check if write is complete
    busyCheck16();

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void writeFlash16_29F1601() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut16();

    // Fill sdBuffer with 1 page at a time then write it repeat until all bytes are written
    int d = 0;
    for (unsigned long currByte = 0; currByte < fileSize / 2; currByte += 64) {
      myFile.read(sdBuffer, 128);

      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      // Check if write is complete
      delayMicroseconds(100);
      busyCheck16();

      // Write command sequence
      writeWord_Flash(0x5555, 0xaa);
      writeWord_Flash(0x2aaa, 0x55);
      writeWord_Flash(0x5555, 0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 64; c++) {
        word currWord = ((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF);
        writeWord_Flash(currByte + c, currWord);

        if (c == 63) {
          // Write the last byte twice or else it won't write at all
          writeWord_Flash(currByte + c, sdBuffer[d + 1]);
        }
        d += 2;
      }
      d = 0;
    }

    // Check if write is complete
    busyCheck16();

    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void idFlash16() {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x90);

  // Set data pins to input again
  dataIn16();

  // Read the two id bytes into a string
  flashid = (readWord_Flash(0) & 0xFF) << 8;
  flashid |= readWord_Flash(1) & 0xFF;
  sprintf(flashid_str, "%04X", flashid);
}

byte readStatusReg16() {
  // Set data pins to output
  dataOut16();

  // Status reg command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x70);

  // Set data pins to input again
  dataIn16();

  // Read the status register
  byte statusReg = readWord_Flash(0);
  return statusReg;
}

void eraseFlash16() {
  // Set data pins to output
  dataOut16();

  // Erase command sequence
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x80);
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, 0x10);

  // Set data pins to input again
  dataIn16();

  busyCheck16();
}

void blankcheck16() {

  println_Msg(F("Please wait..."));
  display_Update();

  blank = 1;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte++) {
    if (readWord_Flash(currByte) != 0xFFFF) {
      currByte = flashSize / 2;
      blank = 0;
    }
  }
  if (blank) {
    println_Msg(F("Flashrom is empty."));
    display_Update();
  } else {
    print_Error(F("Error: Not blank"));
  }
}

void verifyFlash16() {
  print_STR(verifying_STR, 1);
  display_Update();

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
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 256; c++) {
        word currWord = ((sdBuffer[d + 1] << 8) | sdBuffer[d]);

        if (readWord_Flash(currByte + c) != currWord) {
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
      println_Msg(F("Verification ERROR!"));
      print_Msg(blank);
      print_Error(F("B did not verify."));
      display_Update();
    }
    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void readFlash16() {
  // Reset to root directory
  sd.chdir("/");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  display_Clear();
  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("Can't create file on SD."));
    display_Update();
    while (1)
      ;
  }
  word d = 0;
  for (unsigned long currByte = 0; currByte < flashSize / 2; currByte += 256) {
    for (word c = 0; c < 256; c++) {
      word currWord = readWord_Flash(currByte + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = ((currWord >> 8) & 0xFF);
      // Left
      sdBuffer[d] = (currWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading."));
  display_Update();
}

void printFlash16(int numBytes) {
  /*
    right_byte = short_val & 0xFF;
    left_byte = ( short_val >> 8 ) & 0xFF
    short_val = ( ( left_byte & 0xFF ) << 8 ) | ( right_byte & 0xFF );
  */

  char buf[3];

  for (int currByte = 0; currByte < numBytes / 2; currByte += 5) {
    // 5 words per line
    for (int c = 0; c < 5; c++) {
      word currWord = readWord_Flash(currByte + c);

      // Split word into two bytes
      byte left_byte = currWord & 0xFF;
      byte right_byte = (currWord >> 8) & 0xFF;


      sprintf(buf, "%x", left_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(buf);

      sprintf(buf, "%x", right_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(buf);
    }
    println_Msg("");
  }
  display_Update();
}

// Delay between write operations based on status register
void busyCheck16() {
  // Set data pins to input
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash(0);

  while ((statusReg | 0xFF7F) != 0xFFFF) {
    statusReg = readWord_Flash(0);
  }

  // Set data pins to output
  dataOut16();
}

/******************************************
  MX29LV flashrom functions 16bit
*****************************************/
// Delay between write operations based on status register
void busyCheck16_29LV640(unsigned long myAddress, word myData) {
  // Set data pins to input
  dataIn16();

  // Read the status register
  word statusReg = readWord_Flash(myAddress);
  while ((statusReg & 0x80) != (myData & 0x80)) {
    statusReg = readWord_Flash(myAddress);
  }

  // Set data pins to output
  dataOut16();
}

void writeFlash16_29LV640() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Set data pins to output
    dataOut16();

    int d = 0;
    for (unsigned long currWord = 0; currWord < fileSize / 2; currWord += 256) {
      // Fill sdBuffer
      myFile.read(sdBuffer, 512);

      // Blink led
      if (currWord % 4096 == 0)
        blinkLED();

      for (int c = 0; c < 256; c++) {
        // Write command sequence
        writeWord_Flash(0x5555, 0xaa);
        writeWord_Flash(0x2aaa, 0x55);
        writeWord_Flash(0x5555, 0xa0);

        // Write current word
        word myWord = ((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF);
        writeWord_Flash(currWord + c, myWord);
        d += 2;
        // Check if write is complete
        busyCheck16_29LV640(currWord + c, myWord);
      }
      d = 0;
    }
    // Set data pins to input again
    dataIn16();

    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

/******************************************
  Eprom functions
*****************************************/
word writeWord_Eprom(unsigned long myAddress, word myData) {
  // Data out
  DDRC = 0xFF;
  DDRA = 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  // Set data
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  __asm__("nop\n\t");

  // Switch VPP/OE(PH5) to HIGH
  PORTH |= (1 << 5);
  // Wait 1us for VPP High to Chip Enable Low
  delayMicroseconds(1);
  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Leave VPP HIGH for 50us Chip Enable Program Pulse Width
  delayMicroseconds(55);

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);
  // Wait 2us for Chip Enable High to VPP Transition
  delayMicroseconds(2);
  // Switch VPP/OE(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave CE High for 1us for VPP Low to Chip Enable Low
  delayMicroseconds(1);

  // Data in
  DDRC = 0x00;
  DDRA = 0x00;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Wait 1us for Chip Enable Low to Output Valid while program verify
  delayMicroseconds(3);

  // Read
  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);

  // Delay 130ns for Chip Enable High to Output Hi-Z
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempWord;
}

word readWord_Eprom(unsigned long myAddress) {
  // Data in
  DDRC = 0x00;
  DDRA = 0x00;
  // Set address
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t");

  // Setting CE(PH6) LOW
  PORTH &= ~(1 << 6);

  // Delay for 100ns for Address Valid/Chip Enable Low to Output Valid
  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  // Setting CE(PH6) HIGH
  PORTH |= (1 << 6);

  return tempWord;
}

void blankcheck_Eprom() {
  println_Msg(F("Please wait..."));
  display_Update();

  blank = 1;
  for (unsigned long currWord = 0; currWord < flashSize / 2; currWord++) {
    if (readWord_Eprom(currWord) != 0xFFFF) {
      currWord = flashSize / 2;
      blank = 0;
    }
  }
  if (blank) {
    println_Msg(F("Flashrom is empty."));
    display_Update();
  } else {
    print_Error(F("Error: Not blank"));
  }
}

void read_Eprom() {
  // Reset to root directory
  sd.chdir("/");

  // Get name, add extension and convert to char array for sd lib
  EEPROM_readAnything(0, foldern);
  sd.mkdir("FLASH", true);
  sd.chdir("FLASH");
  sprintf(fileName, "FL%d", foldern);
  strcat(fileName, ".bin");
  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  display_Clear();
  print_Msg(F("Saving as "));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    println_Msg(F("Can't create file on SD."));
    display_Update();
    while (1)
      ;
  }
  word d = 0;
  for (unsigned long currWord = 0; currWord < flashSize / 2; currWord += 256) {
    for (word c = 0; c < 256; c++) {
      word myWord = readWord_Eprom(currWord + c);
      // Split word into two bytes
      // Right
      sdBuffer[d + 1] = ((myWord >> 8) & 0xFF);
      // Left
      sdBuffer[d] = (myWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
  }

  // Close the file:
  myFile.close();
  println_Msg(F("Finished reading."));
  display_Update();
}

void write_Eprom() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 1);
  println_Msg(filePath);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize)
      print_FatalError(file_too_big_STR);

    // Switch VPP/OE(PH5) to HIGH
    PORTH |= (1 << 5);
    delay(1000);

    for (unsigned long currWord = 0; currWord < fileSize / 2; currWord += 256) {
      // Fill SD buffer
      myFile.read(sdBuffer, 512);
      int d = 0;

      // Blink led
      if (currWord % 2048 == 0)
        blinkLED();

      // Work through SD buffer
      for (int c = 0; c < 256; c++) {
        word checkWord;
        word myWord = ((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF);

        // Error counter
        byte n = 0;

        // Presto III allows up to 25 rewrites per word
        do {
          // Write word
          checkWord = writeWord_Eprom(currWord + c, myWord);
          // Check for fail
          if (n == 25) {
            print_Msg(F("Program Error 0x"));
            println_Msg(currWord + c, HEX);
            print_Msg(F("0x"));
            print_Msg(readWord_Eprom(currWord + c), HEX);
            print_Msg(F(" != 0x"));
            println_Msg(myWord, HEX);
            print_FatalError(F("Press button to reset"));
          }
          n++;
        } while (checkWord != myWord);
        d += 2;
      }
    }
    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void verify_Eprom() {
  print_STR(verifying_STR, 1);
  display_Update();

  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize) {
      print_FatalError(file_too_big_STR);
    }

    blank = 0;
    word d = 0;
    for (unsigned long currWord = 0; currWord < (fileSize / 2); currWord += 256) {
      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 256; c++) {
        word myWord = (((sdBuffer[d + 1] & 0xFF) << 8) | (sdBuffer[d] & 0xFF));

        if (readWord_Eprom(currWord + c) != myWord) {
          blank++;
        }
        d += 2;
      }
      d = 0;
    }
    if (blank == 0) {
      println_Msg(F("Eprom verified OK"));
      display_Update();
    } else {
      println_Msg(F("Verification ERROR!"));
      print_Msg(blank);
      print_Error(F(" words did not verify."));
      display_Update();
    }
    // Close the file:
    myFile.close();
  } else {
    println_Msg(F("Can't open file on SD."));
    display_Update();
  }
}

void print_Eprom(int numBytes) {
  char buf[3];

  for (int currByte = 0; currByte < numBytes / 2; currByte += 5) {
    // 5 words per line
    for (int c = 0; c < 5; c++) {
      word currWord = readWord_Eprom(currByte + c);

      // Split word into two bytes
      byte left_byte = currWord & 0xFF;
      byte right_byte = (currWord >> 8) & 0xFF;


      sprintf(buf, "%x", left_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(buf);

      sprintf(buf, "%x", right_byte);
      for (size_t i = 0; i < 2 - strlen(buf); i++) {
        print_Msg(F("0"));
      }
      // Now print the significant bits
      print_Msg(buf);
    }
    println_Msg("");
  }
  display_Update();
}
#endif
#endif

//******************************************
// End of File
//******************************************
