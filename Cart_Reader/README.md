This Arduino code is written by a beginner for beginners, therefore I tried to comment every line of code. To an experienced coder this probably will seem very wrong but it really helps if you're just starting out.   

Every submodule has it's own setup_XX() function that configures the needed pins and a submenu that lets you choose what you want to do. 
The code directly addresses the pins via the DDR, PIN and PORT registers.   
Please also refer to the [pinout Excel sheet](https://github.com/sanni/cartreader/blob/master/pinout.xls).    
```
void setup_N64_Controller() {  
  // Output a low signal  
  PORTH &= ~(1 << 4);  
  // Set Controller Data Pin(PH4) to Input  
  DDRH &= ~(1 << 4);  
}  
```

Would be the same as this in a more traditional Arduino sketch:  
```
int dataPin = 7;   

void setup(){    
  // Output a low signal   
  digitalWrite(dataPin, LOW);   
  // Set controller data pin to input  
  pinMode(dataPin, INPUT);  
}  
```
To preserve memory every string is saved into the flash of the Arduino, also called progmem. This is done by using the F() macro.   
```
println_Msg(F("Press Button."));  
```
Also all the menus are stored in progmem and are only recalled to sram when needed.  
```
// N64 controller menu items  
const char N64ContMenuItem1[] PROGMEM = "Test Controller";  
const char N64ContMenuItem2[] PROGMEM = "Read ControllerPak";  
const char N64ContMenuItem3[] PROGMEM = "Write ControllerPak";  
const char N64ContMenuItem4[] PROGMEM = "Reset";  
const char* const menuOptionsN64Controller[] PROGMEM = {N64ContMenuItem1, N64ContMenuItem2, N64ContMenuItem3, N64ContMenuItem4};  
```
In an effort to keep the codebase as portable as possible instead of using the functions supplied by the OLED library directly to print out text, auxiliary functions like `println_Msg` are being used. So if you want to change to another display library you don't need to change all the code but only the helper functions. 
```
void print_Msg(long unsigned int message) {
  if (enable_OLED)
    display.print(message);
  if (enable_Serial)
    Serial.print(message);
}
```

For development purposes you can route all the text output to the Arduino's Serial Monitor instead of to the OLED screen. In this case you control the cart reader by typing numbers into the serial monitor corresponding to the action you want to perform. If you are asked to press the button just send a random number.   
```
// Comment out to change to Serial Output
// be sure to change the Arduino Serial Monitor to no line ending
//#define enable_OLED
```

To compile and upload the code please have a look at [this wiki article](https://github.com/sanni/cartreader/wiki/How-to-flash-the-Arduino).   

Needed libraries(already included in the portable Arduino IDE under Releases)   

SD lib: https://github.com/greiman/SdFat  
LCD lib: https://github.com/adafruit/Adafruit_SSD1306       
GFX Lib: https://github.com/adafruit/Adafruit-GFX-Library    
BusIO: https://github.com/adafruit/Adafruit_BusIO    
RGB Tools lib: https://github.com/joushx/Arduino-RGB-Tools  
SI5351 lib: https://github.com/etherkit/Si5351Arduino  
