//******************************************
// FLASHROM MODULE
// (also includes SNES repro functions)
//******************************************
#ifdef ENABLE_FLASH8

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
byte mapping = 0;

/******************************************
   Menu
 *****************************************/
// General Flash menu items
static const char flashMenuItemBlankcheck[] PROGMEM = "Blankcheck";
static const char flashMenuItemID[] PROGMEM = "ID";
static const char flashMenuItemRead[] PROGMEM = "Read";
static const char flashMenuItemWrite[] PROGMEM = "Write";
static const char flashMenuItemErase[] PROGMEM = "Erase";
static const char flashMenuItemPrint[] PROGMEM = "Print";

// 8bit Flash menu items
static const char* const menuOptionsFLASH8[] PROGMEM = { flashMenuItemBlankcheck, flashMenuItemErase, flashMenuItemRead, flashMenuItemWrite, flashMenuItemID, flashMenuItemPrint, FSTRING_RESET };

#ifndef ENABLE_FLASH16
// Flash mode menu
static const char modeMenuItem1[] PROGMEM = "CFI Mode";
static const char modeMenuItem2[] PROGMEM = "Standard Mode";
static const char* const menuOptionsMode[] PROGMEM = { modeMenuItem1, modeMenuItem2, FSTRING_RESET };
#endif

// Misc flash strings
const char PROGMEM ATTENTION_3_3V[] = "ATTENTION 3.3V";

#ifdef ENABLE_FLASH16
// Flash start menu
static const char flashMenuItem1[] PROGMEM = "CFI";
static const char flashMenuItem2[] PROGMEM = "8bit Flash";
static const char flashMenuItem3[] PROGMEM = "Eprom";
static const char flashMenuItem4[] PROGMEM = "16bit Flash";
static const char* const menuOptionsFlash[] PROGMEM = { flashMenuItem1, flashMenuItem2, flashMenuItem3, flashMenuItem4, FSTRING_RESET };

// 16bit Flash menu items
static const char* const menuOptionsFLASH16[] PROGMEM = { flashMenuItemBlankcheck, flashMenuItemErase, flashMenuItemRead, flashMenuItemWrite, flashMenuItemID, flashMenuItemPrint, FSTRING_RESET };

// Eprom menu items
static const char epromMenuItem4[] PROGMEM = "Verify";
static const char* const menuOptionsEprom[] PROGMEM = { flashMenuItemBlankcheck, flashMenuItemRead, flashMenuItemWrite, epromMenuItem4, flashMenuItemPrint, FSTRING_RESET };

void flashMenu() {
  display_Clear();
  display_Update();
  mapping = 0;

  // create menu with title and 5 options to choose from
  unsigned char flashSlot;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsFlash, 5);
  flashSlot = question_box(F("Select Mode"), menuOptions, 5, 0);

  // wait for user choice to come back from the question box menu
  switch (flashSlot) {
    case 0:
      setupCFI();
      flashSize = 8388608;
      writeCFI_Flash(1, 1, 0);
      verifyFlash();
      print_STR(press_button_STR, 0);
      display_Update();
      wait();
      resetArduino();
      break;

    case 1:
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;

    case 2:
      setup_Eprom();
      mode = CORE_EPROM;
      break;

    case 3:
      setup_Flash16();
      id_Flash16();
      wait();
      mode = CORE_FLASH16;
      break;

    case 4:
      resetArduino();
      break;

    default:
      print_MissingModule();  // does not return
  }
}
#else
void flashMenu() {
  display_Clear();
  display_Update();
  mapping = 0;

  // create menu with title and 3 options to choose from
  unsigned char flashMode;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsMode, 3);
  flashMode = question_box(F("Select Flash Mode"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (flashMode) {
    case 0:
      setupCFI();
      flashSize = 8388608;
      writeCFI_Flash(1, 1, 0);
      verifyFlash();
      print_STR(press_button_STR, 0);
      display_Update();
      wait();
      resetArduino();
      break;

    case 1:
      setup_Flash8();
      id_Flash8();
      wait();
      mode = CORE_FLASH8;
      break;
  }
}
#endif

void setupCFI() {
  display_Clear();
  display_Update();
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(F("Select file"));
  display_Clear();
  setup_Flash8();
  identifyCFI_Flash();
  sprintf(filePath, "%s/%s", filePath, fileName);
  display_Clear();
}

void readOnlyMode() {
  display_Clear();
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(F("Read-only Mode!"));
  println_Msg(FS(FSTRING_EMPTY));
  println_Msg(FS(FSTRING_EMPTY));
  display_Update();
}

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
      if (flashromType != 0) {
        display_Clear();
        println_Msg(F("Warning: This will erase"));
        println_Msg(F("your flashrom/repro"));
        print_STR(press_button_STR, 1);
        display_Update();
        wait();
        rgbLed(black_color);
        println_Msg(FS(FSTRING_EMPTY));
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
      } else {
        readOnlyMode();
      }
      break;

    case 2:
      time = millis();
      resetFlash8();
      readFlash();
      break;

    case 3:
      if (flashromType != 0) {
        filePath[0] = '\0';
        sd.chdir("/");
        fileBrowser(FS(FSTRING_SELECT_FILE));
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
            else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB) || (flashid == 0x0149) || (flashid == 0x01C4) || (flashid == 0x01F9) || (flashid == 0x01F6) || (flashid == 0x01D7))
              writeFlash29LV640();
            else if (flashid == 0x017E)
              writeFlash29GL(sectorSize, bufferSize);
            else if ((flashid == 0x0458) || (flashid == 0x0158) || (flashid == 0x01AB))
              writeFlash29F800();
            else if (flashid == 0x0)  // Manual flash config, pick most common type
              writeFlash29LV640();
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
      } else {
        readOnlyMode();
      }
      break;

    case 4:
      time = 0;
      display_Clear();
      resetFlash8();
      println_Msg(F("ID Flashrom"));
      switch (flashromType) {
        case 0: break;
        case 1: idFlash29F032(); break;
        case 2: idFlash29F1610(); break;
        case 3: idFlash28FXXX(); break;
      }
      println_Msg(FS(FSTRING_EMPTY));
      printFlash(40);
      println_Msg(FS(FSTRING_EMPTY));
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

#ifdef ENABLE_FLASH16
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
      fileBrowser(FS(FSTRING_SELECT_FILE));
      display_Clear();
      time = millis();
      if (flashid == 0xC2F3) {
        writeFlash16_29F1601();
      } else if ((flashid == 0xC2C4) || (flashid == 0xC249) || (flashid == 0xC2A7) || (flashid == 0xC2A8) || (flashid == 0xC2C9) || (flashid == 0xC2CB) || (flashid == 0x0149) || (flashid == 0x01C4) || (flashid == 0x01F9) || (flashid == 0x01F6) || (flashid == 0x01D7) || (flashid == 0xC2FC)) {
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
      println_Msg(FS(FSTRING_EMPTY));
      printFlash16(40);
      println_Msg(FS(FSTRING_EMPTY));
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
      fileBrowser(FS(FSTRING_SELECT_FILE));
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
   Flash ID
 *****************************************/
void printFlashSize(int index) {
  display_Clear();
  print_Msg(F("Flashsize: "));
  print_Msg(index);
  println_Msg(F("MB"));
}

void printFlashType(int index) {
  display_Clear();
  print_Msg(F("Flashtype: "));
  println_Msg(index);
}

byte selectFlashtype(boolean option) {
  byte selectionByte;
  if (option)
    selectionByte = navigateMenu(0, 3, &printFlashType);
  else
    selectionByte = navigateMenu(1, 8, &printFlashSize);
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  display.setCursor(0, 56);  // Display selection at bottom
#endif
  if (option) {
    print_Msg(F("Flash Type: "));
    println_Msg(selectionByte);
  } else {
    print_Msg(F("Flash Size: "));
    print_Msg(selectionByte);
    println_Msg(F("MB"));
  }
  display_Update();
  delay(200);
  return selectionByte;
}

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
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C4) || (flashid == 0xC249)) {
    println_Msg(F("MX29LV160 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0xC2A7) || (flashid == 0xC2A8)) {
    println_Msg(F("MX29LV320 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C9) || (flashid == 0xC2CB)) {
    println_Msg(F("MX29LV640 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 8388608;
    flashromType = 2;
  } else if ((flashid == 0x0149) || (flashid == 0x01C4)) {
    println_Msg(F("AM29LV160 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0x01F9) || (flashid == 0x01F6)) {
    println_Msg(F("AM29LV320 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if (flashid == 0x01D7) {
    println_Msg(F("AM29LV640 detected"));
    println_Msg(FS(ATTENTION_3_3V));
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
      flashSize = 8388608;
      sectorSize = 65536;
      bufferSize = 32;
    }
    println_Msg(FS(ATTENTION_3_3V));
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
    print_Error(press_button_STR);
    display_Update();
    wait();

    // Select flashrom config manually
    flashSize = selectFlashtype(0) * 1024UL * 1024UL;
    flashromType = selectFlashtype(1);
    flashid = 0;
    sprintf(flashid_str, "%04X", 0);

    // print first 40 bytes of flash
    display_Clear();
    println_Msg(F("First 40 bytes:"));
    println_Msg(FS(FSTRING_EMPTY));
    printFlash(40);
    resetFlash8();
  }
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  resetFlash8();
}

#ifdef ENABLE_FLASH16
void id_Flash16() {
  // ID flash
  idFlash16();
  resetFlash16();

  display_Clear();
  display_Update();
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
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C4) || (flashid == 0xC249)) {
    println_Msg(F("MX29LV160 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0xC2A7) || (flashid == 0xC2A8)) {
    println_Msg(F("MX29LV320 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if ((flashid == 0xC2C9) || (flashid == 0xC2CB)) {
    println_Msg(F("MX29LV640 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 8388608;
    flashromType = 2;
  } else if ((flashid == 0x0149) || (flashid == 0x01C4)) {
    println_Msg(F("AM29LV160 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 2097152;
    flashromType = 2;
  } else if ((flashid == 0x01F9) || (flashid == 0x01F6)) {
    println_Msg(F("AM29LV320 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 4194304;
    flashromType = 2;
  } else if (flashid == 0x01D7) {
    println_Msg(F("AM29LV640 detected"));
    println_Msg(FS(ATTENTION_3_3V));
    flashSize = 8388608;
    flashromType = 2;
  } else if (flashid == 0xC2FC) {
    println_Msg(F("MX26L6420 detected"));
    println_Msg(FS(ATTENTION_3_3V));
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
#if defined(ENABLE_VSELECT) || defined(ENABLE_3V3FIX)
static const char flashvoltItem1[] PROGMEM = "3.3V";
static const char flashvoltItem2[] PROGMEM = "5V";
static const char* const flashvoltOptions[] PROGMEM = { flashvoltItem1, flashvoltItem2, FSTRING_RESET };

void setup_FlashVoltage() {
  // create menu with title and 3 options to choose from
  unsigned char flashvolt;
  // Copy menuOptions out of progmem
  convertPgm(flashvoltOptions, 3);
  flashvolt = question_box(F("Select Flash Voltage"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (flashvolt) {
    case 0:
      // Request 3.3V
      setVoltage(VOLTS_SET_3V3);
      break;

    case 1:
      // Request 5V
      setVoltage(VOLTS_SET_5V);
      break;

    case 2:
      resetArduino();
      break;
  }
}
#else
// The compiler will optimize this out when this condition is met.
void setup_FlashVoltage() {}
#endif

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

#ifdef ENABLE_FLASH16
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

#ifdef ENABLE_FLASH16
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

  // flash adapter (without SRAM save chip)
  if (mapping == 0) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
  }
  // SNES LoRom
  else if (mapping == 1) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
  }
  // SNES HiRom
  else if (mapping == 2) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
    // Set PL7 to value of PL6
    if (!(((myAddress >> 16) & 0xFF) & 0x40)) {
      // if PL6 is 0 set PL7 to 0
      PORTL &= ~(1 << 7);
    } else if (((myAddress >> 16) & 0xFF) & 0x40) {
      // if PL6 is 1 set PL7 to 1
      PORTL |= (1 << 7);
    }
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }
  // for SNES LoRom repro with 2x 2MB
  else if (mapping == 122) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip BA6(PL6) to address second rom chip
    PORTL ^= (1 << 6);
  }
  // for SNES HiRom repro with 2x 2MB
  else if (mapping == 222) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
    // Flip BA5(PL5) to address second rom chip
    PORTL ^= (1 << 5);
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }
  // for SNES ExLoRom repro with 2x 4MB
  else if (mapping == 124) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << 7);
  }
  // for SNES ExHiRom repro with 2x 4MB
  else if (mapping == 224) {
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
  // for SNES ExLoRom repro with 4x 2MB
  else if (mapping == 142) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip BA6(PL6) to address second rom chip
    PORTL ^= (1 << 6);
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << 7);
  }
  // for SNES ExHiRom repro with 4x 2MB
  else if (mapping == 242) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A22
    PORTL = (myAddress >> 16) & 0xFF;
    // Flip BA5(PL5) to address second rom chip
    PORTL ^= (1 << 5);
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

  // flash adapter (without SRAM save chip)
  if (mapping == 0) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
  }
  // SNES LoRom
  else if (mapping == 1) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
  }
  // SNES HiRom
  else if (mapping == 2) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
    // Set PL7 to value of PL6
    if (!(((myAddress >> 16) & 0xFF) & 0x40)) {
      // if PL6 is 0 set PL7 to 0
      PORTL &= ~(1 << 7);
    } else if (((myAddress >> 16) & 0xFF) & 0x40) {
      // if PL6 is 1 set PL7 to 1
      PORTL |= (1 << 7);
    }
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }
  // for SNES LoRom repro with 2x 2MB
  else if (mapping == 122) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip BA6(PL6) to address second rom chip
    PORTL ^= (1 << 6);
  }
  // for SNES HiRom repro with 2x 2MB
  else if (mapping == 222) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A23
    PORTL = (myAddress >> 16) & 0xFF;
    // Flip BA5(PL5) to address second rom chip
    PORTL ^= (1 << 5);
    // Switch SNES BA6(PL6) to HIGH to disable SRAM
    PORTL |= (1 << 6);
  }
  // for SNES ExLoRom repro with 2x 4MB
  else if (mapping == 124) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << 7);
  }
  // for SNES ExHiRom repro with 2x 4MB
  else if (mapping == 224) {
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
  // for SNES ExLoRom repro with 4x 2MB
  else if (mapping == 142) {
    // A8-A14
    PORTK = (myAddress >> 8) & 0x7F;
    // Set SNES A15(PK7) HIGH to disable SRAM
    PORTK |= (1 << 7);
    // A15-A22
    PORTL = (myAddress >> 15) & 0xFF;
    // Flip BA6(PL6) to address second rom chip
    PORTL ^= (1 << 6);
    // Flip A22(PL7) to reverse P0 and P1 roms
    PORTL ^= (1 << 7);
  }
  // for SNES ExHiRom repro with 4x 2MB
  else if (mapping == 242) {
    // A8-A15
    PORTK = (myAddress >> 8) & 0xFF;
    // A16-A22
    PORTL = (myAddress >> 16) & 0xFF;
    // Flip BA5(PL5) to address second rom chip
    PORTL ^= (1 << 5);
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

/******************************************
  write helper functions
*****************************************/
bool openFileOnSD() {
  // Open file on sd card
  if (myFile.open(filePath, O_READ)) {
    // Get rom size from file
    fileSize = myFile.fileSize();
    if (fileSize > flashSize) {
      display_Clear();
      println_Msg(filePath);
      println_Msg(FS(FSTRING_EMPTY));
      print_Msg(F("File:"));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(fileSize);
      print_STR(_bytes_STR, 1);
      print_Msg(F("Flash:"));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(flashSize);
      print_STR(_bytes_STR, 1);
      print_FatalError(file_too_big_STR);
    }
    return true;
  }
  print_STR(open_file_STR, 1);
  display_Update();
  return false;
}

bool openFlashFile() {
  // Create filepath
  sprintf(filePath, "%s/%s", filePath, fileName);
  print_STR(flashing_file_STR, 0);
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  return openFileOnSD();
}

bool openVerifyFlashFile() {
  print_STR(verifying_STR, 1);
  display_Update();

  return openFileOnSD();
}

/******************************************
  Command functions
*****************************************/
void writeByteCommand_Flash(byte command) {
  writeByte_Flash(0x555, 0xaa);
  writeByte_Flash(0x2aa, 0x55);
  writeByte_Flash(0x555, command);
}

void writeByteCommandShift_Flash(byte command) {
  writeByte_Flash(0x5555 << 1, 0xaa);
  writeByte_Flash(0x2aaa << 1, 0x55);
  writeByte_Flash(0x5555 << 1, command);
}

void writeWordCommand_Flash(byte command) {
  writeWord_Flash(0x5555, 0xaa);
  writeWord_Flash(0x2aaa, 0x55);
  writeWord_Flash(0x5555, command);
}

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
  writeByteCommand_Flash(0x90);

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
  writeByteCommand_Flash(0x80);
  writeByteCommand_Flash(0x10);

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
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    // Retry writing, for when /RESET is not connected (floating)
    int dq5failcnt = 0;
    int noread = 0;

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      // if (currByte >= 0) {
      //   print_Msg(currByte);
      //   print_Msg(FS(FSTRING_SPACE));
      //   print_Msg(dq5failcnt);
      //   println_Msg(FS(FSTRING_EMPTY));
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
        writeByteCommand_Flash(0xa0);
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
        println_Msg(FS(FSTRING_EMPTY));
        dq5failcnt -= blockfailcnt;
        currByte -= 512;
        delay(100);
        noread = 1;
      } else {
        noread = 0;
      }
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
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
  writeByteCommandShift_Flash(0xf0);

  // Set data pins to input again
  dataIn8();

  delay(500);
}

void writeFlash29F1610() {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

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
      writeByteCommandShift_Flash(0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);
      }
      // update progress bar
      processedProgressBar += 128;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  }
}

void writeFlash29F1601() {
  if (openFlashFile()) {

    // Set data pins to output
    dataOut();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

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
      writeByteCommandShift_Flash(0xa0);

      // Write one full page at a time
      for (byte c = 0; c < 128; c++) {
        writeByte_Flash(currByte + c, sdBuffer[c]);

        if (c == 127) {
          // Write the last byte twice or else it won't write at all
          writeByte_Flash(currByte + c, sdBuffer[c]);
        }
      }
      // update progress bar
      processedProgressBar += 128;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Check if write is complete
    busyCheck29F1610();

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
  }
}

void idFlash29F1610() {
  // Set data pins to output
  dataOut();

  // ID command sequence
  writeByteCommandShift_Flash(0x90);

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
  writeByteCommandShift_Flash(0x70);

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
  writeByteCommandShift_Flash(0x80);
  writeByteCommandShift_Flash(0x10);

  // Set data pins to input again
  dataIn8();

  // Read the status register
  byte statusReg = readByte_Flash(0);

  while ((statusReg & 0x80) != 0x80) {
    statusReg = readByte_Flash(0);
    // Blink led
    blinkLED();
    delay(100);
  }
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
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

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
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    // Set data pins to input again
    dataIn8();
    // Close the file:
    myFile.close();
  }
}

/******************************************
  S29GL flashrom functions
*****************************************/
void writeFlash29GL(unsigned long sectorSize, byte bufferSize) {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

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
      // update progress bar
      processedProgressBar += sectorSize;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    // Set data pins to input again
    dataIn8();
    // Close the file:
    myFile.close();
  }
}

/******************************************
  29F800 functions
*****************************************/
void writeFlash29F800() {
  if (openFlashFile()) {
    // Set data pins to output
    dataOut();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    // Fill sdBuffer
    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      myFile.read(sdBuffer, 512);
      // Blink led
      if (currByte % 2048 == 0)
        blinkLED();

      for (int c = 0; c < 512; c++) {
        // Write command sequence
        writeByteCommandShift_Flash(0xa0);
        // Write current byte
        writeByte_Flash(currByte + c, sdBuffer[c]);
        busyCheck29F032(currByte + c, sdBuffer[c]);
      }
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }

    // Set data pins to input again
    dataIn8();

    // Close the file:
    myFile.close();
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
  if (openFlashFile()) {
    if ((flashid == 0xB088))
      writeFlashLH28F0XX();
    else if ((flashid == 0x8916) || (flashid == 0x8917) || (flashid == 0x8918)) {
      writeFlashE28FXXXJ3A();
    }

    myFile.close();
  }
}

void writeFlashE28FXXXJ3A() {
  uint32_t block_addr;
  uint32_t block_addr_mask = ~(sectorSize - 1);

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)fileSize;
  draw_progressbar(0, totalProgressBar);

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
    // update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

  dataIn8();
}

void writeFlashLH28F0XX() {

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = (uint32_t)fileSize;
  draw_progressbar(0, totalProgressBar);

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
    // update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
  }

  dataIn8();
}

/******************************************
  Common flashrom functions
*****************************************/
void blankcheck_Flash() {

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  blank = 1;
  for (unsigned long currBuffer = 0; currBuffer < flashSize; currBuffer += 512) {
    // Fill buffer
    for (int c = 0; c < 512; c++) {
      // Read byte
      sdBuffer[c] = readByte_Flash(currBuffer + c);
    }
    // Check if all bytes are 0xFF
    for (uint32_t currByte = 0; currByte < 512; currByte++) {
      if (sdBuffer[currByte] != 0xFF) {
        currByte = 512;
        currBuffer = flashSize;
        blank = 0;
      }
    }
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currBuffer % 25600 == 0)
      blinkLED();
  }
  if (blank) {
    println_Msg(F("Flashrom is empty"));
    display_Update();
  } else {
    println_Msg(FS(FSTRING_EMPTY));
    print_Error(F("Error: Not blank"));
  }
}

void verifyFlash() {
  verifyFlash(1, 1, 0);
}

void verifyFlash(byte currChip, byte totalChips, boolean reversed) {
  if (openVerifyFlashFile()) {
    blank = 0;

    // Adjust filesize to fit flashchip
    adjustFileSizeOffset(currChip, totalChips, reversed);

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = fileSize;
    draw_progressbar(0, totalProgressBar);

    for (unsigned long currByte = 0; currByte < fileSize; currByte += 512) {
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 8388608) && (currByte == 4194304)) {
        myFile.seekSet(0);
      }
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 6291456) && (currByte == 2097152)) {
        myFile.seekSet(0);
        currByte = 4194304;
        fileSize = 8388608;
      }

      //fill sdBuffer
      myFile.read(sdBuffer, 512);
      for (int c = 0; c < 512; c++) {
        if (readByte_Flash(currByte + c) != sdBuffer[c]) {
          blank++;
        }
      }
      // Update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
      // Blink led
      if (currByte % 25600 == 0)
        blinkLED();
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
  }
}

void readFlash() {
  // Reset to root directory
  sd.chdir("/");

  createFolderAndOpenFile("FLASH", NULL, "FL", "bin");

  //Initialize progress bar
  uint32_t processedProgressBar = 0;
  uint32_t totalProgressBar = flashSize;
  draw_progressbar(0, totalProgressBar);

  for (unsigned long currByte = 0; currByte < flashSize; currByte += 512) {
    for (int c = 0; c < 512; c++) {
      sdBuffer[c] = readByte_Flash(currByte + c);
    }
    myFile.write(sdBuffer, 512);
    // Update progress bar
    processedProgressBar += 512;
    draw_progressbar(processedProgressBar, totalProgressBar);
    // Blink led
    if (currByte % 25600 == 0)
      blinkLED();
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
      sprintf(myBuffer, "%.2x", readByte_Flash(currByte + c));
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

#ifdef ENABLE_FLASH16
/******************************************
  29L3211 16bit flashrom functions
*****************************************/
void resetFlash16() {
  // Set data pins to output
  dataOut16();

  // Reset command sequence
  writeWordCommand_Flash(0xf0);

  // Set data pins to input again
  dataIn16();

  delay(500);
}

void writeFlash16() {
  if (openFlashFile()) {

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
      writeWordCommand_Flash(0xa0);

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
  }
}

void writeFlash16_29F1601() {
  if (openFlashFile()) {
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
      writeWordCommand_Flash(0xa0);

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
  }
}

void idFlash16() {
  // Set data pins to output
  dataOut16();

  // ID command sequence
  writeWordCommand_Flash(0x90);

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
  writeWordCommand_Flash(0x70);

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
  writeWordCommand_Flash(0x80);
  writeWordCommand_Flash(0x10);

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
  if (openVerifyFlashFile()) {
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
  }
}

void readFlash16() {
  // Reset to root directory
  sd.chdir("/");

  createFolderAndOpenFile("FLASH", NULL, "FL", "bin");

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


      sprintf(buf, "%.2x", left_byte);
      // Now print the significant bits
      print_Msg(buf);

      sprintf(buf, "%.2x", right_byte);
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
  if (openFlashFile()) {
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
        writeWordCommand_Flash(0xa0);

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

  createFolderAndOpenFile("FLASH", NULL, "FL", "bin");

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
  if (openFlashFile()) {
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
  }
}

void verify_Eprom() {
  if (openVerifyFlashFile()) {
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


      sprintf(buf, "%.2x", left_byte);
      // Now print the significant bits
      print_Msg(buf);

      sprintf(buf, "%.2x", right_byte);
      // Now print the significant bits
      print_Msg(buf);
    }
    println_Msg("");
  }
  display_Update();
}

#endif

/******************************************
CFI flashrom functions (modified from GB.ino)
*****************************************/
void sendCFICommand_Flash(byte cmd) {
  writeByteCompensated_Flash(0xAAA, 0xaa);
  writeByteCompensated_Flash(0x555, 0x55);
  writeByteCompensated_Flash(0xAAA, cmd);
}

byte readByteCompensated_Flash(int address) {
  byte data = readByte_Flash(address >> (flashX16Mode ? 1 : 0));
  if (flashSwitchLastBits) {
    return (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  return data;
}

void writeByteCompensated_Flash(int address, byte data) {
  if (flashSwitchLastBits) {
    data = (data & 0b11111100) | ((data << 1) & 0b10) | ((data >> 1) & 0b01);
  }
  writeByte_Flash(address >> (flashX16Mode ? 1 : 0), data);
}

void startCFIMode_Flash(boolean x16Mode) {
  if (x16Mode) {
    writeByte_Flash(0x555, 0xf0);  //x16 mode reset command
    delay(500);
    writeByte_Flash(0x555, 0xf0);  //Double reset to get out of possible Autoselect + CFI mode
    delay(500);
    writeByte_Flash(0x55, 0x98);  //x16 CFI Query command
  } else {
    writeByte_Flash(0xAAA, 0xf0);  //x8  mode reset command
    delay(100);
    writeByte_Flash(0xAAA, 0xf0);  //Double reset to get out of possible Autoselect + CFI mode
    delay(100);
    writeByte_Flash(0xAA, 0x98);  //x8 CFI Query command
  }
}

void identifyCFI_Flash() {
  display_Clear();

  // Reset flash
  dataOut();
  writeByteCompensated_Flash(0xAAA, 0xf0);
  delay(100);

  // Trying x8 mode first
  startCFIMode_Flash(false);
  dataIn8();

  char cfiQRYx8[7];
  char cfiQRYx16[7];
  sprintf(cfiQRYx8, "%02X%02X%02X", readByte_Flash(0x20), readByte_Flash(0x22), readByte_Flash(0x24));
  sprintf(cfiQRYx16, "%02X%02X%02X", readByte_Flash(0x10), readByte_Flash(0x11), readByte_Flash(0x12));

  if (strcmp(cfiQRYx8, "515259") == 0) {
    println_Msg(F("Normal CFI x8 Mode"));
    flashX16Mode = false;
    flashSwitchLastBits = false;
  } else if (strcmp(cfiQRYx8, "52515A") == 0) {  // QRY in x8 mode with switched last bit
    println_Msg(F("Switched CFI x8 Mode"));
    flashX16Mode = false;
    flashSwitchLastBits = true;
  } else if (strcmp(cfiQRYx16, "515259") == 0) {  // QRY in x16 mode
    println_Msg(F("Normal CFI x16 Mode"));
    flashX16Mode = true;
    flashSwitchLastBits = false;
  } else if (strcmp(cfiQRYx16, "52515A") == 0) {  // QRY in x16 mode with switched last bit
    println_Msg(F("Switched CFI x16 Mode"));
    flashX16Mode = true;
    flashSwitchLastBits = true;
  } else {
    // Try x16 mode next
    startCFIMode_Flash(true);
    sprintf(cfiQRYx16, "%02X%02X%02X", readByte_Flash(0x10), readByte_Flash(0x11), readByte_Flash(0x12));
    if (strcmp(cfiQRYx16, "515259") == 0) {  // QRY in x16 mode
      println_Msg(F("Normal CFI x16 Mode"));
      flashX16Mode = true;
      flashSwitchLastBits = false;
    } else if (strcmp(cfiQRYx16, "52515A") == 0) {  // QRY in x16 mode with switched last bit
      println_Msg(F("Switched CFI x16 Mode"));
      flashX16Mode = true;
      flashSwitchLastBits = true;
    } else {
      println_Msg(F("CFI Query failed!"));
      print_STR(press_button_STR, 0);
      display_Update();
      wait();
      resetArduino();
      return;
    }
  }
  flashBanks = 1 << (readByteCompensated_Flash(0x4E) - 14);  // - flashX16Mode);

  // Reset flash
  dataOut();
  writeByteCompensated_Flash(0xAAA, 0xf0);
  dataIn8();
  delay(100);
  display_Update();
}

// Adjust file size to fit flash chip and goto needed file offset
void adjustFileSizeOffset(byte currChip, byte totalChips, boolean reversed) {
  // 1*2MB, 1*4MB or 1*8MB
  if ((currChip == 1) && (totalChips == 1)) {
    if (reversed)
      myFile.seekSet(4194304);
  }

  // 2*2MB or 2*4MB
  else if ((currChip == 1) && (totalChips == 2)) {
    if (reversed) {
      if (fileSize > 4194304) {
        fileSize = fileSize - flashSize / 2;
        myFile.seekSet(4194304);
      } else
        fileSize = 0;
    } else if (fileSize > flashSize / 2)
      fileSize = flashSize / 2;

  } else if ((currChip == 2) && (totalChips == 2)) {
    if (reversed) {
      fileSize = flashSize / 2;
      myFile.seekSet(0);
    } else if (fileSize > flashSize / 2) {
      fileSize = fileSize - flashSize / 2;
      myFile.seekSet(flashSize / 2);
    } else
      fileSize = 0;
  }

  // 4*2MB
  else if ((currChip == 1) && (totalChips == 4)) {
    if (reversed) {
      if (fileSize > 4194304) {
        myFile.seekSet(4194304);
        fileSize = 2097152;
      } else
        fileSize = 0;
    } else if (fileSize > 2097152)
      fileSize = 2097152;

  } else if ((currChip == 2) && (totalChips == 4)) {
    if (reversed) {
      if (fileSize > 6291456) {
        myFile.seekSet(6291456);
        fileSize = 2097152;
      } else
        fileSize = 0;
    } else {
      if (fileSize > 2097152) {
        myFile.seekSet(2097152);
        fileSize = 2097152;
      } else
        fileSize = 0;
    }

  } else if ((currChip == 3) && (totalChips == 4)) {
    if (reversed) {
      myFile.seekSet(0);
      fileSize = 2097152;
    } else {
      if (fileSize > 4194304) {
        myFile.seekSet(4194304);
        fileSize = 2097152;
      } else
        fileSize = 0;
    }

  } else if ((currChip == 4) && (totalChips == 4)) {
    if (reversed) {
      if (fileSize > 2097152) {
        myFile.seekSet(2097152);
        fileSize = 2097152;
      } else
        fileSize = 0;
    } else {
      if (fileSize > 6291456) {
        myFile.seekSet(6291456);
        fileSize = 2097152;
      } else
        fileSize = 0;
    }
  }
  // skip write
  else
    fileSize = 0;
}

// Write flashrom
void writeCFI_Flash(byte currChip, byte totalChips, boolean reversed) {
  if (openFileOnSD()) {
    // Print filepath
    print_STR(flashing_file_STR, 0);
    print_Msg(filePath);
    println_Msg(F("..."));
    // Check size
    if ((flashSize == 8388608) && (fileSize < 6291456) && reversed) {
      println_Msg(FS(FSTRING_EMPTY));
      print_STR(error_STR, 0);
      print_FatalError(F("ROM file not ExROM"));
    }
    display_Update();

    // Reset flash
    dataOut();
    writeByteCompensated_Flash(0xAAA, 0xf0);
    dataIn8();
    delay(100);

    // Reset flash
    dataOut();
    writeByte_Flash(0x555, 0xf0);
    dataIn8();

    delay(100);
    println_Msg(F("Erasing..."));
    display_Update();

    // Erase flash
    dataOut();
    sendCFICommand_Flash(0x80);
    sendCFICommand_Flash(0x10);
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

    // Adjust filesize to fit flashchip
    adjustFileSizeOffset(currChip, totalChips, reversed);

    print_Msg(F("Writing flash"));

    // File offset indicator for SNES repros with multiple chips
    if ((totalChips > 1) || reversed) {
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(currChip);
      print_Msg(F("/"));
      print_Msg(totalChips);
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));
      print_Msg(FS(FSTRING_SPACE));

      switch (myFile.curPosition() / 1024 / 1024UL) {
        case 0:
          println_Msg(F("[A]BCD"));
          break;

        case 2:
          println_Msg(F("A[B]CD"));
          break;

        case 4:
          println_Msg(F("AB[C]D"));
          break;

        case 6:
          println_Msg(F("ABC[D]"));
          break;

        default:
          println_Msg(FS(FSTRING_SPACE));
          break;
      }
    } else
      println_Msg(F("..."));
    display_Update();

    //Initialize progress bar
    uint32_t processedProgressBar = 0;
    uint32_t totalProgressBar = (uint32_t)fileSize;
    draw_progressbar(0, totalProgressBar);

    for (unsigned long currAddr = 0; currAddr < fileSize; currAddr += 512) {
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 8388608) && (currAddr == 4194304)) {
        myFile.seekSet(0);
      }
      if ((reversed) && (currChip == 1) && (totalChips == 1) && (fileSize == 6291456) && (currAddr == 2097152)) {
        myFile.seekSet(0);
        currAddr = 4194304;
        fileSize = 8388608;
      }

      myFile.read(sdBuffer, 512);

      // Blink led
      if (currAddr % 4096 == 0)
        blinkLED();

      for (int currByte = 0; currByte < 512; currByte++) {
        // Write command sequence
        dataOut();
        sendCFICommand_Flash(0xa0);

        // Write current byte
        writeByte_Flash(currAddr + currByte, sdBuffer[currByte]);

        dataIn8();

        // Read the status register
        byte statusReg = readByte_Flash(currAddr + currByte);
        while ((statusReg & 0x80) != (sdBuffer[currByte] & 0x80)) {
          statusReg = readByte_Flash(currAddr + currByte);
        }
      }
      // update progress bar
      processedProgressBar += 512;
      draw_progressbar(processedProgressBar, totalProgressBar);
    }
    // Close the file:
    myFile.close();
  }
  // Reset flash
  dataOut();
  writeByteCompensated_Flash(0xAAA, 0xf0);
  delay(100);
  dataIn8();
}
#endif

//******************************************
// End of File
//******************************************
