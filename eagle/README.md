With these files you can easily make your own adapters.    
For best compatibility be sure to use the win32 version of Eagle 7.6.0 downloaded from [here](http://eagle.autodesk.com/eagle/software-versions/2).    
All PCBs were autorouted with FreeRouting for which you need the [Java 8 RE](https://www.oracle.com/technetwork/java/javase/downloads/jre8-downloads-2133155.html).    

#### Quick tutorial on modifying an adapter PCB:     
- Install Eagle 7.6.0 Win32    
- Copy the cam, dru, lbr and ulp folders to the Eagle main dir.    
- Open the XXX_adapter.brd file that you want to modify in Eagle PCBs.    
- In the textfield type ripup; and execute by pressing Enter key twice.    
- Go to Schematic and Board view and do your changes, watch basic Eagle tutorials to learn how.    
- Once everything is done. Go back to Board view, then File ->  Run ULP -> eagle2freerouterV6-6.ulp -> Create DSN file.     
- In Windows Explorer execute FreeRouting1.jar in ulp dir and open the XXX_adapter.dsn file you created in the previous step.     
- Let it autoroute for some hours, once finished choose Export Eagle Session script.     
- Back in Eagle in the Board view go File -> Execute Script and select the XXX_adapter.scr you just exported.     
- Finally go File -> Cam Processor, File -> Open -> Job and select Elecrow_Gerber_Generater_DrillAlign.cam, then Process Job to export the Gerber files.    