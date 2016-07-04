/**********************************************************************************
 Nintendo Power Splitter
 
 Author:           sanni
 Date:             2016-05-02
 Version:          V1
 
 Compiled with Processing 3.0.2
 **********************************************************************************/

/******************************************
 Variables
 *****************************************/
// Define 4MB byte array that stores the file
byte[] NP = new byte[4194303];

// Define 12 byte array to store menu string
byte menuString[] = {0x4D, 0x45, 0x4E, 0x55, 0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D};

// Menu variables
boolean npMenu = true;
byte numGames = 0;

// rom info arrays
char[][] romName = new char[8][21];
int[] romAddress = new int[8];
int[] romSize = new int[8];
char[][] romCode = new char[8][12];
boolean[] hirom = new boolean[8];

/******************************************
 Setup
 *****************************************/
void setup() {
  // Make 500x300 pixel window with black background
  size(500, 300);
  background(0);
  
  NP[4194302] = 0x66;
  
  // Write title
  text("Nintendo Power Splitter V1", 20, 20);

  // File open dialog
  selectInput("Select 4MB NP dump:", "fileSelected");
  noLoop();
}

/******************************************
 Helper Functions
 *****************************************/
// Loads selected file into byte array
void fileSelected(File selection) {
  if (selection != null) 
    NP = loadBytes(selection);
}

/******************************************
 Main function
 *****************************************/
void draw() {
  // Wait until array is filled with the 4MB file
  while (NP[4194302] == 0x66) {
    delay(200);
  }

  // Check if menu is present
  for (int i = 0; i < 12; i++) {
    if (menuString[i] != NP[0x7FC0+i]) {
      npMenu = false;
    }
  }
  if (npMenu) {
    // Count number of games
    for (int i = 0x60000; i < 0x6E000; i += 0x2000) {
      if (NP[i] == numGames )
        numGames++;
    }
    text("Number of games: " + (numGames), 20, 60);

    // Get game info
    for (int i = 0; i < numGames; i++) {

      // Read starting address and size
      romAddress[i] = NP[0x60000 + i*0x2000 + 0x01] * 0x80000;
      romSize[i] = NP[0x60000 + i*0x2000 + 0x03] * 131072;

      // Read game code
      for (int j = 0; j < 12; j++) {
        romCode[i][j] = char(NP[0x60000 + i*0x2000 + 0x07 + j]);
      }

      //check if hirom
      if (NP[romAddress[i]+0xFFD5] == 0x31) {
        hirom[i] = true;
      } else {
        hirom[i] = false;
      }

      // Read rom name
      for (int j = 0; j < 21; j++) {
        if (hirom[i]) {
          romName[i][j] = char(NP[romAddress[i]+0xFFC0+j]);
        } else {
          romName[i][j] = char(NP[romAddress[i]+0x7FC0+j]);
        }
      }
      // Convert char array to String to be printed
      String tempStr1 = new String(romName[i]);

      // Convert char array to String to be printed
      String tempStr2 = new String(romCode[i]);

      // Clean the Strings
      tempStr1 = tempStr1.trim();
      tempStr1 = tempStr1.replaceAll("-$", "");
      tempStr1 = tempStr1.trim(); 
      tempStr2 = tempStr2.trim();
      tempStr2 = tempStr2.replaceAll("-$", "");
      tempStr2 = tempStr2.trim(); 

      // Print all info
      text("Game" + i + ": " + tempStr1 + " " + tempStr2 +  " Addr: 0x" + hex(romAddress[i]) + " Size: " + romSize[i]/1024 + "KB", 20, 80+i*20);
    }

    // Split the rom
    for (int i = 0; i < numGames; i++) {
      byte[] OUT = new byte[romSize[i]];
      String fileName = new String(romCode[i]);

      // Clean the String
      fileName = fileName.trim();
      fileName = fileName.replaceAll("-$", "");
      fileName = fileName.trim(); 

      // Create the array to be written to a file
      for (int j = 0; j < romSize[i]; j++) {
        OUT[j] = NP[romAddress[i] + j];
      }
      // Write the file
      saveBytes(fileName + ".bin", OUT);
    }
  } else {
    text("NP menu not found.", 20, 60);
  }
}
//******************************************
// End of File
//******************************************