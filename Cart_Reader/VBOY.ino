//******************************************
// VIRTUALBOY MODULE
//******************************************
#ifdef ENABLE_VBOY
// Nintendo VirtualBoy
// Cartridge Pinout
// 60P 2.00mm pitch connector
//
//                 TOP SIDE             BOTTOM SIDE
//                           +-------+
//                      GND -| 1   2 |- GND
// /WE0 (SRAM WRITE ENABLE) -| 3   4 |- nc
//                       nc -| 5   6 |- /CS1 (SRAM ENABLE)
//               CS2 (SRAM) -| 7   8 |- VCC(+5V)
//                       nc -| 9  10 |- A23
//                      A19 -| 11 12 |- A22
//                      A18 -| 13 14 |- A21
//                       A8 -| 15 16 |- A20
//                       A7 -| 17 18 |- A9
//                       A6 -| 19 20 |- A10
//                       A5 -| 21 22 |- A11
//                       A4 -| 23 24 |- A12
//                       A3 -| 25 26 |- A13
//                       A2 -| 27 28 |- A14
//                       A1 -| 29 30 |- A15
//                /CE (ROM) -| 31 32 |- A16
//                      GND -| 33 34 |- A17
//                      /OE -| 35 36 |- VCC(+5V)
//                       D8 -| 37 38 |- D7
//                       D0 -| 39 40 |- D15
//                       D9 -| 41 42 |- D6
//                       D1 -| 43 44 |- D14
//                      D10 -| 45 46 |- D5
//                       D2 -| 47 48 |- D13
//                      D11 -| 49 50 |- D4
//                       D3 -| 51 52 |- D12
//                 VCC(+5V) -| 53 54 |- VCC(+5V)
//                       nc -| 55 56 |- nc
//                       nc -| 57 58 |- nc
//                      GND -| 59 60 |- GND
//                           +-------+
//

// CONTROL PINS:
// CS2(SRAM)               - (PH0) - VBOY PIN 7  - SNES RST
// /CE(ROM)                - (PH3) - VBOY PIN 31 - SNES /CS
// /CS1(SRAM ENABLE)       - (PH4) - VBOY PIN 6  - SNES /IRQ
// /WE0(SRAM WRITE ENABLE) - (PH5) - VBOY PIN 3  - SNES /WR
// /OE                     - (PH6) - VBOY PIN 35 - SNES /RD

// NOT CONNECTED:
// CLK(PH1) - N/C

//******************************************
// SETUP
//******************************************

void setup_VBOY() {
  // Request 5V
  setVoltage(VOLTS_SET_5V);

  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output
  //      CS2(PH0)   ---(PH1)   /CE(PH3)   /CS1(PH4)  /WE0(PH5)  /OE(PH6)
  DDRH |= (1 << 0) | (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

  // Set TIME(PJ0) to Output (UNUSED)
  DDRJ |= (1 << 0);

  // Set Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;

  // Setting Control Pins to HIGH
  //       ---(PH1)   /CE(PH3)   /CS1(PH4)  /WE0(PH5)  /OE(PH6)
  PORTH |= (1 << 1) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
  // Set CS2(PH0) to LOW
  PORTH &= ~(1 << 0);

  // Set Unused Pins HIGH
  PORTJ |= (1 << 0);  // TIME(PJ0)

  getCartInfo_VB();

  mode = CORE_VBOY;
}

//******************************************
//  MENU
//******************************************

// Base Menu
static const char* const menuOptionsVBOY[] PROGMEM = { FSTRING_READ_ROM, FSTRING_READ_SAVE, FSTRING_WRITE_SAVE, FSTRING_RESET };

void vboyMenu() {
  convertPgm(menuOptionsVBOY, 4);
  uint8_t mainMenu = question_box(F("VIRTUALBOY MENU"), menuOptions, 4, 0);

  switch (mainMenu) {
    case 0:
      // Read ROM
      sd.chdir("/");
      readROM_VB();
      sd.chdir("/");
      break;

    case 1:
      // Read SRAM
      if (sramSize) {
        sd.chdir("/");
        display_Clear();
        println_Msg(F("Reading SRAM..."));
        display_Update();
        readSRAM_VB();
        sd.chdir("/");
      } else {
        print_Error(F("Cart has no SRAM"));
      }
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 2:
      // Write SRAM
      if (sramSize) {
        // Change working dir to root
        sd.chdir("/");
        fileBrowser(F("Select SRM file"));
        display_Clear();
        writeSRAM_VB();
        writeErrors = verifySRAM_VB();
        if (writeErrors == 0) {
          println_Msg(F("SRAM verified OK"));
          display_Update();
        } else {
          print_STR(error_STR, 0);
          print_Msg(writeErrors);
          print_STR(_bytes_STR, 1);
          print_Error(did_not_verify_STR);
        }
      } else {
        print_Error(F("Cart has no SRAM"));
      }
#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
      // Wait for user input
      // Prints string out of the common strings array either with or without newline
      print_STR(press_button_STR, 1);
      display_Update();
      wait();
#endif
      break;

    case 3:
      // reset
      resetArduino();
      break;
  }
}

//******************************************
//  LOW LEVEL FUNCTIONS
//******************************************

void writeByte_VB(unsigned long myAddress, byte myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  __asm__("nop\n\t");

  PORTA = myData;

  // Set CS2(PH0), /CE(PH3), /OE(PH6) to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 6);
  // Set /CS1(PH4), /WE0(PH5) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 5);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 4) | (1 << 5);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");
}

word readWord_VB(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  __asm__("nop\n\t");

  // Set CS2(PH0), /CS1(PH4), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 4) | (1 << 5);
  // Set /CE(PH3), /OE(PH6) to LOW
  PORTH &= ~(1 << 3) & ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  word tempWord = ((PINA & 0xFF) << 8) | (PINC & 0xFF);

  // Set /CE(PH3), /OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);
  // Setting CS2(PH0) LOW
  PORTH &= ~(1 << 0);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempWord;
}

byte readByte_VB(unsigned long myAddress) {  // SRAM BYTE
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  __asm__("nop\n\t");

  // Set CS2(PH0), /CE(PH3), /WE0(PH5) to HIGH
  PORTH |= (1 << 0) | (1 << 3) | (1 << 5);
  // Set /CS1(PH4), /OE(PH6) to LOW
  PORTH &= ~(1 << 4) & ~(1 << 6);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  byte tempByte = PINA;

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  // Set /CS1(PH4), /OE(PH6) to HIGH
  PORTH |= (1 << 3) | (1 << 6);
  // Setting CS2(PH0) LOW
  PORTH &= ~(1 << 0);

  __asm__("nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t"
          "nop\n\t");

  return tempByte;
}

// Switch data pins to write
void dataOut_VB() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn_VB() {
  DDRC = 0x00;
  DDRA = 0x00;
}

//******************************************
// CART INFO
//******************************************

void getCartInfo_VB() {
  // Set control
  dataIn_VB();

  cartSize = 0;
  for (unsigned long address = 0x80000; address <= 0x400000; address *= 2) {
    // Get Serial
    word vbSerial = readWord_VB((address - 0x204) / 2);  // Cart Serial

    switch (vbSerial) {
      case 0x4D54:           // MT = Mario's Tennis
      case 0x4832:           // H2 = Panic Bomber/Tobidase! Panibomb
      case 0x5350:           // SP = Space Invaders
      case 0x5353:           // SS = Space Squash
      case 0x5452:           // TR = V-Tetris
        cartSize = 0x80000;  // 512KB
        break;

      case 0x494D:            // IM = Insmouse no Yakata
      case 0x4A42:            // JB = Jack Bros.
      case 0x4D43:            // MC = Mario Clash
      case 0x5245:            // RE = Red Alarm
      case 0x4833:            // H3 = Vertical Force
      case 0x5642:            // VB = Virtual Bowling
      case 0x4A56:            // JV = Virtual Lab
      case 0x5650:            // VP = Virtual League Baseball/Virtual Pro Yakyuu '95
        cartSize = 0x100000;  // 1MB
        break;

      case 0x5042:            // PB = 3-D Tetris
      case 0x4750:            // GP = Galactic Pinball
      case 0x5344:            // SD = SD Gundam Dimension War
      case 0x5442:            // TB = Teleroboxer
      case 0x5646:            // VF = Virtual Fishing
        cartSize = 0x100000;  // 1MB
        sramSize = 0x2000;    // 8KB
        break;

      case 0x5647:            // VG = Golf/T&E Virtual Golf
      case 0x4E46:            // NF = Nester's Funky Bowling
      case 0x5745:            // WE = Waterworld
        cartSize = 0x200000;  // 2MB
        break;

      case 0x5743:            // WC = Virtual Boy Wario Land
        cartSize = 0x200000;  // 2MB
        sramSize = 0x2000;    // 8KB
        break;

      case 0x4644:            // FD = Hyper Fighting
        cartSize = 0x400000;  // 4MB
        sramSize = 0x2000;    // 8KB
        break;
    }

    if (cartSize)
      break;
  }

  // Get name
  for (byte c = 0; c < 20; c += 2) {
    // split word
    word myWord = readWord_VB((cartSize - 0x220 + c) / 2);
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }
  byte myLength = 0;
  for (unsigned int i = 0; i < 20; i++) {
    if (((char(sdBuffer[i]) >= 48 && char(sdBuffer[i]) <= 57) || (char(sdBuffer[i]) >= 65 && char(sdBuffer[i]) <= 122)) && myLength < 15) {
      romName[myLength] = char(sdBuffer[i]);
      myLength++;
    }
  }

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(FS(FSTRING_SPACE));
  print_Msg(FS(FSTRING_NAME));
  println_Msg(romName);
  print_Msg(FS(FSTRING_SIZE));
  print_Msg(cartSize * 8 / 1024 / 1024);
  println_Msg(F(" MBit"));
  print_Msg(F("Sram: "));
  if (sramSize > 0) {
    print_Msg(sramSize * 8 / 1024);
    println_Msg(F(" KBit"));
  } else
    println_Msg(F("None"));
  println_Msg(FS(FSTRING_SPACE));

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  // Wait for user input
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
}

//******************************************
// READ CODE
//******************************************

void readROM_VB() {
  dataIn_VB();

  createFolderAndOpenFile("VBOY", "ROM", romName, "vb");

  word d = 0;
  uint32_t progress = 0;
  draw_progressbar(0, cartSize);
  // HYPER FIGHTING FIX
  // VIRTUAL BOY ADDRESSING IS TOP DOWN
  // ONLY FOR HYPER FIGHTING PLUGIN WITH ALL ADDRESS LINES CONNECTED
  // NORMAL PLUGIN DOES NOT HAVE THE UPPER ADDRESS LINES CONNECTED
  if (cartSize == 0x400000) {
    unsigned long romData = 0x1000000 - (cartSize / 2);
    for (unsigned long currBuffer = romData; currBuffer < 0x1000000; currBuffer += 256) {
      for (int currWord = 0; currWord < 256; currWord++) {
        word myWord = readWord_VB(currBuffer + currWord);
        // Split word into two bytes
        sdBuffer[d] = ((myWord >> 8) & 0xFF);
        sdBuffer[d + 1] = (myWord & 0xFF);
        d += 2;
      }
      myFile.write(sdBuffer, 512);
      d = 0;
      progress += 512;
      draw_progressbar(progress, cartSize);
    }
  } else {
    for (unsigned long currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 256) {
      for (int currWord = 0; currWord < 256; currWord++) {
        word myWord = readWord_VB(currBuffer + currWord);
        // Split word into two bytes
        sdBuffer[d] = ((myWord >> 8) & 0xFF);
        sdBuffer[d + 1] = (myWord & 0xFF);
        d += 2;
      }
      myFile.write(sdBuffer, 512);
      d = 0;
      progress += 512;
      draw_progressbar(progress, cartSize);
    }
  }
  myFile.close();

  // Compare CRC32 to database and rename ROM if found
  // Arguments: database name, precalculated crc string or 0 to calculate, rename rom or not, starting offset
  compareCRC("vb.txt", 0, 1, 0);

#if (defined(ENABLE_OLED) || defined(ENABLE_LCD))
  // Wait for user input
  println_Msg(FS(FSTRING_EMPTY));
  // Prints string out of the common strings array either with or without newline
  print_STR(press_button_STR, 1);
  display_Update();
  wait();
#endif
}

//******************************************
// SRAM
//******************************************

void writeSRAM_VB() {
  dataOut_VB();

  sprintf(filePath, "%s/%s", filePath, fileName);
  println_Msg(F("Writing..."));
  println_Msg(filePath);
  display_Update();

  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currByte = 0; currByte < sramSize; currByte++) {
      //        writeWord_VB(currByte, ((myFile.read() << 8 ) & 0xFF));
      writeByte_VB(currByte, (myFile.read()));
    }
    myFile.close();
    print_STR(done_STR, 1);
    display_Update();
  } else {
    print_FatalError(sd_error_STR);
  }
  dataIn_VB();
}

void readSRAM_VB() {
  dataIn_VB();

  createFolder("VBOY", "SAVE", romName, "srm");

  foldern = foldern + 1;
  EEPROM_writeAnything(0, foldern);

  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_FatalError(sd_error_STR);
  }
  for (unsigned long currBuffer = 0; currBuffer < sramSize; currBuffer += 512) {
    for (int currByte = 0; currByte < 512; currByte++) {
      byte myByte = readByte_VB(currBuffer + currByte);
      sdBuffer[currByte] = myByte;
    }
    myFile.write(sdBuffer, 512);
  }
  myFile.close();
  print_Msg(F("Saved to "));
  print_Msg(folder);
  println_Msg(F("/"));
  display_Update();
}

unsigned long verifySRAM_VB() {
  dataIn_VB();
  writeErrors = 0;

  if (myFile.open(filePath, O_READ)) {
    for (unsigned long currBuffer = 0; currBuffer < sramSize; currBuffer += 512) {
      for (int currByte = 0; currByte < 512; currByte++) {
        byte myByte = readByte_VB(currBuffer + currByte);
        sdBuffer[currByte] = myByte;
      }
      for (int i = 0; i < 512; i++) {
        if (myFile.read() != sdBuffer[i]) {
          writeErrors++;
        }
      }
    }
    myFile.close();
  } else {
    print_FatalError(sd_error_STR);
  }

  return writeErrors;
}
#endif
//******************************************
// End of File
//******************************************