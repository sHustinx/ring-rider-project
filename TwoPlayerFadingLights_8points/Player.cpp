#include "Player.h"

Player::Player(int address, float th, float ax, float ay, float az, float gx, float gy, float gz)
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
    Serial.print(millis());
    Serial.println("\tCould not connect to sensor");
    delay(1000);
  }
  sensor.setAccelSensitivity(2); // 16g
  sensor.setGyroSensitivity(1);  // 500 degrees/s
  sensor.setThrottle();
  Serial.println("check");
}

int Player::currentPos()
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



  if (xNull      && left_F)  return 1;
  if (forward_F  && left_F)  return 2;
  if (forward_F  && yNull)   return 3;
  if (forward_F  && right_F) return 4;
  if (xNull      && right_F) return 5;
  if (backward_F && right_F) return 6;
  if (backward_F && yNull)   return 7; 
  if (backward_F && left_F)  return 8;
  return 0;

}
