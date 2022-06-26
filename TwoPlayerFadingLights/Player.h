// Player.h
#ifndef Player_h
#define Player_h

#include <Arduino.h>
#include "GY521.h"

class Player
{
private:
  GY521 sensor;
  double pitchR, rollR, planeX, planeY, projectAng;
  bool forward_F, forward_h, backward_F, backward_h, left_F, left_h, right_F, right_h, xNull, yNull, isCPU;
  float THRESHOLD;

public:
  Player(int address, float th, float ax, float ay, float az, float gx, float gy, float gz);
  void wakeUp_n_check();
  int currentPos();
};

#endif
