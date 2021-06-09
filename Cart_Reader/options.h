//******************************************
// OPTIONS
//******************************************
// Change mainMenu to snsMenu, mdMenu, n64Menu, gbxMenu, pcsMenu,
// flashMenu, nesMenu or smsMenu for single slot Cart Readers
#define startMenu mainMenu

// Comment out to change to Serial Output
// be sure to change the Arduino Serial Monitor to no line ending
#define enable_OLED

// Skip OLED start-up animation
//#define fast_start

// Enable the second button
#define enable_Button2

// Read N64 Eeprom with Adadruit clockgen, CLK1 switch needs to be switch to ON
#define clockgen_installed

// define enable_XXX to enable
#define enable_FLASH
#define enable_GBX
#define enable_MD
#define enable_N64
#define enable_NES
#define enable_NGP
#define enable_NP
#define enable_PCE
#define enable_SMS
#define enable_SNES
#define enable_SV
#define enable_WS
