/* -------------------------------------------------------------------------- *
   Program to run the RingRider platform.
   It is based on the libraries GY521, and FastLED.
 *                                                                            *
   It converts the data transmitted from GY521 modules that are attached to
   spring riders into a conterller like feed for interaction with LED rings.
   This code also runs a collection of games using the controller/LED circle.
 *                                                                            *
   Program written by Luiz F.S. Simão & Saskia Hustinx in the first half of 2022.
   -------------------------------------------------------------------------- */

#include "GY521.h"
#include "math.h"
#include <FastLED.h>
#include "Player.h"

// Define the general set of hardware ports and parameters
#define BUZZ_PIN        8
#define LED_PIN         5
#define NUM_LEDS        16
#define BRIGHTNESS      254
#define LIGHT_DECAY     2
#define LED_TYPE        WS2812
#define COLOR_ORDER     GRB
const float THRESHOLD = 0.15;
Player p1(0x69, THRESHOLD, -0.020, 0.007, -1.001, 3.711, 3.316, 1.718);
Player p2(0x68, THRESHOLD, -0.064, 0.002, -0.843, 0.983, 0.694, -0.645);
CRGB leds[NUM_LEDS]; // The accessable LED array.
CRGB p1color(100,0,50);
CRGB p2color(0,50,100);

bool firstEdit[NUM_LEDS];
 
// define states
enum play_mode{ISIDLE, EXPLORE, REACTION, COMBO}; 
play_mode current_mode = REACTION;

long prev_milli = 0;        // last time LED was updated
long idle_milli = 0;        // timer for idle state
long delay_interval = 2000;   
long idle_interval = 30000;    // 30 sec idle time
int rand_led = 0;

/*
 * todo saskia
 * -check: player position doesnt reset to 0 sometimes (maybe also wiring)
 * -debug reaction game
 * - fix delay in reaction game
 * - implement combo game
 */

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  
  p1.wakeUp_n_check();
  p2.wakeUp_n_check();

  ledController_startup();

  Serial.println("start...");
  idle_milli = millis();
  flash_ring((0,0,0));
}

void loop() {
  
  delay(5); // todo remove?
  check_idle(millis(), idle_interval);
  
  //  according to mode
  switch(current_mode) {
    
    case ISIDLE:
      /*
       * idle mode: 
       * randomly lights up segments to generate interest
       */
      generate_random(millis(), delay_interval, CRGB(100, 0, 100));
      break;
      
    case EXPLORE:
      /*
       * explore/default mode: 
       * each players movement triggers segments to light up & generate sound
       * if hit/overlap, play both sounds at higher volume
       */
      ledController_Playerinput(p1.currentPos(), p1color);
      ledController_Playerinput(p2.currentPos(), p2color);
      
      // HIT/MATCH
      if (p1.currentPos() != 0 && (p1.currentPos() == p2.currentPos())){
        Serial.println("HIT AT");
        Serial.println(p1.currentPos());
        Serial.println(p2.currentPos());
        play_sounds(100); //todo
      }
      else play_sounds(50);
      
      break;
      
    case REACTION:
      /*
       * reaction mode: 
       * a randomly selected zone lights up
       * first player to hit -> ring flashes green + success sound
       * slower player/miss -> ring flashes red + failure sound
       */

       // todo: differentiate btw rings (loser/winner)

       ledController_Playerinput(p1.currentPos(), p1color);
       ledController_Playerinput(p2.currentPos(), p2color);
        
       // HIT/MATCH
      if (p1.currentPos() != 0 && p1.currentPos() == rand_led){
        flash_ring(p1color);
        play_sounds(100); //todo
      }
      else if (p2.currentPos() != 0 && p2.currentPos() == rand_led){
        flash_ring(p2color);
        play_sounds(100); //todo
      }
      
      generate_random(millis(), delay_interval, CRGB(255, 0, 0));
      break;
      
    case COMBO:
      /*
       * combo mode: 
       * step by step, each player adds a move to a combo (either successfully or wrong (ring flashes red)
       * 
       * add array of prev moves to each player, update 
       */
      flash_ring(CRGB(255,255,255));
      break;
  }
  ledController_update();
}

void play_sounds(byte volume){
  //todo
  return;
}


// set to idle mode after inactivity
void check_idle(long current_milli, long delay_int){

  if ((p1.currentPos() != 0 || p2.currentPos() != 0) && current_mode == ISIDLE){
    flash_ring((0,0,0));
    idle_milli = current_milli;
    current_mode = EXPLORE; //if movement, reset to base state
    Serial.println("SET EXPLORE");
    Serial.println(p1.currentPos());
    Serial.println(p2.currentPos());
  }
  if (current_mode != ISIDLE && (current_milli - idle_milli > delay_int)){
    flash_ring((0,0,0));
    current_mode = ISIDLE; // set idle 
    Serial.println("SET IDLE");
    Serial.println(current_mode);
  }  
}

// random/idle mode
void generate_random(long current_milli, long delay_int, CRGB color){
  
  if (current_milli - prev_milli > delay_int){
    prev_milli = current_milli;
    rand_led = random(8)+1;
  }  
  ledController_Playerinput(rand_led, color);
}

//fash ring in solid colour
void flash_ring(CRGB f_color){
  for (int LED = 0; LED < NUM_LEDS; LED++) {
    ledController_setcolor(LED, f_color);
  }
}

void ledController_startup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  for (int i = 0; i < NUM_LEDS; i++) firstEdit[i] = true;
}

void ledController_Playerinput(int PL_Pos, CRGB PL_color) {
  // Matches the active states with a quadrant of a circle (the LED circle)
  for (int LED = 0; LED < NUM_LEDS; LED++) {
    if (LED < int(1 * NUM_LEDS / 8)                                    && PL_Pos == 1) ledController_setcolor(LED, PL_color);
    if (LED < int(2 * NUM_LEDS / 8) && LED >= int(1 * NUM_LEDS / 8) && PL_Pos == 2) ledController_setcolor(LED, PL_color);  
    if (LED < int(3 * NUM_LEDS / 8) && LED >= int(2 * NUM_LEDS / 8) && PL_Pos == 3) ledController_setcolor(LED, PL_color);
    if (LED < int(4 * NUM_LEDS / 8) && LED >= int(3 * NUM_LEDS / 8) && PL_Pos == 4) ledController_setcolor(LED, PL_color);
    if (LED < int(5 * NUM_LEDS / 8) && LED >= int(4 * NUM_LEDS / 8) && PL_Pos == 5) ledController_setcolor(LED, PL_color);
    if (LED < int(6 * NUM_LEDS / 8) && LED >= int(5 * NUM_LEDS / 8) && PL_Pos == 6) ledController_setcolor(LED, PL_color);
    if (LED < int(7 * NUM_LEDS / 8) && LED >= int(6 * NUM_LEDS / 8) && PL_Pos == 7) ledController_setcolor(LED, PL_color);
    if (                                LED >= int(7 * NUM_LEDS / 8) && PL_Pos == 8) ledController_setcolor(LED, PL_color);
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
