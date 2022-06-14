/* -------------------------------------------------------------------------- *
   Program to run the RingRider platform.
   It is based on the libraries GY521, and FastLED.
 *                                                                            *
   It converts the data transmitted from GY521 modules that are attached to
   spring riders into a conterller like feed for interaction with LED rings.
   This code also runs a collection of games using the controller/LED circle.
 *                                                                            *
   Program written by Luiz F.S. Simão in the first half of 2022.
   -------------------------------------------------------------------------- */

#include "GY521.h"
#include "math.h"
#include <FastLED.h>

// Define the general set of hardware ports and parameters
#define BUZZ_PIN    8
#define LED_PIN     5
#define NUM_LEDS    90
#define BRIGHTNESS  15
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
GY521 sensor(0x68); // this is class that connects to the GY521 module and defines the adress. AD0 connected to GND => 0x68 AD0 connected to VCC => 0x69

CRGB leds[NUM_LEDS]; // The accessable LED array.

// Constructing core varialbes.
uint32_t pitch;
const float threshold = 0.25; // used to define how much rotation is necessary to be considered as "bending". 1.5ish is fully flush agaisnt the floor; 0 is perfectly upright
double pitchR, rollR, planeX, planeY, projectAng;
bool forward_F, forward_h, backward_F, backward_h, left_F, left_h, right_F, right_h, xNull, yNull;

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);

  Wire.begin();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  delay(100);
  while (sensor.wakeup() == false) // establishes connection to the module
  {
    Serial.print(millis());
    Serial.println("\tCould not connect to GY521");
    delay(1000);
  }
  sensor.setAccelSensitivity(2);  // 16g
  sensor.setGyroSensitivity(1);   // 500 degrees/s

  sensor.setThrottle();
  Serial.println("start...");

  noTone(BUZZ_PIN);

  randomSeed(analogRead(5));

  // set calibration values that can be obtained with /Examples/GY521/readCalibration
  sensor.axe = -0.896;
  sensor.aye = 0.519;
  sensor.aze = 0.282;
  sensor.gxe = 0.900;
  sensor.gye = 0.628;
  sensor.gze = -0.590;
}


void loop()
{
  // Reset states
  forward_F  = false;
  forward_h  = false;
  backward_F = false;
  backward_h = false;
  left_F     = false;
  left_h     = false;
  right_F    = false;
  right_h    = false;
  xNull      = false;
  yNull      = false;

  // Obtain sensor data and convert the input into some reasonable range
  sensor.read();
  pitchR = (sensor.getPitch() * M_PI) / 180.0;
  rollR  = (sensor.getRoll() * M_PI) / 180.0;
  planeY = sin(pitchR);
  planeX = sin(rollR);

  // Defines currently active state
  if      (planeY < -threshold)       left_F     = true;
  else if (planeY < -threshold / 2)   left_h     = true;
  if      (planeY > threshold)        right_F    = true;
  else if (planeY > threshold  / 2)   right_h    = true;
  if      (planeX < -threshold)       forward_F  = true;
  else if (planeX < -threshold / 2)   forward_h  = true;
  if      (planeX > threshold)        backward_F = true;
  else if (planeX > threshold  / 2)   backward_h = true;
  if (!forward_h && !backward_h &&
      !forward_F && !backward_F)      xNull      = true;
  if (!left_h    && !right_h &&
      !left_F    && !right_F)         yNull      = true;

  for (int LED = 0; LED < NUM_LEDS; LED++) leds[LED] = CRGB::Black; // Turns off all LEDs

  // Resets the buzzer
  delay(5);
  noTone(BUZZ_PIN);

  // Matches the active states with a quadrant of a circle (the LED circle)
  for (int LED = 0; LED < NUM_LEDS; LED++) {
    if (LED <= int(NUM_LEDS / 16)                                       && xNull      && left_F)  glowNbuzz(LED, 1);
    if (LED <= int(2  * NUM_LEDS / 16) && LED > int(NUM_LEDS / 16)      && forward_h  && left_F)  glowNbuzz(LED, 2);
    if (LED <= int(3  * NUM_LEDS / 16) && LED > int(2  * NUM_LEDS / 16) && forward_F  && left_F)  glowNbuzz(LED, 3);
    if (LED <= int(4  * NUM_LEDS / 16) && LED > int(3  * NUM_LEDS / 16) && forward_F  && left_h)  glowNbuzz(LED, 4);
    if (LED <= int(5  * NUM_LEDS / 16) && LED > int(4  * NUM_LEDS / 16) && forward_F  && yNull)   glowNbuzz(LED, 5);
    if (LED <= int(6  * NUM_LEDS / 16) && LED > int(5  * NUM_LEDS / 16) && forward_F  && right_h) glowNbuzz(LED, 6);
    if (LED <= int(7  * NUM_LEDS / 16) && LED > int(6  * NUM_LEDS / 16) && forward_F  && right_F) glowNbuzz(LED, 7);
    if (LED <= int(8  * NUM_LEDS / 16) && LED > int(7  * NUM_LEDS / 16) && forward_h  && right_F) glowNbuzz(LED, 8);
    if (LED <= int(9  * NUM_LEDS / 16) && LED > int(8  * NUM_LEDS / 16) && xNull      && right_F) glowNbuzz(LED, 9);
    if (LED <= int(10 * NUM_LEDS / 16) && LED > int(9  * NUM_LEDS / 16) && backward_h && right_F) glowNbuzz(LED, 10);
    if (LED <= int(11 * NUM_LEDS / 16) && LED > int(10 * NUM_LEDS / 16) && backward_F && right_F) glowNbuzz(LED, 11);
    if (LED <= int(12 * NUM_LEDS / 16) && LED > int(11 * NUM_LEDS / 16) && backward_F && right_h) glowNbuzz(LED, 12);
    if (LED <= int(13 * NUM_LEDS / 16) && LED > int(12 * NUM_LEDS / 16) && backward_F && yNull)   glowNbuzz(LED, 13);
    if (LED <= int(14 * NUM_LEDS / 16) && LED > int(13 * NUM_LEDS / 16) && backward_F && left_h)  glowNbuzz(LED, 14);
    if (LED <= int(15 * NUM_LEDS / 16) && LED > int(14 * NUM_LEDS / 16) && backward_F && left_F)  glowNbuzz(LED, 15);
    if (                                  LED > int(15 * NUM_LEDS / 16) && backward_h && left_F)  glowNbuzz(LED, 16);
  }
  FastLED.show(); // Renders the LED's lights
}

void glowNbuzz(int LEDiteration, int buzzIteration) {
  pitch = pow(2, buzzIteration) * 220;
  tone(BUZZ_PIN, pitch);
  leds[LEDiteration].setHSV(int(random(255)), 200, 255);

}
// -- END OF FILE --
