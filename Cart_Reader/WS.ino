//*********************************************************
// BANDAI WONDERSWAN & BENESSE POCKET CHALLENGE V2 MODULE
//*********************************************************
#ifdef ENABLE_WS
// Cartridge pinout
// 48P 1.25mm pitch connector
// C1, C48     : GND
// C24, C25    : VDD (+3.3v)
// C16-C23     : D7-D0
// C34-C39     : D8-D13
// C14-C15     : D15-D14
// C26-C29     : A(-1)-A2
// C10-C13     : A6-A3
// C30-C33     : A18-A15
// C2,C3,C4,C5 : A14,A9,A10,A8
// C6,C7,C8,C9 : A7,A12,A13,A11
// C40         : /RST
// C41         : /IO? (only use when unlocking MMC)
// C42         : /MMC (access port on cartridge with both /CART and /MMC = L)
// C43         : /OE
// C44         : /WE
// C45         : /CART? (L when accessing cartridge (ROM/SRAM/PORT))
// C46         : INT (for RTC alarm interrupt)
// C47         : CLK (384KHz on WS)

#ifdef OPTION_WS_ADAPTER_V2
#define WS_CLK_BIT 5  // USE PE5 as CLK
#else
#define WS_CLK_BIT 3  // USE PE3 as CLK
#endif

/******************************************
  Menu
*****************************************/
static const char wsMenuItem5[] PROGMEM = "Write WitchOS";
static const char *const menuOptionsWS[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, FSTRING_RESET, wsMenuItem5 };
static const uint8_t wwLaunchCode[] PROGMEM = { 0xea, 0x00, 0x00, 0x00, 0xe0, 0x00, 0xff, 0xff };
static uint8_t wsGameOrientation = 0;
static uint8_t wsGameHasRTC = 0;
static uint16_t wsGameChecksum = 0;
static uint8_t wsEepromShiftReg[2];
static boolean wsWitch = false;

void setup_WS() {
  // Request 3.3V
  setVoltage(VOLTS_SET_3V3);

  // A-1 - A6
  DDRF = 0xff;
  // A7 - A14
  DDRK = 0xff;
  // A15 - A22
  DDRL = 0xff;

  // D0 - D15
  DDRC = 0x00;
  DDRA = 0x00;

  // controls
  DDRH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));
  PORTH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));

  // CLK outputs LOW
  DDRE |= (1 << WS_CLK_BIT);
  PORTE &= ~(1 << WS_CLK_BIT);

  // IO? as input with internal pull-up enabled
  DDRE &= ~(1 << 4);
  PORTE |= (1 << 4);

  // INT as input with internal pull-up enabled
  DDRG &= ~(1 << 5);
  PORTG |= (1 << 5);

  // unlock MMC
  //  if (!unlockMMC2003_WS())
  //    print_FatalError(F("Can't initial MMC"));

  //  if (getCartInfo_WS() != 0xea)
  //    print_FatalError(F("Rom header read error"));

  println_Msg(F("Initializing..."));
  display_Update();

  do {
    unlockMMC2003_WS();
  } while (!headerCheck());

  getCartInfo_WS();

  showCartInfo_WS();
}

static boolean headerCheck() {
  dataIn_WS();

  for (uint32_t i = 0; i < 16; i += 2)
    *((uint16_t *)(sdBuffer + i)) = readWord_WS(0xffff0 + i);

  uint8_t startByte = sdBuffer[0];
  if (startByte == 0xEA) {  // Start should be 0xEA
    uint8_t zeroByte = sdBuffer[5];
    if (zeroByte == 0) {  // Zero Byte
      uint8_t systemByte = sdBuffer[7];
      if (systemByte < 2) {  // System < 2
        uint8_t revisionByte = sdBuffer[9];
        if ((revisionByte < 7) || (revisionByte == 0x80)) {  // Known Revisions: 0 to 6 and 0x80
          uint8_t sizeByte = sdBuffer[10];
          if (sizeByte < 10)  // Rom Size < 10
            return true;
        }
      }
    }
  }
  return false;
}

void wsMenu() {
  uint8_t mainMenu = (wsWitch ? 5 : 4);

  convertPgm(menuOptionsWS, mainMenu);
  mainMenu = question_box(F("WS Menu"), menuOptions, mainMenu, 0);

  switch (mainMenu) {
    case 0:
      {
        // Read Rom
        sd.chdir("/");
        compareChecksum_WS(readROM_WS(filePath, FILEPATH_LENGTH));
        sd.chdir("/");
        break;
      }
    case 1:
      {
        // Read Save
        sd.chdir("/");
        switch (saveType) {
          case 0: println_Msg(F("No save for this game")); break;
          case 1: readSRAM_WS(); break;
          case 2: readEEPROM_WS(); break;
          default: println_Msg(F("Unknown save type")); break;
        }

        break;
      }
    case 2:
      {
        // Write Save
        sd.chdir("/");
        switch (saveType) {
          case 0: println_Msg(F("No save for this game")); break;
          case 1:
            {
              writeSRAM_WS();
              verifySRAM_WS();
              break;
            }
          case 2:
            {
              writeEEPROM_WS();
              verifyEEPROM_WS();
              break;
            }
          default: println_Msg(F("Unknown save type")); break;
        }

        break;
      }
    case 4:
      {
        writeWitchOS_WS();
        break;
      }
    default:
      {
        // reset
        asm volatile("  jmp 0");
        break;
      }
  }

  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);

  display_Update();
  wait();
}

static uint8_t getCartInfo_WS() {
  dataIn_WS();

  //  for (uint32_t i = 0; i < 16; i += 2)
  //    *((uint16_t*)(sdBuffer + i)) = readWord_WS(0xffff0 + i);

  wsGameChecksum = *(uint16_t *)(sdBuffer + 14);
  wsWitch = false;

  // some game has wrong info in header
  // patch here
  switch (wsGameChecksum) {
    // games with wrong save type/size
    // 256kbits sram
    case 0xe600:  // BAN007
    case 0x8eed:  // BANC16
    case 0xee90:  // WIZC01
      {
        sdBuffer[11] = 0x02;
        break;
      }
    // games missing 'COLOR' flag
    case 0x26db:  // SQRC01
    case 0xbfdf:  // SUMC07
    case 0x50ca:  // BANC09
    case 0x9238:  // BANC0E
    case 0x04F1:  // BANC1A
      {
        sdBuffer[7] |= 0x01;
        break;
      }
    case 0x7f73:  // BAN030
      {
        // missing developerId and cartId
        sdBuffer[6] = 0x01;
        sdBuffer[8] = 0x30;
        break;
      }
    case 0xeafd:  //BANC33
      {
        // enable GPIO and set to LOW
        dataOut_WS();
        writeByte_WSPort(0xcc, 0x03);
        writeByte_WSPort(0xcd, 0x00);
        break;
      }
    case 0x0000:
      {
        // developerId/cartId/checksum are all filled with 0x00 in witch based games
        dataIn_WS();
        if (readWord_WS(0xf0000) == 0x4c45 && readWord_WS(0xf0002) == 0x5349 && readWord_WS(0xf0004) == 0x0041) {
          // check witch BIOS
          if (readWord_WS(0xfff5e) == 0x006c && readWord_WS(0xfff60) == 0x5b1b) {
            // check flashchip
            // should be a MBM29DL400TC
            dataOut_WS();
            writeWord_WS(0x80aaa, 0xaaaa);
            writeWord_WS(0x80555, 0x5555);
            writeWord_WS(0x80aaa, 0x9090);

            dataIn_WS();
            if (readWord_WS(0x80000) == 0x0004 && readWord_WS(0x80002) == 0x220c)
              wsWitch = true;

            dataOut_WS();
            writeWord_WS(0x80000, 0xf0f0);
            dataIn_WS();

            // 7AC003
            sdBuffer[6] = 0x7a;
            sdBuffer[8] = 0x03;
          }
          // check service menu
          else if (readWord_WS(0xfff22) == 0x006c && readWord_WS(0xfff24) == 0x5b1b) {
            if (readWord_WS(0x93246) == 0x4a2f && readWord_WS(0x93248) == 0x5353 && readWord_WS(0x9324a) == 0x2e32) {
              // jss2
              sdBuffer[6] = 0xff;   // WWGP
              sdBuffer[8] = 0x1a;   // 2001A
              sdBuffer[7] = 0x01;   // color only
              sdBuffer[10] = 0x04;  // size based on ROM chip capacity

              if (readWord_WS(0x93e9c) == 0x4648 && readWord_WS(0x93e9e) == 0x0050) {
                // WWGP2001A3 -> HFP Version
                sdBuffer[9] = 0x03;
                wsGameChecksum = 0x4870;
              } else {
                // TODO check other jss2 version
              }
            } else if (readWord_WS(0xe4260) == 0x6b64 && readWord_WS(0xe4262) == 0x696e) {
              // dknight
              sdBuffer[6] = 0xff;  // WWGP
              sdBuffer[8] = 0x2b;  // 2002B
              sdBuffer[7] = 0x01;  // color only
              sdBuffer[9] = 0x00;
              sdBuffer[10] = 0x04;  // size based on ROM chip (MR27V1602) capacity
              wsGameChecksum = 0x8b1c;
            }
          }
        } else if (sdBuffer[6] == 0x2a && sdBuffer[8] == 0x01 && sdBuffer[9] == 0x01) {
          // Mobile WonderGate v1.1, checksum is filled with 0x0000
          wsGameChecksum = 0x1da0;
        }

        break;
      }
  }

  romType = (sdBuffer[7] & 0x01);  // wsc only = 1
  romVersion = sdBuffer[9];
  romSize = sdBuffer[10];
  sramSize = sdBuffer[11];
  wsGameOrientation = (sdBuffer[12] & 0x01);
  wsGameHasRTC = (sdBuffer[13] & 0x01);

  getDeveloperName_WS(sdBuffer[6], vendorID, 5);
  snprintf(cartID, 5, "%c%02X", (romType ? 'C' : '0'), sdBuffer[8]);
  snprintf(checksumStr, 5, "%04X", wsGameChecksum);
  snprintf(romName, 17, "%s%s", vendorID, cartID);

  switch (romSize) {
    case 0x01: cartSize = 131072 * 2; break;
    case 0x02: cartSize = 131072 * 4; break;
    case 0x03: cartSize = 131072 * 8; break;
    case 0x04: cartSize = 131072 * 16; break;
    // case 0x05: cartSize = 131072 * 24; break;
    case 0x06: cartSize = 131072 * 32; break;
    // case 0x07: cartSize = 131072 * 48; break;
    case 0x08: cartSize = 131072 * 64; break;
    case 0x09: cartSize = 131072 * 128; break;
    default: cartSize = 0; break;
  }

  switch (sramSize) {
    case 0x00:
      saveType = 0;
      sramSize = 0;
      break;
    case 0x01:
      saveType = 1;
      sramSize = 64;
      break;
    case 0x02:
      saveType = 1;
      sramSize = 256;
      break;
    case 0x03:
      saveType = 1;
      sramSize = 1024;
      break;
    case 0x04:
      saveType = 1;
      sramSize = 2048;
      break;
    case 0x05:
      saveType = 1;
      sramSize = 4096;
      break;
    case 0x10:
      saveType = 2;
      sramSize = 1;
      break;
    case 0x20:
      saveType = 2;
      sramSize = 16;
      break;
    case 0x50:
      saveType = 2;
      sramSize = 8;
      break;
    default:
      saveType = 0xff;
      break;
  }

  if (saveType == 2)
    unprotectEEPROM_WS();

  // should be 0xea (JMPF instruction)
  return sdBuffer[0];
}

static void showCartInfo_WS() {
  display_Clear();

  println_Msg(F("Cart Info"));

  print_Msg(F("Game: "));
  println_Msg(romName);

  print_Msg(FS(FSTRING_ROM_SIZE));
  if (cartSize == 0x00)
    println_Msg(romSize, HEX);
  else {
    print_Msg((cartSize >> 17));
    println_Msg(F(" Mb"));
  }

  print_Msg(F("Save: "));
  switch (saveType) {
    case 0: println_Msg(F("None")); break;
    case 1:
      print_Msg(F("Sram "));
      print_Msg(sramSize);
      println_Msg(F(" Kb"));
      break;
    case 2:
      print_Msg(F("Eeprom "));
      print_Msg(sramSize);
      println_Msg(F(" Kb"));
      break;
    default: println_Msg(sramSize, HEX); break;
  }

  print_Msg(F("Version: 1."));
  println_Msg(romVersion, HEX);

  print_Msg(FS(FSTRING_CHECKSUM));
  println_Msg(checksumStr);

  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
}

static void getDeveloperName_WS(uint8_t id, char *buf, size_t length) {
  if (buf == NULL)
    return;

  char *devName = NULL;

  switch (id) {
    case 0x01: devName = PSTR("BAN"); break;
    case 0x02: devName = PSTR("TAT"); break;
    case 0x03: devName = PSTR("TMY"); break;
    case 0x04: devName = PSTR("KEX"); break;
    case 0x05: devName = PSTR("DTE"); break;
    case 0x06: devName = PSTR("AAE"); break;
    case 0x07: devName = PSTR("MDE"); break;
    case 0x08: devName = PSTR("NHB"); break;
    case 0x0a: devName = PSTR("CCJ"); break;
    case 0x0b: devName = PSTR("SUM"); break;
    case 0x0c: devName = PSTR("SUN"); break;
    case 0x0d: devName = PSTR("PAW"); break;
    case 0x0e: devName = PSTR("BPR"); break;
    case 0x10: devName = PSTR("JLC"); break;
    case 0x11: devName = PSTR("MGA"); break;
    case 0x12: devName = PSTR("KNM"); break;
    case 0x16: devName = PSTR("KBS"); break;
    case 0x17: devName = PSTR("BTM"); break;
    case 0x18: devName = PSTR("KGT"); break;
    case 0x19: devName = PSTR("SRV"); break;
    case 0x1a: devName = PSTR("CFT"); break;
    case 0x1b: devName = PSTR("MGH"); break;
    case 0x1d: devName = PSTR("BEC"); break;
    case 0x1e: devName = PSTR("NAP"); break;
    case 0x1f: devName = PSTR("BVL"); break;
    case 0x20: devName = PSTR("ATN"); break;
    case 0x21: devName = PSTR("KDX"); break;  // KDK for Memories of Festa?
    case 0x22: devName = PSTR("HAL"); break;
    case 0x23: devName = PSTR("YKE"); break;
    case 0x24: devName = PSTR("OMM"); break;
    case 0x25: devName = PSTR("LAY"); break;
    case 0x26: devName = PSTR("KDK"); break;
    case 0x27: devName = PSTR("SHL"); break;
    case 0x28: devName = PSTR("SQR"); break;
    case 0x2a: devName = PSTR("SCC"); break;
    case 0x2b: devName = PSTR("TMC"); break;
    case 0x2d: devName = PSTR("NMC"); break;
    case 0x2e: devName = PSTR("SES"); break;
    case 0x2f: devName = PSTR("HTR"); break;
    case 0x31: devName = PSTR("VGD"); break;
    case 0x33: devName = PSTR("WIZ"); break;
    case 0x36: devName = PSTR("CPC"); break;

    // custom developerId
    case 0x7a: devName = PSTR("7AC"); break;  // witch
    case 0xff:
      devName = PSTR("WWGP");
      break;  // WWGP series (jss2, dknight)

    // if not found, use id
    default: snprintf(buf, length, "%02X", id); return;
  }

  strlcpy_P(buf, devName, length);
}

static uint16_t readROM_WS(char *outPathBuf, size_t bufferSize) {
  // generate fullname of rom file
  snprintf(fileName, FILENAME_LENGTH, "%s.ws%c", romName, ((romType & 1) ? 'c' : '\0'));

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  snprintf(folder, sizeof(folder), "WS/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // filling output file path to buffer
  if (outPathBuf != NULL && bufferSize > 0)
    snprintf(outPathBuf, bufferSize, "%s/%s", folder, fileName);

  printAndIncrementFolder(true);

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_FatalError(create_file_STR);

  // get correct starting rom bank
  uint16_t bank = (256 - (cartSize >> 16));
  uint32_t progress = 0;
  uint16_t checksum = 0;

  draw_progressbar(0, cartSize);

  // start reading rom
  for (; bank <= 0xff; bank++) {
    // switch bank on segment 0x2
    dataOut_WS();
    writeByte_WSPort(0xc2, bank);

    // blink LED on cartridge (only for BANC33)
    if (wsGameChecksum == 0xeafd)
      writeByte_WSPort(0xcd, (bank & 0x03));

    dataIn_WS();
    for (uint32_t addr = 0; addr < 0x10000; addr += 512) {
      // blink LED
      if ((addr & ((1 << 14) - 1)) == 0)
        blinkLED();

      for (uint32_t w = 0; w < 512; w += 2) {
        *((uint16_t *)(sdBuffer + w)) = readWord_WS(0x20000 + addr + w);

        // only calculate last two banks of checksum (os and bios region)
        if (wsWitch && bank < 0xfe)
          continue;

        // skip last two bytes of rom (checksum value)
        if (w == 510 && progress == cartSize - 512)
          continue;

        checksum += sdBuffer[w];
        checksum += sdBuffer[w + 1];
      }

      myFile.write(sdBuffer, 512);
      progress += 512;
    }

    draw_progressbar(progress, cartSize);
  }

  myFile.close();

  // turn off LEDs (only for BANC33)
  if (wsGameChecksum == 0xeafd) {
    dataOut_WS();
    writeByte_WSPort(0xcd, 0x00);
  }

  return checksum;
}

static void readSRAM_WS() {
  // generate fullname of rom file
  createFolderAndOpenFile("WS", "SAVE", romName, "save");

  uint32_t bank_size = (sramSize << 7);
  uint16_t end_bank = (bank_size >> 16);  // 64KB per bank

  if (bank_size > 0x10000)
    bank_size = 0x10000;

  uint16_t bank = 0;

  do {
    dataOut_WS();
    writeByte_WSPort(0xc1, bank);

    dataIn_WS();
    for (uint32_t addr = 0; addr < bank_size; addr += 512) {
      // blink LED
      if ((addr & ((1 << 14) - 1)) == 0)
        blinkLED();

      // SRAM data on D0-D7, with A-1 to select high/low byte
      for (uint32_t w = 0; w < 512; w++)
        sdBuffer[w] = readByte_WS(0x10000 + addr + w);

      myFile.write(sdBuffer, 512);
    }
  } while (++bank < end_bank);

  myFile.close();

  print_STR(done_STR, 1);
  display_Update();
}

static void verifySRAM_WS() {
  print_Msg(F("Verifying... "));
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    uint32_t bank_size = (sramSize << 7);
    uint16_t end_bank = (bank_size >> 16);  // 64KB per bank
    uint16_t bank = 0;
    uint32_t write_errors = 0;

    if (bank_size > 0x10000)
      bank_size = 0x10000;

    do {
      dataOut_WS();
      writeByte_WSPort(0xc1, bank);

      dataIn_WS();
      for (uint32_t addr = 0; addr < bank_size && myFile.available(); addr += 512) {
        myFile.read(sdBuffer, 512);

        // SRAM data on D0-D7, with A-1 to select high/low byte
        for (uint32_t w = 0; w < 512; w++) {
          if (readByte_WS(0x10000 + addr + w) != sdBuffer[w])
            write_errors++;
        }
      }
    } while (++bank < end_bank);

    myFile.close();

    if (write_errors == 0) {
      println_Msg(F("passed"));
    } else {
      println_Msg(F("failed"));
      print_STR(error_STR, 0);
      print_Msg(write_errors);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

static void writeSRAM_WS() {
  filePath[0] = 0;
  sd.chdir("/");
  fileBrowser(F("Select sav file"));
  snprintf(filePath, FILEPATH_LENGTH, "%s/%s", filePath, fileName);

  display_Clear();
  print_Msg(F("Writing "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    uint32_t bank_size = (sramSize << 7);
    uint16_t end_bank = (bank_size >> 16);  // 64KB per bank

    if (bank_size > 0x10000)
      bank_size = 0x10000;

    uint16_t bank = 0;
    dataOut_WS();
    do {
      writeByte_WSPort(0xc1, bank);

      for (uint32_t addr = 0; addr < bank_size && myFile.available(); addr += 512) {
        // blink LED
        if ((addr & ((1 << 14) - 1)) == 0)
          blinkLED();

        myFile.read(sdBuffer, 512);

        // SRAM data on D0-D7, with A-1 to select high/low byte
        for (uint32_t w = 0; w < 512; w++)
          writeByte_WS(0x10000 + addr + w, sdBuffer[w]);
      }
    } while (++bank < end_bank);

    myFile.close();

    println_Msg(F("Writing finished"));
    display_Update();
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

static void readEEPROM_WS() {
  // generate fullname of eep file
  createFolderAndOpenFile("WS", "SAVE", romName, "eep");

  uint32_t eepromSize = (sramSize << 7);
  uint32_t bufSize = (eepromSize < 512 ? eepromSize : 512);

  for (uint32_t i = 0; i < eepromSize; i += bufSize) {
    for (uint32_t j = 0; j < bufSize; j += 2) {
      // blink LED
      if ((j & 0x1f) == 0x00)
        blinkLED();

      generateEepromInstruction_WS(wsEepromShiftReg, 0x2, ((i + j) >> 1));

      dataOut_WS();
      writeByte_WSPort(0xc6, wsEepromShiftReg[0]);
      writeByte_WSPort(0xc7, wsEepromShiftReg[1]);
      writeByte_WSPort(0xc8, 0x10);

      // MMC will shift out from port 0xc7 to 0xc6
      // and shift in 16bits into port 0xc5 to 0xc4
      pulseCLK_WS(1 + 32 + 3);

      dataIn_WS();
      sdBuffer[j] = readByte_WSPort(0xc4);
      sdBuffer[j + 1] = readByte_WSPort(0xc5);
    }

    myFile.write(sdBuffer, bufSize);
  }

  myFile.close();

  print_STR(done_STR, 1);
}

static void verifyEEPROM_WS() {
  print_Msg(F("Verifying... "));
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    uint32_t write_errors = 0;
    uint32_t eepromSize = (sramSize << 7);
    uint32_t bufSize = (eepromSize < 512 ? eepromSize : 512);

    for (uint32_t i = 0; i < eepromSize; i += bufSize) {
      myFile.read(sdBuffer, bufSize);

      for (uint32_t j = 0; j < bufSize; j += 2) {
        // blink LED
        if ((j & 0x1f) == 0x00)
          blinkLED();

        generateEepromInstruction_WS(wsEepromShiftReg, 0x2, ((i + j) >> 1));

        dataOut_WS();
        writeByte_WSPort(0xc6, wsEepromShiftReg[0]);
        writeByte_WSPort(0xc7, wsEepromShiftReg[1]);
        writeByte_WSPort(0xc8, 0x10);

        // MMC will shift out from port 0xc7 to 0xc6
        // and shift in 16bits into port 0xc5 to 0xc4
        pulseCLK_WS(1 + 32 + 3);

        dataIn_WS();
        if (readByte_WSPort(0xc4) != sdBuffer[j])
          write_errors++;

        if (readByte_WSPort(0xc5) != sdBuffer[j + 1])
          write_errors++;
      }
    }

    myFile.close();

    if (write_errors == 0) {
      println_Msg(F("passed"));
    } else {
      println_Msg(F("failed"));
      print_STR(error_STR, 0);
      print_Msg(write_errors);
      print_STR(_bytes_STR, 1);
      print_Error(did_not_verify_STR);
    }
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

static void writeEEPROM_WS() {
  filePath[0] = 0;
  sd.chdir("/");
  fileBrowser(F("Select eep file"));
  snprintf(filePath, FILEPATH_LENGTH, "%s/%s", filePath, fileName);

  display_Clear();
  print_Msg(F("Writing "));
  print_Msg(filePath);
  println_Msg(F("..."));
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    uint32_t eepromSize = (sramSize << 7);
    uint32_t bufSize = (eepromSize < 512 ? eepromSize : 512);

    for (uint32_t i = 0; i < eepromSize; i += bufSize) {
      myFile.read(sdBuffer, bufSize);

      for (uint32_t j = 0; j < bufSize; j += 2) {
        // blink LED
        if ((j & 0x1f) == 0x00)
          blinkLED();

        generateEepromInstruction_WS(wsEepromShiftReg, 0x1, ((i + j) >> 1));

        dataOut_WS();
        writeByte_WSPort(0xc6, wsEepromShiftReg[0]);
        writeByte_WSPort(0xc7, wsEepromShiftReg[1]);
        writeByte_WSPort(0xc4, sdBuffer[j]);
        writeByte_WSPort(0xc5, sdBuffer[j + 1]);
        writeByte_WSPort(0xc8, 0x20);

        // MMC will shift out from port 0xc7 to 0xc4
        pulseCLK_WS(1 + 32 + 3);

        dataIn_WS();
        do {
          pulseCLK_WS(128);
        } while ((readByte_WSPort(0xc8) & 0x02) == 0x00);
      }
    }

    myFile.close();

    print_STR(done_STR, 1);
  } else {
    print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
  }
}

static void writeWitchOS_WS() {
  // make sure that OS sectors not protected
  dataOut_WS();
  writeWord_WS(0x80aaa, 0xaaaa);
  writeWord_WS(0x80555, 0x5555);
  writeWord_WS(0xe0aaa, 0x9090);

  dataIn_WS();
  if (readWord_WS(0xe0004) || readWord_WS(0xe4004) || readWord_WS(0xec004) || readWord_WS(0xee004)) {
    display_Clear();
    print_Error(F("OS sectors are protected!"));
  } else {
    filePath[0] = 0;
    sd.chdir("/");
    fileBrowser(F("Select fbin file"));
    snprintf(filePath, FILEPATH_LENGTH, "%s/%s", filePath, fileName);

    display_Clear();

    if (myFile.open(filePath, O_READ)) {
      println_Msg(F("Erasing OS..."));
      display_Update();
      eraseWitchFlashSector_WS(0xe0000);
      eraseWitchFlashSector_WS(0xe4000);
      eraseWitchFlashSector_WS(0xec000);
      eraseWitchFlashSector_WS(0xee000);

      print_Msg(F("Flashing OS "));
      print_Msg(filePath);
      println_Msg(F("..."));
      display_Update();

      uint32_t fbin_length = myFile.fileSize();
      uint32_t i, bytes_read;
      uint16_t pd;
      uint8_t key;

      // OS size seems limit to 64KBytes
      // last 16 bytes contains jmpf code and block count (written by BIOS)
      if (fbin_length > 65520)
        fbin_length = 65520;

      // enter fast program mode
      dataOut_WS();
      writeWord_WS(0x80aaa, 0xaaaa);
      writeWord_WS(0x80555, 0x5555);
      writeWord_WS(0x80aaa, 0x2020);

      // 128bytes per block
      for (i = 0; i < fbin_length; i += 128) {
        // blink LED
        if ((i & 0x3ff) == 0)
          blinkLED();

        // reset key
        key = 0xff;
        bytes_read = myFile.read(sdBuffer, 128);

        for (uint32_t j = 0; j < bytes_read; j += 2) {
          // for each decoded[n] = encoded[n] ^ key
          // where key = encoded[n - 1]
          // key = 0xff when n = 0, 0 <= n < 128
          pd = ((sdBuffer[j] ^ key) | ((sdBuffer[j + 1] ^ sdBuffer[j]) << 8));
          key = sdBuffer[j + 1];

          fastProgramWitchFlash_WS(0xe0000 + i + j, pd);
        }
      }

      // write jmpf instruction and block counts at 0xe0000
      memcpy_P(sdBuffer, wwLaunchCode, 8);
      *((uint16_t *)(sdBuffer + 6)) = ((i >> 7) & 0xffff);

      for (uint32_t i = 0; i < 8; i += 2)
        fastProgramWitchFlash_WS(0xefff0 + i, *((uint16_t *)(sdBuffer + i)));

      // leave fast program mode
      dataOut_WS();
      writeWord_WS(0xe0000, 0x9090);
      writeWord_WS(0xe0000, 0xf0f0);

      myFile.close();

      print_STR(done_STR, 1);
    } else {
      print_Error(FS(FSTRING_FILE_DOESNT_EXIST));
    }
  }

  dataOut_WS();
  writeWord_WS(0x80000, 0xf0f0);
}

static void fastProgramWitchFlash_WS(uint32_t addr, uint16_t data) {
  dataOut_WS();

  writeWord_WS(addr, 0xa0a0);
  writeWord_WS(addr, data);

  dataIn_WS();
  while (readWord_WS(addr) != data)
    ;
}

static void eraseWitchFlashSector_WS(uint32_t sector_addr) {
  // blink LED
  blinkLED();

  dataOut_WS();
  writeWord_WS(0x80aaa, 0xaaaa);
  writeWord_WS(0x80555, 0x5555);
  writeWord_WS(0x80aaa, 0x8080);
  writeWord_WS(0x80aaa, 0xaaaa);
  writeWord_WS(0x80555, 0x5555);
  writeWord_WS(sector_addr, 0x3030);

  dataIn_WS();
  while ((readWord_WS(sector_addr) & 0x0080) == 0x0000)
    ;
}

static boolean compareChecksum_WS(uint16_t checksum) {
  char result[11];
  snprintf(result, 11, "%04X(%04X)", wsGameChecksum, checksum);
  print_Msg(F("Result: "));
  print_Msg(result);

  if (checksum == wsGameChecksum) {
    println_Msg(F(" matches."));

    // Compare CRC32 to database and rename ROM if found
    // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
    compareCRC("ws.txt", 0, 1, 0);

    return 1;
  } else {
    println_Msg(FS(FSTRING_EMPTY));
    print_Error(F("Checksum Error"));
    return 0;
  }
}

static void writeByte_WSPort(uint8_t port, uint8_t data) {
  PORTF = (port & 0x0f);
  PORTL = (port >> 4);

  // switch CART(PH3), MMC(PH4) to LOW
  PORTH &= ~((1 << 3) | (1 << 4));

  // set data
  PORTC = data;

  // switch WE(PH5) to LOW
  PORTH &= ~(1 << 5);
  NOP;

  // switch WE(PH5) to HIGH
  PORTH |= (1 << 5);
  NOP;
  NOP;

  // switch CART(PH3), MMC(PH4) to HIGH
  PORTH |= ((1 << 3) | (1 << 4));
}

static uint8_t readByte_WSPort(uint8_t port) {
  PORTF = (port & 0x0f);
  PORTL = (port >> 4);

  // switch CART(PH3), MMC(PH4) to LOW
  PORTH &= ~((1 << 3) | (1 << 4));

  // switch OE(PH6) to LOW
  PORTH &= ~(1 << 6);
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // switch OE(PH6) to HIGH
  PORTH |= (1 << 6);

  // switch CART(PH3), MMC(PH4) to HIGH
  PORTH |= ((1 << 3) | (1 << 4));

  return ret;
}

static void writeWord_WS(uint32_t addr, uint16_t data) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x0f;

  PORTC = data & 0xff;
  PORTA = (data >> 8);

  // switch CART(PH3) and WE(PH5) to LOW
  PORTH &= ~((1 << 3) | (1 << 5));
  NOP;

  // switch CART(PH3) and WE(PH5) to HIGH
  PORTH |= (1 << 3) | (1 << 5);
  NOP;
  NOP;
}

static uint16_t readWord_WS(uint32_t addr) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x0f;

  // switch CART(PH3) and OE(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
  NOP;
  NOP;
  NOP;

  uint16_t ret = ((PINA << 8) | PINC);

  // switch CART(PH3) and OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  return ret;
}

static void writeByte_WS(uint32_t addr, uint8_t data) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x0f;

  PORTC = data;

  // switch CART(PH3) and WE(PH5) to LOW
  PORTH &= ~((1 << 3) | (1 << 5));
  NOP;

  // switch CART(PH3) and WE(PH5) to HIGH
  PORTH |= (1 << 3) | (1 << 5);
  NOP;
  NOP;
}

static uint8_t readByte_WS(uint32_t addr) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x0f;

  // switch CART(PH3) and OE(PH6) to LOW
  PORTH &= ~((1 << 3) | (1 << 6));
  NOP;
  NOP;
  NOP;

  uint8_t ret = PINC;

  // switch CART(PH3) and OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);

  return ret;
}

static void unprotectEEPROM_WS() {
  generateEepromInstruction_WS(wsEepromShiftReg, 0x0, 0x3);

  dataOut_WS();
  writeByte_WSPort(0xc6, wsEepromShiftReg[0]);
  writeByte_WSPort(0xc7, wsEepromShiftReg[1]);
  writeByte_WSPort(0xc8, 0x40);

  // MMC will shift out port 0xc7 to 0xc6 to EEPROM
  pulseCLK_WS(1 + 16 + 3);
}

// generate data for port 0xc6 to 0xc7
// number of CLK pulses needed for each instruction is 1 + (16 or 32) + 3
static void generateEepromInstruction_WS(uint8_t *instruction, uint8_t opcode, uint16_t addr) {
  uint8_t addr_bits = (sramSize > 1 ? 10 : 6);
  uint16_t *ptr = (uint16_t *)instruction;
  *ptr = 0x0001;  // initial with a start bit

  if (opcode == 0) {
    // 2bits opcode = 0x00
    *ptr <<= 2;
    // 2bits ext cmd (from addr)
    *ptr <<= 2;
    *ptr |= (addr & 0x0003);
    *ptr <<= (addr_bits - 2);
  } else {
    // 2bits opcode
    *ptr <<= 2;
    *ptr |= (opcode & 0x03);
    // address bits
    *ptr <<= addr_bits;
    *ptr |= (addr & ((1 << addr_bits) - 1));
  }
}

// 2003 MMC need to be unlock,
// or it will reject all reading and bank switching
// All signals' timing are analyzed by using LogicAnalyzer
static boolean unlockMMC2003_WS() {
  // initialize all control pin state
  // RST(PH0) and CLK(PE3or5) to LOW
  // CART(PH3) MMC(PH4) WE(PH5) OE(PH6) to HIGH
  PORTH &= ~(1 << 0);
  PORTE &= ~(1 << WS_CLK_BIT);
  PORTH |= ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));

  // switch RST(PH0) to HIGH
  PORTH |= (1 << 0);

  PORTF = 0x0a;
  PORTL = 0x05;
  pulseCLK_WS(3);

  PORTF = 0x05;
  PORTL = 0x0a;
  pulseCLK_WS(4);

  // MMC is outputing something on IO? pin synchronized with CLK
  // so still need to pulse CLK until MMC is ok to work
  pulseCLK_WS(18);

  // unlock procedure finished
  // see if we can set bank number to MMC
  dataOut_WS();
  writeByte_WSPort(0xc2, 0xaa);
  writeByte_WSPort(0xc3, 0x55);

  dataIn_WS();
  if (readByte_WSPort(0xc2) == 0xaa && readByte_WSPort(0xc3) == 0x55) {
    // now set initial bank number to MMC
    dataOut_WS();
    writeByte_WSPort(0xc0, 0x2f);
    writeByte_WSPort(0xc1, 0x3f);
    writeByte_WSPort(0xc2, 0xff);
    writeByte_WSPort(0xc3, 0xff);
    return true;
  }

  return false;
}

// doing a L->H on CLK pin
static void pulseCLK_WS(uint8_t count) {
  register uint8_t tic;

  // about 384KHz, 50% duty cycle
  asm volatile("L0_%=:\n\t"
               "cpi %[count], 0\n\t"
               "breq L3_%=\n\t"
               "dec %[count]\n\t"
               "cbi %[porte], %[ws_clk_bit]\n\t"
               "ldi %[tic], 6\n\t"
               "L1_%=:\n\t"
               "dec %[tic]\n\t"
               "brne L1_%=\n\t"
               "sbi %[porte], %[ws_clk_bit]\n\t"
               "ldi %[tic], 5\n\t"
               "L2_%=:\n\t"
               "dec %[tic]\n\t"
               "brne L2_%=\n\t"
               "rjmp L0_%=\n\t"
               "L3_%=:\n\t"
               : [tic] "=a"(tic)
               : [count] "a"(count), [porte] "I"(_SFR_IO_ADDR(PORTE)), [ws_clk_bit] "I"(WS_CLK_BIT));
}

static void dataIn_WS() {
  DDRC = 0x00;
  DDRA = 0x00;

  // some game's ROM chip needs internal-pullup be disabled to work properly
  // ex: Mobile Suit Gundam Vol.2 - JABURO (MX23L6410MC-12 Mask ROM)
  PORTC = 0x00;
  PORTA = 0x00;
}

static void dataOut_WS() {
  DDRC = 0xff;
  DDRA = 0xff;
}

#endif
//******************************************
// End of File
//******************************************
