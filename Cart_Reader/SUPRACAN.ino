//******************************************
// Super A'can MODULE
// Only tested with HW3 and HW5
//******************************************
#ifdef ENABLE_SUPRACAN

/******************************************
  Menu
*****************************************/
static const char acanMenuItem4[] PROGMEM = "Read UM6650";
static const char acanMenuItem5[] PROGMEM = "Write UM6650";
static const char acanMenuItem6[] PROGMEM = "Flash repro";

static const char *const menuOptionsAcan[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, acanMenuItem4, acanMenuItem5, FSTRING_RESET, acanMenuItem6 };

void setup_SuprAcan() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // addr as output
  DDRF = 0xff;  // A0 - A7
  DDRK = 0xff;  // A8 - A15
  DDRL = 0xff;  // A16 - A23

  // data as input
  DDRC = 0xff;
  DDRA = 0xff;
  PORTC = 0x00;  // disable internal pull-up
  PORTA = 0x00;
  DDRC = 0x00;  // D0 - D7
  DDRA = 0x00;  // D8 - D15

  // set /RST(PH0), /CS(PH3), C27(PH4), R/W(PH5), /AS(PH6) output
  DDRH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
  PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
  PORTH &= ~((1 << 0) | (1 << 4));

  // set 6619_124(PE4) input
  DDRE &= ~(1 << 4);

  // detect if flash chip exists
  PORTG |= (1 << 5);
  DDRG |= (1 << 5);
  PORTG |= (1 << 5);

  dataOut_MD();
  writeCommand_Acan(0, 0x9090);

  dataIn_MD();
  eepbit[0] = readWord_Acan(0x2);
  eepbit[1] = readWord_Acan(0x0);

  dataOut_MD();
  writeWord_Acan(0x0, 0xf0f0);

  dataIn_MD();
  // set /CARTIN(PG5) input
  PORTG &= ~(1 << 5);
  DDRG &= ~(1 << 5);

  *((uint32_t *)(eepbit + 4)) = getFlashChipSize_Acan(*((uint16_t *)eepbit));

  display_Clear();
  initializeClockOffset();

  // clockgen
  if (i2c_found) {
    clockgen.set_freq(1073863500ULL, SI5351_CLK1);  // cpu
    clockgen.set_freq(357954500ULL, SI5351_CLK2);   // subcarrier
    clockgen.set_freq(5369317500ULL, SI5351_CLK0);  // master clock

    clockgen.output_enable(SI5351_CLK1, 1);
    clockgen.output_enable(SI5351_CLK2, 1);
    clockgen.output_enable(SI5351_CLK0, 0);

    // Wait for clock generator
    clockgen.update_status();
    delay(500);
  }
#ifdef ENABLE_CLOCKGEN
  else {
    print_FatalError(F("Clock Generator not found"));
  }
#endif

  checkRomExist_Acan();
  wait();
}

void suprAcanMenu() {
  uint8_t mainMenu = 6;

  if (*((uint32_t *)(eepbit + 4)) > 0)
    mainMenu = 7;

  convertPgm(menuOptionsAcan, mainMenu);
  mainMenu = question_box(F("Super A'can Menu"), menuOptions, mainMenu, 0);

  switch (mainMenu) {
    case 0:
      {
        readROM_Acan();
        break;
      }
    case 1:
      {
        readSRAM_Acan();
        break;
      }
    case 2:
      {
        writeSRAM_Acan();
        verifySRAM_Acan();
        break;
      }
    case 3:
      {
        readUM6650();
        break;
      }
    case 4:
      {
        writeUM6650();
        verifyUM6650();
        break;
      }
    case 6:
      {
        flashCart_Acan();
        break;
      }
    default:
      {
        resetCart_Acan();
        resetArduino();
        break;
      }
  }

  println_Msg(FS(FSTRING_EMPTY));
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

static void writeCommand_Acan(uint32_t offset, uint16_t command) {
  writeWord_Acan(offset + 0xaaaa, 0xaaaa);
  writeWord_Acan(offset + 0x5555, 0x5555);
  writeWord_Acan(offset + 0xaaaa, command);
}

static void openFile_Acan() {
  filePath[0] = 0;
  sd.chdir();
  fileBrowser(FS(FSTRING_SELECT_FILE));
  snprintf(filePath, FILEPATH_LENGTH, "%s/%s", filePath, fileName);

  display_Clear();

  if (!myFile.open(filePath, O_READ)) {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
    return;
  }
}

static void readROM_Acan() {
  uint32_t crc32 = 0xffffffff;

  createFolderAndOpenFile("/ACAN", "ROM", "rom", "bin");

  draw_progressbar(0, cartSize);

  dataIn_MD();
  for (uint32_t addr = 0; addr < cartSize; addr += 512, draw_progressbar(addr, cartSize)) {
    for (uint32_t i = 0; i < 512; i += 2) {
      *((uint16_t *)(sdBuffer + i)) = readWord_Acan(addr + i);
      UPDATE_CRC(crc32, sdBuffer[i]);
      UPDATE_CRC(crc32, sdBuffer[i + 1]);
    }

    myFile.write(sdBuffer, 512);

    if ((addr & ((1 << 14) - 1)) == 0)
      blinkLED();
  }

  crc32 = ~crc32;
  myFile.close();

  print_Msg(F("CRC32: "));
  print_Msg_PaddedHex32(crc32);
  println_Msg(FS(FSTRING_EMPTY));
  print_STR(done_STR, 1);
}

static void readSRAM_Acan() {
  createFolderAndOpenFile("/ACAN", "SAVE", "save", "bin");

  dataIn_MD();
  for (uint32_t i = 0; i < 0x10000; i += 1024) {
    for (uint32_t j = 0; j < 1024; j += 2)
      sdBuffer[(j >> 1)] = readWord_Acan(0xec0000 + i + j);

    myFile.write(sdBuffer, 512);
  }

  myFile.close();
  print_STR(done_STR, 1);
}

static void writeSRAM_Acan() {
  openFile_Acan();

  print_Msg(F("Writing "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  dataOut_MD();
  for (uint32_t i = 0; i < 0x10000 && myFile.available(); i += 1024) {
    myFile.read(sdBuffer, 512);

    for (uint32_t j = 0; j < 1024; j += 2)
      writeWord_Acan(0xec0000 + i + j, sdBuffer[(j >> 1)]);
  }

  myFile.close();

  dataIn_MD();
  print_STR(done_STR, 1);
}

static void verifySRAM_Acan() {
  print_STR(verifying_STR, 0);
  display_Update();

  if (!myFile.open(filePath, O_READ)) {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
    return;
  }

  uint16_t write_errors = 0;

  dataIn_MD();
  for (uint32_t i = 0; i < 0x10000 && myFile.available(); i += 1024) {
    myFile.read(sdBuffer, 512);

    for (uint32_t j = 0; j < 1024; j += 2) {
      if (readWord_Acan(0xec0000 + i + j) != sdBuffer[(j >> 1)])
        write_errors++;
    }
  }

  myFile.close();

  if (write_errors == 0) {
    println_Msg(F("passed"));
  } else {
    println_Msg(F("failed"));
    print_Msg(F("Error: "));
    print_Msg(write_errors);
    println_Msg(F(" bytes "));
    print_Error(did_not_verify_STR);
  }
}

static void readUM6650() {
  createFolderAndOpenFile("/ACAN", "UM6650", "UM6650", "bin");

  for (uint16_t i = 0; i < 256; i++) {
    dataOut_MD();
    writeWord_Acan(0xeb0d03, i);

    dataIn_MD();
    sdBuffer[i] = readWord_Acan(0xeb0d01);
  }

  myFile.write(sdBuffer, 256);
  myFile.close();

  print_STR(done_STR, 1);
}

static void verifyUM6650() {
  print_STR(verifying_STR, 0);
  display_Update();

  if (!myFile.open(filePath, O_READ)) {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
    return;
  }

  uint16_t write_errors = 0;
  uint16_t len = myFile.read(sdBuffer, 256);
  myFile.close();

  for (uint16_t i = 0; i < len; i++) {
    dataOut_MD();
    writeWord_Acan(0xeb0d03, i);

    dataIn_MD();
    if (readWord_Acan(0xeb0d01) != sdBuffer[i])
      write_errors++;
  }

  if (write_errors) {
    println_Msg(F("failed"));
    print_Msg(F("Error: "));
    print_Msg(write_errors);
    println_Msg(F(" bytes "));
    print_Error(did_not_verify_STR);
  } else {
    println_Msg(F("passed"));
  }
}

static void writeUM6650() {
  openFile_Acan();

  uint16_t len = myFile.read(sdBuffer, 256);
  myFile.close();

  print_Msg(F("Writing "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  dataOut_MD();
  for (uint16_t i = 0; i < len; i++) {
    writeWord_Acan(0xeb0d03, i);
    writeWord_Acan(0xeb0d01, sdBuffer[i]);

    delay(10);  // for AT28C64B write
  }

  dataIn_MD();
  print_STR(done_STR, 1);
}

static void flashCart_Acan() {
  uint32_t *flash_size = (uint32_t *)(eepbit + 4);

  openFile_Acan();

  print_Msg(F("Writing "));
  print_Msg(filePath + 1);
  println_Msg(F("..."));
  display_Update();

  uint32_t i, j, k, file_length = myFile.fileSize();
  uint16_t data;

  DDRG |= (1 << 5);
  PORTG |= (1 << 5);

  draw_progressbar(0, file_length);

  for (i = 0; i < file_length; i += *flash_size) {
    // erase chip
    dataOut_MD();
    writeCommand_Acan(i, 0x8080);
    writeCommand_Acan(i, 0x1010);

    dataIn_MD();
    while (readWord_Acan(i) != 0xffff)
      ;

    for (j = 0; j < *flash_size; j += 512) {
      myFile.read(sdBuffer, 512);

      for (k = 0; k < 512; k += 2) {
        data = *((uint16_t *)(sdBuffer + k));

        dataOut_MD();
        writeCommand_Acan(i, 0xa0a0);
        writeWord_Acan(i + j + k, data);

        dataIn_MD();
        while (readWord_Acan(i + j + k) != data)
          ;
      }

      draw_progressbar(i + j + k, file_length);

      if ((j & 0xfff) == 0)
        blinkLED();
    }
  }

  PORTG &= ~(1 << 5);
  DDRG &= ~(1 << 5);

  myFile.close();
  print_STR(done_STR, 1);

  checkRomExist_Acan();
}

static void checkRomExist_Acan() {
  // RST to 0
  PORTH &= ~(1 << 0);
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;
  NOP;

  // /RST to 1
  PORTH |= (1 << 0);

  cartSize = getRomSize_Acan();
  romSize = cartSize >> 17;
  mode = CORE_SUPRACAN;

  if (cartSize == 0)
    print_Error(F("Unable to find rom signature..."));
  else {
    print_Msg(FS(FSTRING_ROM_SIZE));
    print_Msg(romSize);
    println_Msg(F(" Mb"));
  }

  display_Update();
}

static uint32_t getRomSize_Acan() {
  uint32_t addr = 0;
  uint32_t crc32;

  do {
    // check if there is rom chip exists
    // pull-up enable
    DDRC = 0xff;
    DDRA = 0xff;
    PORTC = 0xff;
    PORTA = 0xff;
    DDRC = 0x00;
    DDRA = 0x00;
    *((uint16_t *)sdBuffer) = readWord_Acan(addr);

    // pull-up disable
    DDRC = 0xff;
    DDRA = 0xff;
    PORTC = 0x00;
    PORTA = 0x00;
    DDRC = 0x00;
    DDRA = 0x00;
    *((uint16_t *)(sdBuffer + 2)) = readWord_Acan(addr);

    // should be them same if chip exists
    if (sdBuffer[0] != sdBuffer[2] || sdBuffer[1] != sdBuffer[3])
      break;

    crc32 = 0xffffffff;

    for (uint32_t i = 0x2000; i < 0x2080; i += 2) {
      *((uint16_t *)sdBuffer) = readWord_Acan(addr + i);
      UPDATE_CRC(crc32, sdBuffer[0]);
      UPDATE_CRC(crc32, sdBuffer[1]);
    }

    crc32 = ~crc32;

    if (crc32 == 0xa2bc9d7a) {
      if (addr > 0)
        break;
    } else {
      if (addr == 0)
        break;
    }

    addr += 0x80000;

  } while (addr < 0x800000);

  return addr;
}

static void resetCart_Acan() {
  // set /CS(PH3), R/W(PH5), /AS(PH6) high
  // /RST(PH0) and C27(PH4) low
  PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
  PORTH &= ~((1 << 0) | (1 << 4));

  if (i2c_found) {
    clockgen.output_enable(SI5351_CLK1, 0);  // CPU clock
    clockgen.output_enable(SI5351_CLK2, 0);  // CIC clock
    clockgen.output_enable(SI5351_CLK0, 0);  // master clock
  }
}

static void writeWord_Acan(uint32_t addr, uint16_t data) {
  uint8_t *ptr = (uint8_t *)&addr;

  PORTF = *ptr++;
  PORTK = *ptr++;
  PORTL = *ptr;

  if (*ptr < 0xe0) {
    // ROM area
    // /CS(PH3), C27(PH4), R/W(PH5), /AS(PH6) to L
    PORTH &= ~((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
  } else if (*ptr == 0xec) {
    // save area
    // /CS(PH3) to H, C27(PH4), R/W(PH5), /AS(PH6) to L
    PORTH |= (1 << 3);
    PORTH &= ~((1 << 4) | (1 << 5) | (1 << 6));
  } else if (addr == 0x00eb0d03 || addr == 0x00eb0d01) {
    // UM6650 area
    // /CS(PH3), C27(PH4) to H, R/W(PH5), /AS(PH6) to L
    PORTH |= ((1 << 3) | (1 << 4));
    PORTH &= ~((1 << 5) | (1 << 6));
  }

  ptr = (uint8_t *)&data;
  PORTC = *ptr++;
  PORTA = *ptr;

  NOP;
  NOP;
  NOP;

  PORTH &= ~(1 << 4);
  PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));
}

static uint16_t readWord_Acan(uint32_t addr) {
  uint8_t *ptr = (uint8_t *)&addr;
  uint16_t data;

  PORTF = *ptr++;
  PORTK = *ptr++;
  PORTL = *ptr;

  if (*ptr < 0xe0) {
    // ROM area
    // /CS(PH3), C27(PH4), /AS(PH6) to L
    PORTH &= ~((1 << 3) | (1 << 4) | (1 << 6));
  } else if (*ptr == 0xec) {
    // save area
    // /CS(PH3) to H, C27(PH4), /AS(PH6) to L
    PORTH |= (1 << 3);
    PORTH &= ~((1 << 4) | (1 << 6));
  } else if (addr == 0x00eb0d03 || addr == 0x00eb0d01) {
    // UM6650 area
    // /CS(PH3), C27(PH4) to H, /AS(PH6) to L
    PORTH |= ((1 << 3) | (1 << 4));
    PORTH &= ~(1 << 6);
  }

  ptr = (uint8_t *)&data;
  NOP;
  NOP;
  NOP;

  *ptr++ = PINC;
  *ptr = PINA;

  PORTH &= ~(1 << 4);
  PORTH |= ((1 << 3) | (1 << 5) | (1 << 6));

  return data;
}

static uint32_t getFlashChipSize_Acan(uint16_t chip_id) {
  // 0x0458 (8M), 0x01ab (4M), 0x01d8 (16M)
  switch (chip_id) {
    case 0x01ab: return 524288;
    case 0x0458: return 1048576;
    case 0x01d8: return 2097152;
  }

  return 0;
}

#endif
