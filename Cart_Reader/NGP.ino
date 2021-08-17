//******************************************
// NGP MODULE
//******************************************

#include "options.h"
#ifdef enable_NGP

static const char ngpMenuItem1[] PROGMEM = "Read Rom";
static const char ngpMenuItemReset[] PROGMEM = "Reset";
static const char* const menuOptionsNGP[] PROGMEM = {ngpMenuItem1, ngpMenuItemReset};

static const char ngpRomItem1[] PROGMEM = "4 Mbits";
static const char ngpRomItem2[] PROGMEM = "8 Mbits";
static const char ngpRomItem3[] PROGMEM = "16 Mbits";
static const char ngpRomItem4[] PROGMEM = "32 Mbits";
static const char* const ngpRomOptions[] PROGMEM = {ngpRomItem1, ngpRomItem2, ngpRomItem3, ngpRomItem4};

char ngpRomVersion[3];
uint8_t ngpSystemType;
uint8_t manufacturerID;
uint8_t deviceID;

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
    printCartInfo_NGP();
  else {
    println_Msg(F("NeoGeo Pocket"));
    println_Msg(F(""));
    println_Msg(F(""));
    println_Msg(F("Cartridge read error"));
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

  switch (mainMenu) {
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
  
  // get chip manufacturer and device IDs 
  manufacturerID = readByte_NGP(0);
  deviceID = readByte_NGP(1);
  tmp = (uint8_t*)&romSize;
  *(tmp + 0) = deviceID;
  *(tmp + 1) = manufacturerID;

  switch (romSize) {
    // detection error
    case 0xffff:
      return false;
      break;
    
    // 4 Mbits
    case 0x98ab:
      cartSize = 524288;
      break;
    // Toshiba ?
    case 0x204c:
      cartSize = 524288;
      break;      

    // 8 Mbits
    // Toshiba
    case 0x982c:
      cartSize = 1048576;
      break;
    // Samsung
    case 0xec2c:
      cartSize = 1048576;
      break;

    // 16 Mbits
    // Toshiba
    case 0x982f:
      cartSize = 2097152;
      break;
    // Samsung
    case 0xec2f:
      cartSize = 2097152;
      break;
  }

  // reset to read mode
  dataOut();
  writeByte_NGP(0x0, 0xf0);

  dataIn();

  // confirm NGP cart recognition
  for (uint32_t addr = 0; addr < 28; addr++)
    sdBuffer[addr] = readByte_NGP(addr);
  if (memcmp_P(sdBuffer, PSTR("COPYRIGHT BY SNK CORPORATION"), 28) != 0 && memcmp_P(sdBuffer, PSTR(" LICENSED BY SNK CORPORATION"), 28) != 0)
    return false;

  // get app ID
  snprintf(cartID, 5, "%02X%02X", readByte_NGP(0x21), readByte_NGP(0x20));
  
  // force rom size to 32Mbits for few titles
  if (strcmp(cartID,"0060") == 0 || strcmp(cartID,"0061") == 0 || strcmp(cartID,"0069") == 0 )
    cartSize = 4194304;
  
  // get app version 
  snprintf(ngpRomVersion, 3, "%02X", readByte_NGP(0x22));
  
  // get app system compatibility
  ngpSystemType = readByte_NGP(0x23);

  // get app name
  for (uint32_t i = 0; i < 17; i++)
    romName[i] = readByte_NGP(0x24 + i);

  return true;
}

void printCartInfo_NGP() {
  display_Clear();

  println_Msg(F("NGP Cart Info"));

  print_Msg(F("Name: "));
  println_Msg(romName);

  print_Msg(F("ID: "));
  println_Msg(cartID);

  print_Msg(F("Version: "));
  println_Msg(ngpRomVersion);

  print_Msg(F("System: "));
  if (ngpSystemType == 0x0)
    println_Msg(F("NGPMonochrome"));
  else if (ngpSystemType == 0x10)
    println_Msg(F("NGPColor"));
  else
    println_Msg(F("Unknown"));

  print_Msg(F("Rom Size: "));
  if (cartSize == 0) {
    println_Msg(F("Unknown"));
    print_Msg(F("Chip ID: "));
    println_Msg(String(manufacturerID,HEX) + " " + String(deviceID,HEX));
  }
  else {
    print_Msg((cartSize >> 17));
    println_Msg(F(" Mbits"));
  }

  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

void readROM_NGP(char *outPathBuf, size_t bufferSize) {
  // Set cartsize manually if chip ID is unknown
  if (cartSize == 0) {
    unsigned char ngpRomMenu;
    
    // Copy menuOptions out of progmem
    convertPgm(ngpRomOptions, 4);
    ngpRomMenu = question_box(F("Select ROM size"), menuOptions, 4, 0);

    // wait for user choice to come back from the question box menu
    switch (ngpRomMenu) {
      case 0:
        // 4 Mbits
        cartSize = 524288;
        break;

      case 1:
        // 8 Mbits
        cartSize = 1048576;
        break;

      case 2:
        // 16 Mbits
        cartSize = 2097152;
        break;

      case 3:
        // 32 Mbits
        cartSize = 4194304;
        break;
    }
  }
  
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

  // read rom
  dataIn();
  for (uint32_t addr = 0; addr < cartSize; addr += 512) {
    // blink LED
    if ((addr & ((1 << 14) - 1)) == 0)
      PORTB ^= (1 << 4);

    // read block
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
