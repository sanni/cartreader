//******************************************
// GB SMART MODULE
// Supports 32M cart with LH28F016SUT flash
//******************************************
#ifdef ENABLE_GBX
#if defined(ENABLE_FLASH)
#define GB_SMART_GAMES_PER_PAGE 6

/******************************************
   Menu
 *****************************************/
// GB Smart menu items
static const char gbSmartMenuItem1[] PROGMEM = "Game Menu";
static const char gbSmartMenuItem2[] PROGMEM = "Flash Menu";
static const char *const menuOptionsGBSmart[] PROGMEM = { gbSmartMenuItem1, gbSmartMenuItem2, FSTRING_RESET };

static const char gbSmartFlashMenuItem1[] PROGMEM = "Read Flash";
static const char gbSmartFlashMenuItem2[] PROGMEM = "Write Flash";
static const char gbSmartFlashMenuItem3[] PROGMEM = "Back";
static const char *const menuOptionsGBSmartFlash[] PROGMEM = { gbSmartFlashMenuItem1, gbSmartFlashMenuItem2, gbSmartFlashMenuItem3 };

static const char gbSmartGameMenuItem1[] PROGMEM = "Read Game";
static const char gbSmartGameMenuItem4[] PROGMEM = "Switch Game";
static const char *const menuOptionsGBSmartGame[] PROGMEM = { gbSmartGameMenuItem1, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, gbSmartGameMenuItem4, FSTRING_RESET };

struct GBSmartGameInfo {
  uint8_t start_bank;
  uint8_t rom_type;
  uint8_t rom_size;
  uint8_t sram_size;
  char title[16];
};

uint32_t gbSmartSize = 32 * 131072;
uint16_t gbSmartBanks = 256;

uint8_t gbSmartBanksPerFlashChip = 128;
uint8_t gbSmartBanksPerFlashBlock = 4;
uint32_t gbSmartFlashBlockSize = (gbSmartBanksPerFlashBlock << 14);

uint8_t gbSmartRomSizeGB = 0x07;
uint8_t gbSmartSramSizeGB = 0x04;
uint8_t gbSmartFlashSizeGB = 0x06;

byte signature[48];
uint16_t gameMenuStartBank;

// Compare checksum
boolean compare_checksum_GBS() {
  println_Msg(F("Calculating Checksum"));
  display_Update();

  strcpy(fileName, romName);
  strcat(fileName, ".GB");

  // last used rom folder
  EEPROM_readAnything(0, foldern);
  sprintf(folder, "GB/ROM/%s/%d", romName, foldern - 1);

  char calcsumStr[5];
  sprintf(calcsumStr, "%04X", calc_checksum_GB(fileName));

  if (strcmp(calcsumStr, checksumStr) == 0) {
    print_Msg(F("Result: "));
    println_Msg(calcsumStr);
    println_Msg(F("Checksum matches"));
    display_Update();
    return 1;
  } else {
    print_Msg(F("Result: "));
    println_Msg(calcsumStr);
    print_Error(F("Checksum Error"));
    return 0;
  }
}

byte readByte_GBS(word myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Switch CS(PH3) and RD(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Read
  byte tempByte = PINC;

  // Switch CS(PH3) and RD(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

void setup_GBSmart() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // take from setup_GB
  // Set RST(PH0) to Input
  DDRH &= ~(1 << 0);
  // Activate Internal Pullup Resistors
  PORTH |= (1 << 0);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;

  // Set Control Pins to Output CS(PH3) WR(PH5) RD(PH6) AUDIOIN(PH4) RESET(PH0)
  DDRH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // Output a high signal on all pins, pins are active low therefore everything is disabled now
  PORTH |= (1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D7) to Input
  DDRC = 0x00;

  delay(400);

  gbSmartRemapStartBank(0x00, 0x00, 0x00);

  getCartInfo_GB();

  for (byte i = 0; i < 0x30; i++)
    signature[i] = readByte_GBS(0x0104 + i);

  gameMenuStartBank = 0x02;

  display_Clear();
  display_Update();
}

void gbSmartMenu() {
  uint8_t mainMenu;

  // Copy menuOptions out of progmem
  convertPgm(menuOptionsGBSmart, 3);
  mainMenu = question_box(F("GB Smart"), menuOptions, 3, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu) {
    case 0:
      {
        gbSmartGameMenu();
        break;
      }
    case 1:
      {
        mode = CORE_GB_GBSMART_FLASH;
        break;
      }
    default:
      {
        asm volatile("  jmp 0");
        break;
      }
  }
}

void gbSmartGameOptions() {
  uint8_t gameSubMenu;

  convertPgm(menuOptionsGBSmartGame, 5);
  gameSubMenu = question_box(F("GB Smart Game Menu"), menuOptions, 5, 0);

  switch (gameSubMenu) {
    case 0:  // Read Game
      {
        display_Clear();
        sd.chdir("/");
        readROM_GB();
        compare_checksum_GBS();
        break;
      }
    case 1:  // Read SRAM
      {
        display_Clear();
        sd.chdir("/");
        readSRAM_GB();
        break;
      }
    case 2:  // Write SRAM
      {
        display_Clear();
        sd.chdir("/");
        writeSRAM_GB();
        uint32_t wrErrors = verifySRAM_GB();
        if (wrErrors == 0) {
          println_Msg(F("Verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(wrErrors);
          println_Msg(F(" bytes"));
          print_Error(did_not_verify_STR);
        }
        break;
      }
    case 3:  // Switch Game
      {
        gameMenuStartBank = 0x02;
        gbSmartGameMenu();
        break;
      }
    default:
      {
        asm volatile("  jmp 0");
        break;
      }
  }

  if (gameSubMenu != 3) {
    println_Msg(FS(FSTRING_EMPTY));
    // Prints string out of the common strings array either with or without newline
    print_STR(press_button_STR, 1);
    display_Update();
    wait();
  }
}

void gbSmartGameMenu() {
  boolean hasMenu;
  byte numGames;
  struct GBSmartGameInfo gbSmartGames[GB_SMART_GAMES_PER_PAGE];
  uint8_t gameSubMenu = 0;
gb_smart_load_more_games:
  if (gameMenuStartBank > 0xfe)
    gameMenuStartBank = 0x02;

  gbSmartGetGames(gbSmartGames, &hasMenu, &numGames);

  if (hasMenu) {
    char menuOptionsGBSmartGames[7][20];
    int i = 0;
    for (; i < numGames; i++)
      strncpy(menuOptionsGBSmartGames[i], gbSmartGames[i].title, 16);

    strncpy(menuOptionsGBSmartGames[i], "...", 16);
    gameSubMenu = question_box(F("Select Game"), menuOptionsGBSmartGames, i + 1, 0);

    if (gameSubMenu >= i)
      goto gb_smart_load_more_games;
  } else {
    gameSubMenu = 0;
  }

  // copy romname
  strcpy(romName, gbSmartGames[gameSubMenu].title);

  // select a game
  gbSmartRemapStartBank(gbSmartGames[gameSubMenu].start_bank, gbSmartGames[gameSubMenu].rom_size, gbSmartGames[gameSubMenu].sram_size);
  getCartInfo_GB();
  showCartInfo_GB();

  mode = CORE_GB_GBSMART_GAME;
}

void gbSmartFlashMenu() {
  uint8_t flashSubMenu;

  convertPgm(menuOptionsGBSmartFlash, 3);
  flashSubMenu = question_box(F("GB Smart Flash Menu"), menuOptions, 3, 0);

  switch (flashSubMenu) {
    case 0:
      {
        // read flash
        display_Clear();
        sd.chdir("/");

        EEPROM_readAnything(0, foldern);
        sprintf(fileName, "GBS%d.bin", foldern);
        sd.mkdir("GB/GBS", true);
        sd.chdir("GB/GBS");
        foldern = foldern + 1;
        EEPROM_writeAnything(0, foldern);

        gbSmartReadFlash();
        break;
      }
    case 1:
      {
        // write flash
        display_Clear();

        println_Msg(F("Attention"));
        println_Msg(F("This will erase your"));
        println_Msg(F("GB Smart Cartridge."));
        println_Msg(FS(FSTRING_EMPTY));
        // Prints string out of the common strings array either with or without newline
        print_STR(press_button_STR, 1);
        display_Update();
        wait();

        display_Clear();
        filePath[0] = '\0';
        sd.chdir("/");
        fileBrowser(F("Select 4MB file"));

        sprintf(filePath, "%s/%s", filePath, fileName);
        gbSmartWriteFlash();
        break;
      }
    default:
      {
        mode = CORE_GB_GBSMART;
        return;
      }
  }

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

void gbSmartGetOneGame(struct GBSmartGameInfo *gbSmartGames, byte bank, word base) {
  uint8_t myByte, myLength = 0;
  word title_address = base + 0x0134;

  for (uint8_t j = 0; j < 15; j++) {
    myByte = readByte_GBS(title_address++);

    if (((myByte >= '0' && myByte <= '9') || (myByte >= 'A' && myByte <= 'z')))
      gbSmartGames->title[myLength++] = myByte;
  }

  gbSmartGames->title[myLength] = 0x00;
  gbSmartGames->start_bank = bank;
  gbSmartGames->rom_type = readByte_GBS(base + 0x0147);
  gbSmartGames->rom_size = readByte_GBS(base + 0x0148);
  gbSmartGames->sram_size = readByte_GBS(base + 0x0149);
}

void gbSmartGetGames(struct GBSmartGameInfo *gbSmartGames, boolean *hasMenu, byte *numGames) {
  static const byte menu_title[] = { 0x47, 0x42, 0x31, 0x36, 0x4d };

  // reset remap setting
  gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

  uint16_t i;

  // check if contain menu
  *hasMenu = true;
  dataIn();
  for (i = 0; i < 5; i++) {
    if (readByte_GBS(0x0134 + i) != menu_title[i]) {
      *hasMenu = false;
      break;
    }
  }

  if (*hasMenu) {
    for (i = gameMenuStartBank, *numGames = 0; i < gbSmartBanks && *numGames < GB_SMART_GAMES_PER_PAGE;) {
      // switch bank
      dataOut();
      writeByte_GB(0x2100, i);

      dataIn();
      // read signature
      for (uint8_t j = 0x00; j < 0x30; j++) {
        if (readByte_GBS(0x4104 + j) != signature[j]) {
          i += 0x02;
          continue;
        }
      }
      gbSmartGetOneGame(&gbSmartGames[*numGames], i, 0x4000);
      i += (2 << gbSmartGames[(*numGames)++].rom_size);
    }

    gameMenuStartBank = i;
  } else {
    dataIn();
    gbSmartGetOneGame(&gbSmartGames[0], 0, 0);

    *numGames = 1;
    gameMenuStartBank = 0xfe;
  }
}

void gbSmartReadFlash() {
  print_Msg(F("Saving as GB/GBS/"));
  print_Msg(fileName);
  println_Msg(F("..."));
  display_Update();

  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(create_file_STR);

  // reset flash to read array state
  for (uint16_t i = 0x00; i < gbSmartBanks; i += gbSmartBanksPerFlashChip)
    gbSmartResetFlash(i);

  // remaps mmc to full access
  gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

  // dump fixed bank 0x00
  dataIn();
  for (uint16_t addr = 0x0000; addr <= 0x3fff; addr += 512) {
    for (uint16_t c = 0; c < 512; c++)
      sdBuffer[c] = readByte_GBS(addr + c);

    myFile.write(sdBuffer, 512);
  }

  // read rest banks
  for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++) {
    dataOut();
    writeByte_GB(0x2100, bank);

    dataIn();
    for (uint16_t addr = 0x4000; addr <= 0x7fff; addr += 512) {
      for (uint16_t c = 0; c < 512; c++)
        sdBuffer[c] = readByte_GBS(addr + c);

      myFile.write(sdBuffer, 512);
    }
  }

  // back to initial state
  writeByte_GB(0x2100, 0x01);

  myFile.close();
  println_Msg("");
  println_Msg(F("Finished reading"));
  display_Update();
}

void gbSmartWriteFlash() {
  for (uint16_t bank = 0x00; bank < gbSmartBanks; bank += gbSmartBanksPerFlashChip) {
    display_Clear();

    print_Msg(F("Erasing..."));
    display_Update();

    gbSmartEraseFlash(bank);
    gbSmartResetFlash(bank);

    print_STR(done_STR, 1);
    print_Msg(F("Blankcheck..."));
    display_Update();

    if (!gbSmartBlankCheckingFlash(bank))
      print_FatalError(F("Could not erase flash"));

    println_Msg(F("Passed"));
    display_Update();

    // write full chip
    gbSmartWriteFlash(bank);

    // reset chip
    gbSmartWriteFlashByte(0x0000, 0xff);
  }

  print_STR(verifying_STR, 0);
  display_Update();

  writeErrors = gbSmartVerifyFlash();
  if (writeErrors == 0) {
    println_Msg(FS(FSTRING_OK));
    display_Update();
  } else {
    print_STR(error_STR, 0);
    print_Msg(writeErrors);
    print_STR(_bytes_STR, 1);
    print_FatalError(did_not_verify_STR);
  }
}

void gbSmartWriteFlash(uint32_t start_bank) {
  if (!myFile.open(filePath, O_READ))
    print_FatalError(open_file_STR);

  // switch to flash base bank
  gbSmartRemapStartBank(start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  myFile.seekCur((start_bank << 14));

  print_Msg(F("Writing Bank 0x"));
  print_Msg(start_bank, HEX);
  print_Msg(F("..."));
  display_Update();

  // handle bank 0x00 on 0x0000
  gbSmartWriteFlashFromMyFile(0x0000);

  // handle rest banks on 0x4000
  for (uint8_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++) {
    dataOut();
    writeByte_GB(0x2100, bank);

    gbSmartWriteFlashFromMyFile(0x4000);
  }

  myFile.close();
  println_Msg("");
}

void gbSmartWriteFlashFromMyFile(uint32_t addr) {
  for (uint16_t i = 0; i < 16384; i += 256) {
    myFile.read(sdBuffer, 256);

    // sequence load to page
    dataOut();
    gbSmartWriteFlashByte(addr, 0xe0);
    gbSmartWriteFlashByte(addr, 0xff);
    gbSmartWriteFlashByte(addr, 0x00);  // BCH should be 0x00

    // fill page buffer
    for (int d = 0; d < 256; d++)
      gbSmartWriteFlashByte(d, sdBuffer[d]);

    // start flashing page
    gbSmartWriteFlashByte(addr, 0x0c);
    gbSmartWriteFlashByte(addr, 0xff);
    gbSmartWriteFlashByte(addr + i, 0x00);  // BCH should be 0x00

    // waiting for finishing
    dataIn();
    while ((readByte_GBS(addr + i) & 0x80) == 0x00)
      ;
  }

  // blink LED
  blinkLED();
}

uint32_t gbSmartVerifyFlash() {
  uint32_t verified = 0;

  if (!myFile.open(filePath, O_READ)) {
    verified = 0xffffffff;
    print_Error(F("Can't open file on SD"));
  } else {
    // remaps mmc to full access
    gbSmartRemapStartBank(0x00, gbSmartRomSizeGB, gbSmartSramSizeGB);

    // verify bank 0x00
    dataIn();
    for (uint16_t addr = 0x0000; addr <= 0x3fff; addr += 512) {
      myFile.read(sdBuffer, 512);

      for (uint16_t c = 0; c < 512; c++) {
        if (readByte_GBS(addr + c) != sdBuffer[c])
          verified++;
      }
    }

    // verify rest banks
    for (uint16_t bank = 0x01; bank < gbSmartBanks; bank++) {
      dataOut();
      writeByte_GB(0x2100, bank);

      dataIn();
      for (uint16_t addr = 0x4000; addr <= 0x7fff; addr += 512) {
        myFile.read(sdBuffer, 512);

        for (uint16_t c = 0; c < 512; c++) {
          if (readByte_GBS(addr + c) != sdBuffer[c])
            verified++;
        }
      }
    }

    // back to initial state
    writeByte_GB(0x2100, 0x01);

    myFile.close();
  }

  return verified;
}

byte gbSmartBlankCheckingFlash(uint8_t flash_start_bank) {
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  // check first bank
  dataIn();
  for (uint16_t addr = 0x0000; addr <= 0x3fff; addr++) {
    if (readByte_GBS(addr) != 0xff)
      return 0;
  }

  // check rest banks
  for (uint16_t bank = 0x01; bank < gbSmartBanksPerFlashChip; bank++) {
    dataOut();
    writeByte_GB(0x2100, bank);

    dataIn();
    for (uint16_t addr = 0x4000; addr <= 0x7fff; addr++) {
      if (readByte_GBS(addr) != 0xff)
        return 0;
    }
  }

  return 1;
}

void gbSmartResetFlash(uint8_t flash_start_bank) {
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  dataOut();
  gbSmartWriteFlashByte(0x0, 0xff);
}

void gbSmartEraseFlash(uint8_t flash_start_bank) {
  gbSmartRemapStartBank(flash_start_bank, gbSmartFlashSizeGB, gbSmartSramSizeGB);

  // handling first flash block
  dataOut();
  gbSmartWriteFlashByte(0x0000, 0x20);
  gbSmartWriteFlashByte(0x0000, 0xd0);

  dataIn();
  while ((readByte_GBS(0x0000) & 0x80) == 0x00)
    ;

  // blink LED
  blinkLED();

  // rest of flash block
  for (uint32_t ba = gbSmartBanksPerFlashBlock; ba < gbSmartBanksPerFlashChip; ba += gbSmartBanksPerFlashBlock) {
    dataOut();
    writeByte_GB(0x2100, ba);

    gbSmartWriteFlashByte(0x4000, 0x20);
    gbSmartWriteFlashByte(0x4000, 0xd0);

    dataIn();
    while ((readByte_GBS(0x4000) & 0x80) == 0x00)
      ;

    // blink LED
    blinkLED();
  }
}

void gbSmartWriteFlashByte(uint32_t myAddress, uint8_t myData) {
  PORTF = myAddress & 0xff;
  PORTK = (myAddress >> 8) & 0xff;
  PORTC = myData;

  // wait for 62.5 x 4 = 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");

  // Pull FLASH_WE (PH4) low
  PORTH &= ~(1 << 4);

  // pull low for another 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");

  // Pull FLASH_WE (PH4) high
  PORTH |= (1 << 4);

  // pull high for another 250ns
  __asm__("nop\n\tnop\n\tnop\n\tnop\n\t");
}

// rom_start_bank = 0x00 means back to original state
void gbSmartRemapStartBank(uint8_t rom_start_bank, uint8_t rom_size, uint8_t sram_size) {
  rom_start_bank &= 0xfe;

  dataOut();

  // clear base bank setting
  writeByte_GB(0x1000, 0xa5);
  writeByte_GB(0x7000, 0x00);
  writeByte_GB(0x1000, 0x98);
  writeByte_GB(0x2000, rom_start_bank);

  if (rom_start_bank > 1) {
    // start set new base bank
    writeByte_GB(0x1000, 0xa5);

    dataIn();
    rom_start_bank = gbSmartGetResizeParam(rom_size, sram_size);

    dataOut();
    writeByte_GB(0x7000, rom_start_bank);
    writeByte_GB(0x1000, 0x98);

    writeByte_GB(0x2100, 0x01);
  }

  dataIn();
}

// Get magic number for 0x7000 register.
// Use for setting correct rom and sram size
// Code logic is take from SmartCard32M V1.3 menu code,
// see 0x2db2 to 0x2e51 (0xa0 bytes)
uint8_t gbSmartGetResizeParam(uint8_t rom_size, uint8_t sram_size) {
  if (rom_size < 0x0f) {
    rom_size &= 0x07;
    rom_size ^= 0x07;
  } else {
    rom_size = 0x01;
  }

  if (sram_size > 0) {
    if (sram_size > 1) {
      sram_size--;
      sram_size ^= 0x03;
      sram_size <<= 4;
      sram_size &= 0x30;
    } else {
      sram_size = 0x20;  //  2KiB treat as 8KiB
    }
  } else {
    sram_size = 0x30;  // no sram
  }

  return (sram_size | rom_size);
}

#endif
#endif
//******************************************
// End of File
//******************************************