This is the hex file that needs to be flashed to the PIC12F629 if you want to dump SA1 SNES games.  
Source: https://github.com/mrehkopf/sd2snes/tree/master/cic   

You can build a simple PIC flasher for the COM port using the schematic provided here but you need to adapt the pinout from the PIC16F84 to the PIC12F629.  

![image](https://github.com/sanni/cartreader/blob/master/extras/snesCIC/pic_prog_schematic.gif)    
      
![image](https://dl.dropboxusercontent.com/s/7c6ql87mu4dh70a/12f629.jpg?dl=1)   

In WxPic choose "COM84 programmer for serial port" as interface type.   
You might also need to copy the PIC12F629.dev file from MPLAB into the devices folder.   
