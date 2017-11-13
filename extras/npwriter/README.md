This Arduino sketch can write Nintendo Power SF Memory cartridges without an external clock source like the Adafruit Clock Generator. It also uses the Arduino Serial Monitor instead of the OLED display and buttons.      

Copy the rom and map file to the root directory of your SD card and make sure that the filenames are below 8 characters in length.   

Configure the switches like so and use the USB port that is connected directly to the Arduino Mega:   

![image](https://dl.dropboxusercontent.com/s/yzewnnx5mb2ajk0/flash_firmware.jpg?dl=1)  

First send "8" to switch the cart to "hirom all" mode. Check that it did succeed:   
(8)HIROM ALL 0x04    
Success    
2A 4 2A 2A FE 61 A5 0    
In case it failed close the Serial Monitor and switch the Arduino Mega off and on again and start from the beginning.      

Next send "q" to unlock the write protection:   
(Q)Unlock WP   
Success.  
2A 4 2A 2A FE 61 A5 0    

Before you write anything you should always do a backup of your rom and mapping.    
To flash something new you need to erase both the flash and the mapping first before you send the write command.   

![](https://dl.dropboxusercontent.com/s/7ptv5hdf4iwi0lb/npwriter10.jpg?dl=1)    
