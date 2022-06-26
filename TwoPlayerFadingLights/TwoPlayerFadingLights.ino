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
#include "Player.h"

// Define the general set of hardware ports and parameters
#define BUZZ_PIN        8
#define LED_PIN         5
#define NUM_LEDS        40
#define BRIGHTNESS      15
#define LIGHT_DECAY     2
#define LED_TYPE        WS2812
#define COLOR_ORDER     GRB
const float THRESHOLD = 0.25;
Player p1(0x69, THRESHOLD, -0.020, 0.007, -1.001, 3.711, 3.316, 1.718);
Player p2(0x68, THRESHOLD, -0.064, 0.002, -0.843, 0.983, 0.694, -0.645);
CRGB leds[NUM_LEDS]; // The accessable LED array.
CRGB p1color(100,0,50);
CRGB p2color(0,100,50);

bool firstEdit[NUM_LEDS];

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  p1.wakeUp_n_check();
  p2.wakeUp_n_check();

  ledController_startup();

  Serial.println("start...");
}

void loop() {
  ledController_Playerinput(p1.currentPos(), p1color);
  ledController_Playerinput(p2.currentPos(), p2color);
  ledController_update();
}

void ledController_startup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  for (int i = 0; i < NUM_LEDS; i++) firstEdit[i] = true;
}

void ledController_Playerinput(int PL_Pos, CRGB PL_color) {
  // Matches the active states with a quadrant of a circle (the LED circle)
  for (int LED = 0; LED < NUM_LEDS; LED++) {
    if (LED <= int(NUM_LEDS / 16)                                       && PL_Pos == 1)  ledController_setcolor(LED, PL_color);
    if (LED <= int(2  * NUM_LEDS / 16) && LED > int(NUM_LEDS / 16)      && PL_Pos == 2)  ledController_setcolor(LED, PL_color);
    if (LED <= int(3  * NUM_LEDS / 16) && LED > int(2  * NUM_LEDS / 16) && PL_Pos == 3)  ledController_setcolor(LED, PL_color);
    if (LED <= int(4  * NUM_LEDS / 16) && LED > int(3  * NUM_LEDS / 16) && PL_Pos == 4)  ledController_setcolor(LED, PL_color);
    if (LED <= int(5  * NUM_LEDS / 16) && LED > int(4  * NUM_LEDS / 16) && PL_Pos == 5)  ledController_setcolor(LED, PL_color);
    if (LED <= int(6  * NUM_LEDS / 16) && LED > int(5  * NUM_LEDS / 16) && PL_Pos == 6)  ledController_setcolor(LED, PL_color);
    if (LED <= int(7  * NUM_LEDS / 16) && LED > int(6  * NUM_LEDS / 16) && PL_Pos == 7)  ledController_setcolor(LED, PL_color);
    if (LED <= int(8  * NUM_LEDS / 16) && LED > int(7  * NUM_LEDS / 16) && PL_Pos == 8)  ledController_setcolor(LED, PL_color);
    if (LED <= int(9  * NUM_LEDS / 16) && LED > int(8  * NUM_LEDS / 16) && PL_Pos == 9)  ledController_setcolor(LED, PL_color);
    if (LED <= int(10 * NUM_LEDS / 16) && LED > int(9  * NUM_LEDS / 16) && PL_Pos == 10) ledController_setcolor(LED, PL_color);
    if (LED <= int(11 * NUM_LEDS / 16) && LED > int(10 * NUM_LEDS / 16) && PL_Pos == 11) ledController_setcolor(LED, PL_color);
    if (LED <= int(12 * NUM_LEDS / 16) && LED > int(11 * NUM_LEDS / 16) && PL_Pos == 12) ledController_setcolor(LED, PL_color);
    if (LED <= int(13 * NUM_LEDS / 16) && LED > int(12 * NUM_LEDS / 16) && PL_Pos == 13) ledController_setcolor(LED, PL_color);
    if (LED <= int(14 * NUM_LEDS / 16) && LED > int(13 * NUM_LEDS / 16) && PL_Pos == 14) ledController_setcolor(LED, PL_color);
    if (LED <= int(15 * NUM_LEDS / 16) && LED > int(14 * NUM_LEDS / 16) && PL_Pos == 15) ledController_setcolor(LED, PL_color);
    if (                                  LED > int(15 * NUM_LEDS / 16) && PL_Pos == 16) ledController_setcolor(LED, PL_color);
  }
}

void ledController_setcolor(int iteration, CRGB PL_color) {
  if (firstEdit[iteration]) {
    leds[iteration].setRGB(PL_color.r, PL_color.g, PL_color.b);
    firstEdit[iteration] = false;
  }
  else {
    float inv_OGr = 1.0 - (float(leds[iteration].r) / 255.0);
    float inv_OGg = 1.0 - (float(leds[iteration].g) / 255.0);
    float inv_OGb = 1.0 - (float(leds[iteration].b) / 255.0);

    float inv_newr = 1.0 - (float(PL_color.r) / 255.0);
    float inv_newg = 1.0 - (float(PL_color.g) / 255.0);
    float inv_newb = 1.0 - (float(PL_color.b) / 255.0);

    float convr = 1.0 - (inv_newr * inv_OGr);
    float convg = 1.0 - (inv_newg * inv_OGg);
    float convb = 1.0 - (inv_newb * inv_OGb);

    leds[iteration].setRGB(int(convr * 255), int(convg * 255), int(convb * 255));
  }
}

void ledController_update() {
  FastLED.show(); // Renders the LED's lights
  for (int LED = 0; LED < NUM_LEDS; LED++) leds[LED].subtractFromRGB(LIGHT_DECAY); // Turns off all LEDs
  for (int i = 0; i < NUM_LEDS; i++) firstEdit[i] = true;
}
