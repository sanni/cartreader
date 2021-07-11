//******************************************
// NGP MODULE
//******************************************

#include "options.h"
#ifdef enable_NGP

static const char ngpMenuItem1[] PROGMEM = "Read Rom";
static const char ngpMenuItemReset[] PROGMEM = "Reset";
static const char* const menuOptionsNGP[] PROGMEM = {ngpMenuItem1, ngpMenuItemReset};

char ngpRomVersion[3];
uint8_t ngpSystemType;

void setup_NGP() {
  // A0 - A7
  DDRF = 0xff;
  // A8 - A15
  DDRK = 0xff;
  // A16 - A20
  DDRL = 0xff;

  // D0 - D7
  DDRC = 0x00;

  // controls
  // /CE0: PH3
  // /CE1: PH0
  // /OE:  PH6
  // /WE:  PH5
  // PWR:  PH4
  DDRH |= ((1 << 0) | (1 << 3) | (1 << 5) | (1 << 6));
  DDRH &= ~(1 << 4);

  PORTH |= ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6));

  if (getCartInfo_NGP())
  {
    showCartInfo_NGP();
  }
  else
  {
    println_Msg(F("NeoGeo Pocket"));
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F("Rom Size Error"));
    println_Msg(F(""));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
}

void ngpMenu() {
  uint8_t mainMenu;

  convertPgm(menuOptionsNGP, 2);
  mainMenu = question_box(F("NGP Menu"), menuOptions, 2, 0);

  switch (mainMenu)
  {
    case 0:
      sd.chdir("/");
      readROM_NGP(filePath, FILEPATH_LENGTH);
      sd.chdir("/");
      break;

    case 1:
      resetArduino();
      break;
  }

  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

bool getCartInfo_NGP() {
  uint8_t *tmp;

  // enter autoselect mode
  dataOut();
  writeByte_NGP(0x555, 0xaa);
  writeByte_NGP(0x2aa, 0x55);
  writeByte_NGP(0x555, 0x90);

  dataIn();

  cartSize = 0;
  tmp = (uint8_t*)&romSize;
  *(tmp + 0) = readByte_NGP(0);
  *(tmp + 1) = readByte_NGP(1);

  switch (romSize)
  {
      // 4 Mbits
      // Toshiba
      case 0xab98:
        cartSize = 524288;
        break;
      // Toshiba ?
      case 0x4c20:
        cartSize = 524288;
        break;      

      // 8 Mbits
      // Toshiba
      case 0x2c98:
        cartSize = 1048576;
        break;
      // Samsung
      case 0x2cec:
        cartSize = 1048576;
        break;

      // 16 Mbits
      // Toshiba
      case 0x2f98:
        cartSize = 2097152;
        break;
      // Samsung
      case 0x2fec:
        cartSize = 2097152;
        break;     
  }

  // reset to read mode
  dataOut();
  writeByte_NGP(0x000000, 0xf0);

  if (cartSize == 0)
    return false;

  dataIn();

  for (uint32_t addr = 0; addr < 28; addr++)
    sdBuffer[addr] = readByte_NGP(addr);

  if (memcmp_P(sdBuffer, PSTR("COPYRIGHT BY SNK CORPORATION"), 28) != 0 && memcmp_P(sdBuffer, PSTR(" LICENSED BY SNK CORPORATION"), 28) != 0)
    return false;

  snprintf(cartID, 5, "%02X%02X", readByte_NGP(0x000021), readByte_NGP(0x000020));
  snprintf(ngpRomVersion, 3, "%02X", readByte_NGP(0x000022));
  ngpSystemType = readByte_NGP(0x000023);

  for (uint32_t i = 0; i < 17; i++)
    romName[i] = readByte_NGP(0x24 + i);

  return true;
}

void showCartInfo_NGP() {
  display_Clear();

  println_Msg(F("NGP Cart Info"));

  print_Msg(F("Game: "));
  println_Msg(romName);

  print_Msg(F("GameID: "));
  println_Msg(cartID);

  print_Msg(F("Version: "));
  println_Msg(ngpRomVersion);

  print_Msg(F("System: "));
  if (ngpSystemType == 0)
      println_Msg(F("NGPMonochrome"));
  else if (ngpSystemType == 16)
      println_Msg(F("NGPColor"));
  else
      println_Msg(F("Unknown"));

  print_Msg(F("Rom Size: "));
  print_Msg((cartSize >> 17));
  println_Msg(F(" Mbits"));
  
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

void readROM_NGP(char *outPathBuf, size_t bufferSize) {
  // generate fullname of rom file
  snprintf(fileName, FILENAME_LENGTH, "%s.ngp", romName);

  // create a new folder for storing rom file
  EEPROM_readAnything(0, foldern);
  snprintf(folder, sizeof(folder), "NGP/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  // filling output file path to buffer
  if (outPathBuf != NULL && bufferSize > 0)
    snprintf(outPathBuf, bufferSize, "%s/%s", folder, fileName);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // open file on sdcard
  if (!myFile.open(fileName, O_RDWR | O_CREAT))
    print_Error(F("Can't create file on SD"), true);

  // write new folder number back to EEPROM
  foldern++;
  EEPROM_writeAnything(0, foldern);

  // back to read mode
  dataOut();
  writeByte_NGP(0x0, 0xf0);

  dataIn();
  for (uint32_t addr = 0; addr < cartSize; addr += 512) {

    // blink LED
    if ((addr & ((1 << 14) - 1)) == 0)
      PORTB ^= (1 << 4);

    for (uint32_t i = 0; i < 512; i++)
      sdBuffer[i] = readByte_NGP(addr + i);

    myFile.write(sdBuffer, 512);
  }

  myFile.close();
}

void writeByte_NGP(uint32_t addr, uint8_t data) {
  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x1f;
  PORTC = data;

  // which chip to select
  // 0x000000 - 0x1fffff -> /CE0
  // 0x200000 - 0x3fffff -> /CE1
  data = (addr & 0x00200000 ? (1 << 0) : (1 << 3));

  PORTH &= ~data;
  PORTH &= ~(1 << 5);
  NOP;

  PORTH |= data;
  PORTH |= (1 << 5);
  NOP; NOP;
}

uint8_t readByte_NGP(uint32_t addr) {
  uint8_t data;

  PORTF = addr & 0xff;
  PORTK = (addr >> 8) & 0xff;
  PORTL = (addr >> 16) & 0x1f;

  // which chip to select
  // 0x000000 - 0x1fffff -> /CE0
  // 0x200000 - 0x3fffff -> /CE1
  data = (addr & 0x00200000 ? (1 << 0) : (1 << 3));

  PORTH &= ~data;
  PORTH &= ~(1 << 6);
  NOP; NOP; NOP;

  data = PINC;

  PORTH |= data;
  PORTH |= (1 << 6);

  return data;
}

#endif
