
#include <WaveHC.h>
#include <WaveUtil.h>
#include <Wire.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file 
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

#define PLAY_TIME 300

uint16_t fileIndex[90];
char fileLetter[] =  {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}; 

//////////////////////////////////// SETUP
void setup() {
  card.init();
  // enable optimized read - some cards may timeout
  card.partialBlockRead(true);
  vol.init(card);
  root.openRoot(vol);

  indexFiles(); 

  Wire.begin(0x67);                // join I2C bus with address 0x67
  Wire.onReceive(receiveEvent); // register event 
}

void loop() {
}

void indexFiles(void) {
  char name[7];
  
  // copy flash string to RAM
  strcpy_P(name, PSTR("xy.wav"));
  
  for (uint8_t i = 0; i < 9; i++) {
    for (uint8_t j = 1; j < 10; j++) {     
      // Make file name
      name[0] = fileLetter[i];
      name[1] = fileLetter[j];
      // Open file by name
      file.open(root, name);
      // Save file's index (byte offset of directory entry divided by entry size)
      // Current position is just after entry so subtract one.
      fileIndex[(10*i)+j] = root.readPosition()/32 - 1;   
    }
  }
}

void playByIndex(int index) {
  // start time
  uint32_t t = millis();
  
  // open by index
  file.open(root, fileIndex[index]);
  
  // create and play Wave
  wave.create(file);
  wave.play();

  // stop after PLAY_TIME ms
  while((millis() - t) < PLAY_TIME);
  wave.stop();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {    
playByIndex(Wire.read());        // receive byte as an integer
Wire.beginTransmission(0x66);    // transmit to device #0x66
Wire.write('y');                 // sends one byte
Wire.endTransmission();          // stop transmitting
}
