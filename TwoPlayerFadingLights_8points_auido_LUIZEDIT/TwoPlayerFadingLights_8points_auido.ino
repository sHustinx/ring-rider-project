/* -------------------------------------------------------------------------- *
   Program to run the RingRider platform.
   It is based on the libraries GY521, and FastLED.
 *                                                                            *
   It converts the data transmitted from GY521 modules that are attached to
   spring riders into a conterller like feed for interaction with LED rings.
   This code also runs a collection of games (EXPLORE; REACTION; COMBINATION) using the controller/LED circle.
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
#define FILE_COUNT      8
const float THRESHOLD = 0.40;

const byte combo_len = 4;
Player p1(0x69, THRESHOLD, -0.046, 0.007, -0.846, 1.229, 0.622, -0.667, combo_len);
Player p2(0x68, THRESHOLD, -0.043, 0.006, -0.843, 3.640, 3.286, 1.729, combo_len);
CRGB leds[NUM_LEDS]; // The accessable LED array.
CRGB p1color(100, 0, 50);
CRGB p2color(0, 50, 100);

bool firstEdit[NUM_LEDS];
bool flag = false;

// define states
enum play_mode {ISIDLE, EXPLORE, REACTION, COMBO};
play_mode current_mode;

long prev_milli = 0;        // last time LED was updated
long idle_milli = 0;        // timer for idle state
const uint16_t delay_interval = 2000;
const uint16_t idle_interval = 40000;    // 40 sec idle time
byte rand_led = 0;
byte combo_game_turn = 0;
bool move_displayed = false;

byte S_COMBO_LEN = 4;
byte s_combo[4] = {2, 2, 2, 2}; //secret bonus combo

// setter for gamemode, resets LED ring and game stats for combo-mode
void set_game_mode(play_mode mode) {

  if (mode == COMBO) { //reset combos
    for (int i = 0; i < combo_len; i++) {
      p1.combo[i] = 0;
      p2.combo[i] = 0;
    }
    combo_game_turn = 0;
  }
  else {
    p1.is_combo_mode = false;
    p2.is_combo_mode = false;
  }

  flash_ring(CRGB(0, 0, 0)); // reset ring/off
  current_mode = mode;
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(0x66);

  Wire.onReceive(receiveEvent); // register event
  Serial.println("on");
  p1.wakeUp_n_check();
  Serial.println("ready");

  p2.wakeUp_n_check();
  Serial.println("ready");


  ledController_startup();


  // serial.println("start...");
  // idle_milli = millis();
  flash_ring(CRGB(0, 0, 0));

  set_game_mode(EXPLORE);
  Serial.println("ready");

}

void loop() {

  //delay(5);

  check_idle(millis(), idle_interval);

  //  according to mode
  switch (current_mode) {

    case ISIDLE:
      /*
         idle mode:
         randomly lights up segments to generate interest
      */
      generate_random(millis(), delay_interval, CRGB(100, 0, 100));
      break;

    case EXPLORE:
      /*
         explore/default mode:
         each players movement triggers segments to light up & generate sound
         if hit/overlap, play both sounds at higher volume
      */
      ledController_Playerinput(p1.currentPos(), p1color);
      ledController_Playerinput(p2.currentPos(), p2color);

         Serial.println(p1.currentPos());
         Serial.println(p2.currentPos());

      // HIT/MATCH
      if (p1.currentPos() != 0 && (p1.currentPos() == p2.currentPos())) {
        // // serial.println("HIT AT");
        // serial.println(p1.currentPos());
        // serial.println(p2.currentPos());
        //play_sounds(100); //todo
      }
     // audioTrigger(10 * p1.currentPos() + p2.currentPos());

      break;

    case REACTION:
      /*
         reaction mode:
         a randomly selected zone lights up
         first player to hit -> ring flashes green + success sound
         slower player/miss -> ring flashes red + failure sound
      */

      ledController_Playerinput(p1.currentPos(), p1color);
      ledController_Playerinput(p2.currentPos(), p2color);

      // HIT/MATCH
      if (p1.currentPos() != 0 && p1.currentPos() == rand_led) {
        flash_ring(CRGB(0, 0, 0));
        flash_ring(p1color);
        audioTrigger(99);
      }
      else if (p2.currentPos() != 0 && p2.currentPos() == rand_led) {
        flash_ring(CRGB(0, 0, 0));
        flash_ring(p2color);
        audioTrigger(99);
      }

      generate_random(millis(), delay_interval, CRGB(255, 0, 0));
      break;

    case COMBO:
      /*
         combo mode:
         step by step, each player adds a move to a combo (either successfully or wrong (ring flashes red)

         add array of prev moves to each player, update
      */

      ledController_Playerinput(p1.currentPos(), p1color);
      ledController_Playerinput(p2.currentPos(), p2color);


      if (combo_game_turn % 2 == 0) play_combo(p1, p2, combo_game_turn);
      else play_combo(p2, p1, combo_game_turn);

      break;
  }
  ledController_update();
}

// fill combo-array with zeroes
void reset_combo(byte arr[], byte len) {
  for (int i = 0; i < len; i++) {
    arr[i] = 0;
  }
}

// display a bonus
void play_secret_bonus() {
  fill_rainbow( leds, NUM_LEDS, 0, 20);
  //fill_rainbow_circular(leds, NUM_LEDS,0,false);
  FastLED.show();
  delay(1000);
}

// combo game function
void play_combo(Player &player, Player &opponent, byte turn) {

  //restart game after end
  if (turn == combo_len) {
    // serial.println("reset fired");
    set_game_mode(COMBO);
    return;
  }

  // activate player mode
  player.is_combo_mode = true;
  opponent.is_combo_mode = false;

  // if player has hit a move, switch to opponent
  if (player.combo[turn] != 0) {
    opponent.is_combo_mode = true;
    player.is_combo_mode = false;

    //show previous moves that need to be repeated
    if (!move_displayed) {

      //check for secret combo
      if (turn == S_COMBO_LEN - 1 && compare_combo(s_combo, player.combo, combo_len)) play_secret_bonus();

      // display previous moves and reset own array
      display_combo(player.combo, combo_len);
      delay(1000);
      move_displayed = true;
      reset_combo(opponent.combo, combo_len);
    }

    //opponent has hit number of required moves
    if (opponent.combo[turn] != 0) {

      if (compare_combo(opponent.combo, player.combo, combo_len)) {
        // moves match, flash green, chnage player
        flash_ring(CRGB(0, 255, 0));
        delay(500);
        combo_game_turn++;
        move_displayed  = false;
        // serial.println("COMBO SUCCESS");
      }
      else {
        // moves dont match, flash red, retry
        flash_ring(CRGB(255, 0, 0));
        delay(500);
        move_displayed  = false;
        // serial.println("COMBO FAIL");
      }
      // testing output
      // serial.println(turn);
      print_combo(player.combo, combo_len);
      print_combo(opponent.combo, combo_len);
    }
  }
}

// display all moves that need to be repeated
void display_combo(byte arr[], byte len) {
  delay(500);
  for (int i = 0; i < len; i++) {
    if (arr[i] != 0) {
      flash_ring(CRGB(0, 0, 0));
      ledController_Playerinput(arr[i], CRGB(255, 255, 0));
      ledController_update();
      delay(1000);
      flash_ring(CRGB(0, 0, 0));
      delay(500);
    }
  }
}

// compare combo-array of two players
bool compare_combo(byte arr1[], byte arr2[], byte len) {
  for (int i = 0; i < len; i++) {
    if (arr1[i] != arr2[i]) return false;
  }
  return true;
}

// print combo array to // serial monitor
void print_combo(byte arr[], byte len) {
  for (int i = 0; i < len; ++i) {
    // serial.print(arr[i]);
  }
  // serial.println("");
}

// todo, play sounds when triggered
void play_sounds(byte volume) {
  //todo
  return;
}


// set to idle mode after set period of inactivity
void check_idle(long current_milli, long delay_int) {

  if ((p1.currentPos() != 0 || p2.currentPos() != 0) && current_mode == ISIDLE) {
    //if movement, reset to base state (EXPLORE)
    idle_milli = current_milli;
    set_game_mode(EXPLORE);
    /*// serial.println("SET EXPLORE");
      // serial.println(p1.currentPos());
      // serial.println(p2.currentPos());*/
  }
  if (current_mode != ISIDLE && (current_milli - idle_milli > delay_int)) {
    //if inactive, set (IDLE)
    set_game_mode(ISIDLE);
    /*// serial.println("SET IDLE");
      // serial.println(current_mode);*/
  }
}

// Random/Idle mode, lights up random sections of LED ring
void generate_random(long current_milli, long delay_int, CRGB color) {

  if (current_milli - prev_milli > delay_int) {
    flash_ring(CRGB(0, 0, 0));
    prev_milli = current_milli;
    rand_led = random(8) + 1;
  }
  ledController_Playerinput(rand_led, color);
}

// flash LED ring in solid colour
void flash_ring(CRGB f_color) {
  for (int LED = 0; LED < NUM_LEDS; LED++) {
    ledController_setcolor(LED, f_color);
  }
  ledController_update();
}

// init. of LED ring
void ledController_startup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  for (int i = 0; i < NUM_LEDS; i++) firstEdit[i] = true;
}

// set adressed sections/LEDs
void ledController_Playerinput(byte PL_Pos, CRGB PL_color) {
  // Matches the active states with a quadrant of a circle (the LED circle)
  for (byte LED = 0; LED < NUM_LEDS; LED++) {
    if (LED < byte(1 * NUM_LEDS / 8)                                    && PL_Pos == 1) ledController_setcolor(LED, PL_color);
    if (LED < byte(2 * NUM_LEDS / 8) && LED >= byte(1 * NUM_LEDS / 8) && PL_Pos == 2) ledController_setcolor(LED, PL_color);
    if (LED < byte(3 * NUM_LEDS / 8) && LED >= byte(2 * NUM_LEDS / 8) && PL_Pos == 3) ledController_setcolor(LED, PL_color);
    if (LED < byte(4 * NUM_LEDS / 8) && LED >= byte(3 * NUM_LEDS / 8) && PL_Pos == 4) ledController_setcolor(LED, PL_color);
    if (LED < byte(5 * NUM_LEDS / 8) && LED >= byte(4 * NUM_LEDS / 8) && PL_Pos == 5) ledController_setcolor(LED, PL_color);
    if (LED < byte(6 * NUM_LEDS / 8) && LED >= byte(5 * NUM_LEDS / 8) && PL_Pos == 6) ledController_setcolor(LED, PL_color);
    if (LED < byte(7 * NUM_LEDS / 8) && LED >= byte(6 * NUM_LEDS / 8) && PL_Pos == 7) ledController_setcolor(LED, PL_color);
    if (                                LED >= byte(7 * NUM_LEDS / 8) && PL_Pos == 8) ledController_setcolor(LED, PL_color);
  }
}

// LED color function/transitions
void ledController_setcolor(byte iteration, CRGB PL_color) {
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

// call to update LED ring
void ledController_update() {
  FastLED.show(); // Renders the LED's lights
  for (int LED = 0; LED < NUM_LEDS; LED++) leds[LED].subtractFromRGB(LIGHT_DECAY); // Turns off all LEDs
  for (int i = 0; i < NUM_LEDS; i++) firstEdit[i] = true;
}

void audioTrigger(int index) {
  Serial.println(index);
  if (index > 0) {
    Wire.beginTransmission(0x67);  // transmit to device #8
    Wire.write(index);             // sends one byte
    Wire.endTransmission();        // stop transmitting
    delay(350);
//    while (!flag);                 // waits for an echo
//    flag = false;
  }
}

void receiveEvent(int howMany) {
  flag = true;
  Serial.print('a');
}
