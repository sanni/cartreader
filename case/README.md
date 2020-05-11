# Print settings for Slic3r Prusa Edition     
Select the "0.20mm NORMAL" setting that comes with Slic3r and change the following parameters:    
ensure_vertical_shell_thickness = 0     
fill_density = 5%    
first_layer_height = 0.3    
perimeters = 1    
thin_walls = 1    

# Print settings for Cura 4.x    
You can use the Draft 0.2mm profile but you might want to change the build plate adhesion settings since the default seems a bit ![too much](https://www.dropbox.com/s/qu8fvjsh33bxjeh/curaprint.jpg?dl=0).  
For the N64 sleeve you need to enable the special mode "Spiralize outer Contour" or else the walls will be too thick.  

# Combined STL file with all parts      

![image](https://dl.dropboxusercontent.com/s/zbrvq9lf6w6ye3l/combined.png?dl=1)   

# Separate STL files for each part    

#### snes_spacer.stl is a small 3d printed spacer to relieve pressure from the pins of SNES clone console slot   

![image](https://dl.dropboxusercontent.com/s/07slhy8pi9ujiri/snes_spacer.png?dl=1)   

#### n64sleeve.stl helps with aligning N64 cartridges       
![image](https://dl.dropboxusercontent.com/s/8muv7x1fiz2n169/n64sleeve.png?dl=1)   

#### backplate_simple.stl is the backplate that holds the Arduino    

![image](https://dl.dropboxusercontent.com/s/10pbg2umbp6ylv5/backplate_simple.png?dl=1)    

#### microsd_spacer holds the RGB led and fits between the micro SD card and the Cart Reader pcb (print with clear PLA)    

![image](https://dl.dropboxusercontent.com/s/ch045dkev7al9v7/msdspacer.png?dl=1)

#### sidewall_tabs.stl secures the Arduino Mega to the backplate with five M2x8 screws    

![image](https://dl.dropboxusercontent.com/s/p7v2l37f1c130b4/sidewall.png?dl=1)  

#### case_xxx.stl are a replacement for the more simple backplate in case you prefer a full enclosure   

![image](https://dl.dropboxusercontent.com/s/lzgrrkm5yfflll9/v17_case2.png?dl=1) 

#### The rest of the STL files are alternative designs in case you use the battery add-on.    
