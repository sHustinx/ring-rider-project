#include "Player.h"

Player::Player(int address, float th, float ax, float ay, float az, float gx, float gy, float gz, byte combo_len)
{
  if (address != 0)
  {
    GY521 placeholder(address);
    THRESHOLD = th;
    sensor = placeholder;
    sensor.axe = ax;
    sensor.aye = ay;
    sensor.aze = az;
    sensor.gxe = gx;
    sensor.gye = gy;
    sensor.gze = gz;

    isCPU = false;
    combo_length = combo_len;
  }
  else
  {
    isCPU = true;
  }
}

void Player::wakeUp_n_check()
{
  while (sensor.wakeup() == false) // establishes connection to the module
  {
    //Serial.print(millis());
    //Serial.println("\tCould not connect to sensor");
    delay(1000);
  }
  sensor.setAccelSensitivity(2); // 16g
  sensor.setGyroSensitivity(1);  // 500 degrees/s
  sensor.setThrottle();
  prev_pos = -1;
  current_pos = 0;

  combo[combo_length] = {};
  is_combo_mode = false;
  is_match = false;
  //Serial.println("check");
}


// function to check if a section has been hit longer than timespan (to avoid mult. triggers)
void Player::check_combo(byte c_pos, byte p_pos, long c_millis, long match_duration){

  // reset counter, if position changes
  if (c_pos != p_pos){ 
    match_time = c_millis;
    is_match = false;
  }
  // start counter if position stays identical
  if (c_pos != 0 && c_pos == p_pos && is_match == false){ 
    match_time = millis();
    is_match = true;
   } 
  if (is_match && c_millis - match_time > match_duration){ 
    // add move to players combo-array if held for set duration
    for (int i=0; i<combo_length; i++){
      if (Player::combo[i] == 0){
        Player::combo[i] = c_pos;
        break;
      }
    }
    match_time = c_millis;
    is_match = false;
  }
}


byte Player::currentPos()
{
  // Reset states
  forward_F = false;
  forward_h = false;
  backward_F = false;
  backward_h = false;
  left_F = false;
  left_h = false;
  right_F = false;
  right_h = false;
  xNull = false;
  yNull = false;
  prev_pos = current_pos;
  current_pos = 0;
  

  // Obtain sensor data and convert the input into some reasonable range
  sensor.read();
  pitchR = (sensor.getPitch() * M_PI) / 180.0;
  rollR = (sensor.getRoll() * M_PI) / 180.0;
  planeY = sin(pitchR);
  planeX = -sin(rollR);

  // Defines currently active state
  if      (planeY < -THRESHOLD)       left_F     = true;
  //else if (planeY < -THRESHOLD / 2)   left_h     = true;
  if      (planeY > THRESHOLD)        right_F    = true;
 // else if (planeY > THRESHOLD  / 2)   right_h    = true;
  if      (planeX < -THRESHOLD)       forward_F  = true;
  //else if (planeX < -THRESHOLD / 2)   forward_h  = true;
  if      (planeX > THRESHOLD)        backward_F = true;
 // else if (planeX > THRESHOLD  / 2)   backward_h = true;
  //if (!forward_h && !backward_h && !forward_F && !backward_F)    xNull        = true;
 // if (!left_h    && !right_h && !left_F    && !right_F)       yNull        = true;

      if (!forward_F && !backward_F)    xNull        = true;
      if (!left_F    && !right_F)       yNull        = true;

  

  if (xNull      && left_F)  current_pos = 1;
  if (forward_F  && left_F)  current_pos = 2;
  if (forward_F  && yNull)   current_pos = 3;
  if (forward_F  && right_F) current_pos = 4;
  if (xNull      && right_F) current_pos = 5;
  if (backward_F && right_F) current_pos = 6;
  if (backward_F && yNull)   current_pos = 7; 
  if (backward_F && left_F)  current_pos = 8;

  // if player is in combo-mode check for move-updates 
  if (is_combo_mode) check_combo(current_pos, prev_pos, millis(), 1500); 
  
  return current_pos;
}
