#ifndef Motors_h
#define Motors_h

#include "Arduino.h"

class Motors {
public:
  int pwmA, brakeA, dirA, pwmB, brakeB, dirB, speed;

  Motors();

  void setSpeed(int _speed);
  void setSpeed();

  void forward();
  void backward();
  void left();
  void right();
  void stop();
};

#endif
