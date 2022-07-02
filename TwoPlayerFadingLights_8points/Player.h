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
  bool forward_F, forward_h, backward_F, backward_h, left_F, left_h, right_F, right_h, xNull, yNull, isCPU, is_match;
  float THRESHOLD;
  int prev_pos, current_pos, combo_length;
  void check_combo(int c_pos, int p_pos, long c_millis, long match_duration);
  long match_time;

public:
  Player(int address, float th, float ax, float ay, float az, float gx, float gy, float gz, int combo_len);
  bool is_combo_mode;
  void wakeUp_n_check();
  int currentPos(), combo[];
};

#endif
