
#include <WaveHC.h>
#include <WaveUtil.h>
#include <Wire.h>

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time

const String fileformat = ".wav";

//////////////////////////////////// SETUP
void setup() {
  card.init();
  // enable optimized read - some cards may timeout
  card.partialBlockRead(true);
  vol.init(card);
  root.openRoot(vol);

  Serial.begin(115200);

  Wire.begin(0x67);                // join I2C bus with address 0x67
  Wire.onReceive(receiveEvent); // register event
}

void loop() {
}


// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  while (1 < Wire.available()); // loop through all but the last
  int recieved = Wire.read();

  String filename = String(recieved) + fileformat;
  char toopen[7];
  filename.toCharArray(toopen, 7);
  Serial.println(toopen);
  // open by index
  file.open(root, toopen);

  // create and play Wave
  wave.create(file);
  wave.play();

}
