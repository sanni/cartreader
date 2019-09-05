This is a simple Windows application to control the Cart Reader without the need of an OLED screen or buttons.   
Files are still loaded from and saved to the SD card.   

![image](https://dl.dropboxusercontent.com/s/txxlylb3kdt2mnb/crcontrol.jpg?dl=1)     

To enable the Serial Monitor follow the instructions on flashing the Cart_Reader sketch to the Arduino as shown here: [How to flash the Arduino Code to your Cart Reader](https://github.com/sanni/cartreader/wiki/How-to-flash-the-Arduino).    
**But before you hit upload you need to change one line in the Cart_Reader.ino tab like this:**   

```
// Comment out to change to Serial Ouput
// be sure to change the Arduino Serial Monitor to no line ending
//#define enable_OLED
```
