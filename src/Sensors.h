#ifndef Sensors_h
#define Sensors_h

#include "Arduino.h"

class Sensors {
public:
  int colorToFollow, colorForward, colorRight, colorLeft;
  SDL_Arduino_TCA9545 *multiplexer;

  Sensors();
  Sensors(const SDL_Arduino_TCA9545 *multiplexer);

  // updates all color sensors' status
  void update ();

  // get the color detected by a color sensor
  int getColorV2 (int multiplexerOpt);

};

#endif
