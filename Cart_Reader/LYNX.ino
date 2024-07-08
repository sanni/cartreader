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
// Version 1.0
// Future enhancements
// 1. EEPROM read/write
// 2. Homebrew flash cart programming
//
#ifdef ENABLE_LYNX

#pragma region DEFS

#define LYNX_HEADER_SIZE 64
#define LYNX_WE 8
#define LYNX_OE 9
#define LYNX_AUDIN 46
#define LYNX_BLOCKADDR 2048UL
#define LYNX_BLOCKCOUNT 256UL
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
  pinMode(LYNX_AUDIN, OUTPUT);
  digitalWrite(LYNX_WE, HIGH);
  digitalWrite(LYNX_OE, HIGH);
  digitalWrite(LYNX_AUDIN, HIGH);

  strcpy(romName, LYNX);
  mode = CORE_LYNX;
}

static void dataDir_LYNX(byte direction) {
  DDRC = (direction == OUTPUT) ? 0xff : 0x00;
}

static uint8_t readByte_LYNX(uint32_t addr, uint8_t audin = 0) {
  digitalWrite(LYNX_OE, HIGH);
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = ((addr >> 16) & 0b111) | (audin << 3);
  digitalWrite(LYNX_OE, LOW);
  delayMicroseconds(20);
  uint8_t data = PINC;
  digitalWrite(LYNX_OE, HIGH);
  return data;
}

#pragma region HIGHLEVEL

static bool detectCart_LYNX() {
  // Could omit logging to save a few bytes
  display_Clear();
  println_Msg(F("Identifying..."));

  lynxUseAudin = false;
  lynxBlockSize = 0;

  // Somewhat arbitrary, however many bytes would be unlikely to be
  // coincidentally mirrored
  const size_t DETECT_BYTES = 128;

  for (int i = 0; i < DETECT_BYTES; i++) {
    uint8_t b = readByte_LYNX(i);
    // If any differences are detected when AUDIN=1, AUDIN is used to bankswitch
    // meaning we also use the maximum block size
    // (1024kb cart / 256 blocks = 4kb block bank switched between lower/upper 2kb blocks)
    if (b != readByte_LYNX(i, 1)) {
      lynxUseAudin = true;
      lynxBlockSize = 2048;
      break;
    }
    // Identify mirroring of largest stride
    // Valid cart sizes of 128kb, 256kb, 512kb / 256 blocks = block sizes of 512b, 1024b, 2048b
    if (b != readByte_LYNX(i + 1024)) {
      lynxBlockSize = max(lynxBlockSize, 2048);
    } else if (b != readByte_LYNX(i + 512)) {
      lynxBlockSize = max(lynxBlockSize, 1024);
    } else if (b != readByte_LYNX(i + 256)) {
      lynxBlockSize = max(lynxBlockSize, 512);
    }
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

// Saves memory by using existing sd buffer instead of a second block-sized buffer (which could be up to 2KB)
// Minimum block size is 512b, size of sdBuffer is 512b, all block sizes multiples of 512b,
// so we shouldn't need to check for leftovers...
static inline void ringBufferWrite_LYNX(uint32_t blocki, uint8_t byte) {
  sdBuffer[blocki % 512] = byte;
  if ((blocki + 1) % 512 == 0) {
    myFile.write(sdBuffer, 512);
  }
}

static void readROM_LYNX() {
  dataDir_LYNX(INPUT);

  // The upper part of the address is used as a block address
  // There are always 256 blocks, but the size of the block can vary
  // So outer loop always steps through block addresses

  uint32_t i;
  const uint32_t upto = LYNX_BLOCKCOUNT * LYNX_BLOCKADDR;
  for (uint32_t blockAddr = 0; blockAddr < upto; blockAddr += LYNX_BLOCKADDR) {
    draw_progressbar(blockAddr, upto);
    blinkLED();

    if (lynxUseAudin) {
      // AUDIN bank switching uses a 4kb block split to 2 banks
      for (i = 0; i < lynxBlockSize / 2; i++) {
        ringBufferWrite_LYNX(i, readByte_LYNX(blockAddr + i, 0));
      }
      for (; i < lynxBlockSize; i++) {
        ringBufferWrite_LYNX(i, readByte_LYNX(blockAddr + i - (lynxBlockSize / 2), 1));
      }
    } else {
      for (i = 0; i < lynxBlockSize; i++) {
        ringBufferWrite_LYNX(i, readByte_LYNX(i + blockAddr));
      }
    }
  }
  draw_progressbar(upto, upto);
}

#pragma region MENU

static const char* const menuOptionsLYNX[] PROGMEM = {FSTRING_READ_ROM, FSTRING_RESET};

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
      display_Update();
      wait();
      break;
  }
}

#endif