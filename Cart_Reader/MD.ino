//******************************************
// SEGA MEGA DRIVE
//******************************************

/******************************************
   Menu
 *****************************************/
// MD menu items
static const char MDMenuItem1[] PROGMEM = "Read Rom";
static const char MDMenuItem2[] PROGMEM = "Read Save";
static const char MDMenuItem3[] PROGMEM = "Write Save";
static const char MDMenuItem4[] PROGMEM = "Write Flashcart";
static const char MDMenuItem5[] PROGMEM = "Reset";
static const char* const menuOptionsMD[] PROGMEM = {MDMenuItem1, MDMenuItem2, MDMenuItem3, MDMenuItem4, MDMenuItem5};

void mdMenu() {
  // create menu with title and 5 options to choose from
  unsigned char mainMenu;
  // Copy menuOptions out of progmem
  convertPgm(menuOptionsMD, 5);
  mainMenu = question_box("MEGA DRIVE Reader", menuOptions, 1, 0);

  // wait for user choice to come back from the question box menu
  switch (mainMenu)
  {
    case 0:
      display_Clear();
      // Change working dir to root
      sd.chdir("/");
      readROM_MD();
      //compare_checksum_MD();
      break;

      /*case 1:
        display_Clear();
        // Does cartridge have SRAM
        if () {
          // Change working dir to root
          sd.chdir("/");
          readSRAM_MD();
        }
        else {
          print_Error(F("Cart has no Sram"), false);
        }
        break;

        case 2:
        display_Clear();
        // Does cartridge have SRAM
        if () {
          // Change working dir to root
          sd.chdir("/");
          writeSRAM_MD();
          unsigned long wrErrors;
          wrErrors = verifySRAM_MD();
          if (wrErrors == 0) {
            println_Msg(F("Verified OK"));
            display_Update();
          }
          else {
            print_Msg(F("Error: "));
            print_Msg(wrErrors);
            println_Msg(F(" bytes "));
            print_Error(F("did not verify."), false);
          }
        }
        else {
          print_Error(F("Cart has no Sram"), false);
        }
        break;

        case 3:
        // Change working dir to root
        sd.chdir("/");
        writeFlash_MD();
        // Reset
        wait();
        asm volatile ("  jmp 0");
        break;

        case 4:
        asm volatile ("  jmp 0");
        break;*/
  }
  println_Msg(F(""));
  println_Msg(F("Press Button..."));
  display_Update();
  wait();
}

/******************************************
   Setup
 *****************************************/
void setup_MD() {
  // Set Address Pins to Output
  //A0-A7
  DDRF = 0xFF;
  //A8-A15
  DDRK = 0xFF;
  //A16-A23
  DDRL = 0xFF;

  // Set Control Pins to Output CS(PH3) WR(PH5) OE(PH6)
  DDRH |=  (1 << 3) | (1 << 5) | (1 << 6);

  // Set Data Pins (D0-D15) to Input
  DDRC = 0x00;
  DDRA = 0x00;

  // Setting WR(PH5) OE(PH6) HIGH
  PORTH |= (1 << 5) | (1 << 6);
  // Setting CS(PH3) LOW
  PORTH &= ~(1 << 3);

  delay(200);

  // Print all the info
  getCartInfo_MD();
}

/******************************************
   I/O Functions
 *****************************************/

/******************************************
  Low level functions
*****************************************/
void writeWord_MD(unsigned long myAddress, word myData) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;
  PORTC = myData;
  PORTA = (myData >> 8) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  // Wait till output is stable
  __asm__("nop\n\t");

  // Switch WR(PH5) to LOW
  PORTH &= ~(1 << 5);

  // Leave WR low for at least 60ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Switch WR(PH5) to HIGH
  PORTH |= (1 << 5);

  // Leave WR high for at least 50ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
}

word readWord_MD(unsigned long myAddress) {
  PORTF = myAddress & 0xFF;
  PORTK = (myAddress >> 8) & 0xFF;
  PORTL = (myAddress >> 16) & 0xFF;

  // Arduino running at 16Mhz -> one nop = 62.5ns
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Setting OE(PH6) LOW
  PORTH &= ~(1 << 6);

  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  // Read
  word tempWord = ( ( PINA & 0xFF ) << 8 ) | ( PINC & 0xFF );

  // Setting OE(PH6) HIGH
  PORTH |= (1 << 6);
  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");

  return tempWord;
}

// Switch data pins to write
void dataOut_MD() {
  DDRC = 0xFF;
  DDRA = 0xFF;
}

// Switch data pins to read
void dataIn_MD() {
  DDRC = 0x00;
  DDRA = 0x00;
}

/******************************************
  MEGA DRIVE functions
*****************************************/
void getCartInfo_MD() {
  cartSize = ((long(readWord_MD(0xD2)) << 16) | readWord_MD(0xD3)) + 1;
  sramSize = ((long(readWord_MD(0xDC)) << 16) | readWord_MD(0xDD)) - ((long(readWord_MD(0xDA)) << 16) | readWord_MD(0xDB)) + 2;

  // Get name
  for (byte c = 0; c < 48; c += 2) {
    // split word
    word myWord = readWord_MD((0x150 + c) / 2);
    byte loByte = myWord & 0xFF;
    byte hiByte = myWord >> 8;

    // write to buffer
    sdBuffer[c] = hiByte;
    sdBuffer[c + 1] = loByte;
  }
  byte myLength = 0;
  for (unsigned int i = 0; i < 48; i++) {
    if (((char(sdBuffer[i]) >= 48 && char(sdBuffer[i]) <= 57) || (char(sdBuffer[i]) >= 65 && char(sdBuffer[i]) <= 122)) && myLength < 15) {
      romName[myLength] = char(sdBuffer[i]);
      myLength++;
    }
  }

  display_Clear();
  println_Msg(F("Cart Info"));
  println_Msg(F(" "));
  print_Msg(F("Name: "));
  println_Msg(romName);
  print_Msg(F("Size: "));
  print_Msg(cartSize / 1024 / 1024);
  println_Msg(F("MB"));
  print_Msg(F("Sram: "));
  print_Msg(sramSize / 1024);
  println_Msg(F("KB"));

  // Wait for user input
  if (enable_OLED) {
    println_Msg(F(" "));
    println_Msg(F("Press Button..."));
    display_Update();
    wait();
  }
}

// Read rom and save to the SD card
void readROM_MD() {
  // Get name, add extension and convert to char array for sd lib
  strcpy(fileName, romName);
  strcat(fileName, ".MD");

  // create a new folder
  EEPROM_readAnything(10, foldern);
  sprintf(folder, "MD/ROM/%s/%d", romName, foldern);
  sd.mkdir(folder, true);
  sd.chdir(folder);

  display_Clear();
  print_Msg(F("Saving to "));
  print_Msg(folder);
  println_Msg(F("/..."));
  display_Update();

  // write new folder number back to eeprom
  foldern = foldern + 1;
  EEPROM_writeAnything(10, foldern);

  // Open file on sd card
  if (!myFile.open(fileName, O_RDWR | O_CREAT)) {
    print_Error(F("SD Error"), true);
  }

  word d = 0;
  for (unsigned long currBuffer = 0; currBuffer < cartSize / 2; currBuffer += 256) {
    // Blink led
    if (currBuffer % 16384 == 0)
      PORTB ^= (1 << 4);

    for (int currWord = 0; currWord < 256; currWord++) {
      word myWord = readWord_MD(currBuffer + currWord);
      // Split word into two bytes
      // Left
      sdBuffer[d] = (( myWord >> 8 ) & 0xFF);
      // Right
      sdBuffer[d + 1] = (myWord & 0xFF);
      d += 2;
    }
    myFile.write(sdBuffer, 512);
    d = 0;
  }
  // Close the file:
  myFile.close();
}

