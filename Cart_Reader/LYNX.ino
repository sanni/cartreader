//******************************************
// ATARI LYNX MODULE
//******************************************
//
// For use with SNES-Lynx adapter
// +----+
// |  1 |- GND
// |  2 |- D3
// |  3 |- D2
// |  4 |- D4
// |  5 |- D1
// |  6 |- D5
// |  7 |- D0
// |  8 |- D6
// |  9 |- D7
// | 10 |- /OE
// | 11 |- A1
// | 12 |- A2
// | 13 |- A3
// | 14 |- A6
// | 15 |- A4
// | 16 |- A5
// | 17 |- A0
// | 18 |- A7
// | 19 |- A16
// | 20 |- A17
// | 21 |- A18
// | 22 |- A19
// | 23 |- A15
// | 24 |- A14
// | 25 |- A13
// | 26 |- A12
// | 27 |- /WE
// | 28 |- A8
// | 29 |- A9
// | 30 |- A10
// | 31 |- VCC
// | 32 |- AUDIN
// | 33 |- VCC
// | 34 |- SWVCC
// +----+
//
// Version 1.1
// By @partlyhuman
// This implementation would not be possible without the invaluable
// documentation on
// https://atarilynxvault.com/
// by Igor (@theatarigamer) of K-Retro Gaming / Atari Lynx Vault
// and the reference implementation of the Lynx Cart Programmer Pi-Hat
// https://bitbucket.org/atarilynx/lynx/src/master/
// by Karri Kaksonen (whitelynx.fi) and Igor as well as countless contributions
// by the Atari Lynx community
//
#ifdef ENABLE_LYNX

#pragma region DEFS

#define LYNX_HEADER_SIZE 64
#define LYNX_WE 8
#define LYNX_OE 9
#define LYNX_AUDIN_ON 0x80000
#define LYNX_BLOCKADDR 2048UL
#define LYNX_BLOCKCOUNT 256
// Includes \0
static const char LYNX[5] = "LYNX";

// Cart information
static bool lynxUseAudin;
static uint16_t lynxBlockSize;

#pragma region LOWLEVEL

void setup_LYNX() {
  setVoltage(VOLTS_SET_5V);

  // Address pins output
  // A0-7, A8-A16 (A11 doesn't exist)
  DDRF = 0xff;
  DDRK = 0xff;
  DDRL = 0xff;

  // Data pins input
  DDRC = 0x00;

  // Control pins output
  // CE is tied low, not accessible
  pinMode(LYNX_WE, OUTPUT);
  pinMode(LYNX_OE, OUTPUT);
  digitalWrite(LYNX_WE, HIGH);
  digitalWrite(LYNX_OE, HIGH);

  strcpy(romName, LYNX);
  mode = CORE_LYNX;
}

static void dataDir_LYNX(byte direction) {
  DDRC = (direction == OUTPUT) ? 0xff : 0x00;
}

static void setAddr_LYNX(uint32_t addr) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  // AUDIN connected to L3
  PORTL = ((addr >> 16) & 0b1111);
}

static uint8_t readByte_LYNX(uint32_t addr) {
  digitalWrite(LYNX_OE, HIGH);
  setAddr_LYNX(addr);
  digitalWrite(LYNX_OE, LOW);
  delayMicroseconds(20);
  uint8_t data = PINC;
  digitalWrite(LYNX_OE, HIGH);
  return data;
}

static bool waitPressButton_LYNX(bool ret = false) {
  print_STR(press_button_STR, true);
  display_Update();
  wait();
  return ret;
}

#pragma region HIGHLEVEL

static void compareStride_LYNX(uint8_t b, int i, int stride) {
  uint8_t other = readByte_LYNX(i + stride);
  if (other == 0xff) {
    // If this is a flash cart, these in-between spaces should be formatted to all 1's
    // in which case, we DON'T report this as an unmirrored area
    return;
  }
  if (b != other) {
    // if these bytes differ, they're likely in the same block, which means the block size contains both addresses (next POT beyond the stride)
    lynxBlockSize = max(lynxBlockSize, stride << 1);
  }
}

static bool detectCart_LYNX() {
  // Could omit logging to save a few bytes
  display_Clear();
  println_Msg(F("Identifying..."));

  lynxUseAudin = false;
  lynxBlockSize = 0;

  // Somewhat arbitrary, however many bytes would be unlikely to be
  // coincidentally mirrored
  const size_t DETECT_BYTES = 128;
  for (int i = 0; i < DETECT_BYTES && lynxBlockSize < LYNX_BLOCKADDR; i++) {
    uint8_t b = readByte_LYNX(i);
    // If any differences are detected when AUDIN=1, AUDIN is used to bankswitch
    // meaning we also use the maximum block size
    // (1024kb cart / 256 blocks = 4kb block bank switched between lower/upper 2kb blocks)
    if (b != readByte_LYNX(i + LYNX_AUDIN_ON)) {
      lynxUseAudin = true;
      lynxBlockSize = 2048;
      break;
    }
    // Identify mirroring of largest stride
    // Valid cart sizes of 128kb, 256kb, 512kb / 256 blocks = block sizes of 512b, 1024b, 2048b
    compareStride_LYNX(b, i, 256);
    compareStride_LYNX(b, i, 512);
    compareStride_LYNX(b, i, 1024);
  }

  if (lynxBlockSize == 0) {
    print_STR(error_STR, false);
    display_Update();
    wait();
    resetArduino();
  }
  print_Msg(F("AUDIN="));
  print_Msg(lynxUseAudin);
  print_Msg(F(" BLOCK="));
  println_Msg(lynxBlockSize);
  display_Update();
}

static void writeHeader_LYNX() {
  char header[LYNX_HEADER_SIZE] = {};
  // Magic number
  strcpy(header, LYNX);
  // Cart name (dummy)
  strcpy(header + 10, LYNX);
  // Manufacturer (dummy)
  strcpy(header + 42, LYNX);
  // Version
  header[8] = 1;
  // Bank 0 page size
  header[4] = lynxBlockSize & 0xff;
  // Bank 1 page size
  header[5] = (lynxBlockSize >> 8) & 0xff;
  // AUDIN used
  header[59] = lynxUseAudin;
  // TODO detect EEPROM?
  // header[60] = lynxUseEeprom;
  myFile.write(header, LYNX_HEADER_SIZE);
}

static void readROM_LYNX() {
  dataDir_LYNX(INPUT);

  // The upper part of the address is used as a block address
  // There are always 256 blocks, but the size of the block can vary
  // So outer loop always steps through block addresses
  const uint32_t upto = LYNX_BLOCKCOUNT * LYNX_BLOCKADDR;
  for (uint32_t blockAddr = 0; blockAddr < upto; blockAddr += LYNX_BLOCKADDR) {
    draw_progressbar(blockAddr, upto);
    blinkLED();

    for (uint32_t i = 0; i < lynxBlockSize; i++) {
      uint32_t addr = blockAddr + i;
      if (lynxUseAudin && i >= lynxBlockSize / 2) {
        addr += LYNX_AUDIN_ON - lynxBlockSize / 2;
      }
      uint8_t byte = readByte_LYNX(addr);
      sdBuffer[i % 512] = byte;
      if ((i + 1) % 512 == 0) {
        myFile.write(sdBuffer, 512);
      }
    }
  }
  draw_progressbar(upto, upto);
}

#pragma region FLASH

#ifdef ENABLE_FLASH

static void writeByte_LYNX(uint32_t addr, uint8_t data) {
  digitalWrite(LYNX_OE, HIGH);
  digitalWrite(LYNX_WE, HIGH);
  setAddr_LYNX(addr);
  PORTC = data;
  digitalWrite(LYNX_WE, LOW);
  delayMicroseconds(20);
  digitalWrite(LYNX_WE, HIGH);
}

// Implements data complement status checking
// We only look at D7, or the highest bit of expected
void waitForDataComplement_LYNX(uint8_t expected) {
  dataDir_LYNX(INPUT);
  uint8_t status;
  do {
    digitalWrite(LYNX_OE, LOW);
    // one nop = 62.5ns
    // tOE = 30-50ns depending on flash
    NOP;
    status = PINC;
    digitalWrite(LYNX_OE, HIGH);
    // test highest bit
  } while ((status ^ expected) >> 7);

  dataDir_LYNX(OUTPUT);
}

static bool readHeader_LYNX() {
  uint32_t romSize = fileSize;

  print_Msg(F("Checking ROM..."));
  display_Update();

  char header[LYNX_HEADER_SIZE];
  myFile.read(header, LYNX_HEADER_SIZE);

  // Check for header to start with LYNX, assume valid .LNX header
  if (strncmp(header, LYNX, 4) == 0) {
    // Pull values from header
    lynxBlockSize = (header[5] << 8) | header[4];
    lynxUseAudin = header[59];
    romSize = fileSize - LYNX_HEADER_SIZE;
    println_Msg(FS(FSTRING_EMPTY));
  } else {
    // Header not valid, assume unheadered, rewind so we don't skip valid data
    println_Msg(F("[NO HEADER]"));
    myFile.seek(0);

    // Get block size from file size
    lynxBlockSize = fileSize / LYNX_BLOCKCOUNT;
    lynxUseAudin = lynxBlockSize >= 2048;
    romSize = fileSize;
  }

  print_Msg(F("AUDIN="));
  print_Msg(lynxUseAudin);
  print_Msg(F(" BLOCK="));
  println_Msg(lynxBlockSize);

  print_Msg(FS(FSTRING_ROM_SIZE));
  print_Msg(romSize / 1024);
  println_Msg(F("KB"));

  // Ensure valid block size, file size, and file fits in flash
  uint32_t expectedSize = (uint32_t)LYNX_BLOCKCOUNT * lynxBlockSize;
  if (lynxBlockSize % 256 != 0 || lynxBlockSize > LYNX_BLOCKADDR || lynxBlockSize <= 0) {
    println_Msg(FS(FSTRING_EMPTY));
    print_STR(error_STR, false);
    println_Msg(F("Invalid block size"));
    return waitPressButton_LYNX();
  }
  if (romSize != expectedSize) {
    println_Msg(FS(FSTRING_EMPTY));
    print_STR(error_STR, false);
    println_Msg(F("Invalid file size"));
    return waitPressButton_LYNX();
  }
  if (expectedSize > flashSize) {
    println_Msg(FS(FSTRING_EMPTY));
    print_STR(error_STR, false);
    print_STR(file_too_big_STR, true);
    return waitPressButton_LYNX();
  }

  println_Msg(FS(FSTRING_OK));
  display_Update();
  return true;
}

static bool detectFlash_LYNX() {
  print_Msg(F("Detecting flash..."));

  // SOFTWARE ID PROGRAM
  dataDir_LYNX(OUTPUT);
  writeByte_LYNX(0x5555, 0xAA);
  writeByte_LYNX(0x2AAA, 0x55);
  writeByte_LYNX(0x5555, 0x90);
  dataDir_LYNX(INPUT);
  // tIDA = 150ns
  NOP;
  NOP;
  NOP;
  // MFG,DEVICE
  uint16_t deviceId = (readByte_LYNX(0x0) << 8) | readByte_LYNX(0x1);

  // EXIT SOFTWARE ID PROGRAM
  dataDir_LYNX(OUTPUT);
  writeByte_LYNX(0x5555, 0xAA);
  writeByte_LYNX(0x2AAA, 0x55);
  writeByte_LYNX(0x5555, 0xF0);

  flashSize = 0;
  switch (deviceId) {
    case 0xBFB5:
      // SST39SF010 = 1Mbit
      flashSize = 131072UL;
      break;
    case 0xBFB6:
      // SST39SF020 = 2Mbit
      flashSize = 262144UL;
      break;
    case 0xBFB7:
      // SST39SF040 = 4Mbit
      flashSize = 524288UL;
      break;
      // case 0xC2A4:
      //   // MX29F040 = 4Mbit
      //   flashSize = 524288UL;
      //   break;
      // case 0xC2D5:
      //   // MX29F080 = 8Mbit
      //   flashSize = 1048576UL;
      //   break;
  }

  if (flashSize <= 0) {
    println_Msg(FS(FSTRING_EMPTY));
    print_STR(error_STR, false);
    println_Msg(F("Not recognized"));
    return waitPressButton_LYNX();
  }

  print_Msg(flashSize / 1024);
  println_Msg(F("KB"));
  display_Update();
  return true;
}

static void eraseFlash_LYNX() {
  print_Msg(F("Erasing..."));
  display_Update();

  // CHIP ERASE PROGRAM
  dataDir_LYNX(OUTPUT);
  writeByte_LYNX(0x5555, 0xAA);
  writeByte_LYNX(0x2AAA, 0x55);
  writeByte_LYNX(0x5555, 0x80);
  writeByte_LYNX(0x5555, 0xAA);
  writeByte_LYNX(0x2AAA, 0x55);
  writeByte_LYNX(0x5555, 0x10);
  waitForDataComplement_LYNX(0xFF);

  println_Msg(FS(FSTRING_OK));
  display_Update();
}

static void writeROM_LYNX() {
  filePath[0] = '\0';
  sd.chdir("/");
  fileBrowser(FS(FSTRING_SELECT_FILE));
  display_Clear();

  // HACK: openFlashFile() checks fileSize against flashSize for you, but this disregards header size:
  // if size(rom) == size(flash) but size(rom + header) > size(flash) we'd get a false negative
  // Pretend we have infinite flash size, allow openFlashFile() to pass this test, then do our own test in readHeader_LYNX()
  flashSize = ULONG_MAX;

  if (!openFlashFile()) return;

  if (!detectFlash_LYNX()) return;

  if (!readHeader_LYNX()) return;

  // Pause to read debug info
  // wait();
  // or alternately auto-advance
  delay(2000);
  display_Clear();

  eraseFlash_LYNX();

  print_STR(flashing_file_STR, true);
  display_Update();

  dataDir_LYNX(OUTPUT);
  uint8_t block[lynxBlockSize];
  const uint32_t upto = LYNX_BLOCKCOUNT * LYNX_BLOCKADDR;
  for (uint32_t blockAddr = 0; blockAddr < upto; blockAddr += LYNX_BLOCKADDR) {
    draw_progressbar(blockAddr, upto);
    blinkLED();

    myFile.read(block, lynxBlockSize);
    for (uint32_t i = 0; i < lynxBlockSize; i++) {
      uint32_t addr = blockAddr + i;
      if (lynxUseAudin && i >= lynxBlockSize / 2) {
        addr += LYNX_AUDIN_ON - lynxBlockSize / 2;
      }

      // BYTE PROGRAM
      uint8_t b = block[i];
      writeByte_LYNX(0x5555, 0xAA);
      writeByte_LYNX(0x2AAA, 0x55);
      writeByte_LYNX(0x5555, 0xA0);
      writeByte_LYNX(addr, b);

      waitForDataComplement_LYNX(b);
    }
  }
  draw_progressbar(upto, upto);

  myFile.close();
  dataDir_LYNX(INPUT);
  print_STR(done_STR, true);
  waitPressButton_LYNX();
}

#endif

#pragma region MENU

static const char PROGMEM LYNX_MENU_FLASH[] = "Program Flashcart";
static const char* const menuOptionsLYNX[] PROGMEM = { FSTRING_READ_ROM, LYNX_MENU_FLASH, FSTRING_RESET };

void lynxMenu() {
  size_t menuCount = sizeof(menuOptionsLYNX) / sizeof(menuOptionsLYNX[0]);
  convertPgm(menuOptionsLYNX, menuCount);
  uint8_t mainMenu = question_box(F("LYNX MENU"), menuOptions, menuCount, 0);
  display_Clear();
  display_Update();

  switch (mainMenu) {
    case 0:
      sd.chdir("/");
      createFolderAndOpenFile(LYNX, "ROM", romName, "lnx");
      detectCart_LYNX();
      writeHeader_LYNX();
      readROM_LYNX();
      myFile.close();
      sd.chdir("/");
      compareCRC("lynx.txt", 0, true, LYNX_HEADER_SIZE);
      print_STR(done_STR, true);
      waitPressButton_LYNX();
      break;

#ifdef ENABLE_FLASH
    case 1:
      writeROM_LYNX();
      break;
#endif

    case 2:
      resetArduino();
      break;

    default:
      print_MissingModule();
      break;
  }
}

#endif