#include "Arduino.h"
#include "Utils.h"
#include <SDL_Arduino_TCA9545.h>
#include <Adafruit_TCS34725.h>

#include "Sensors.h"

Sensors::Sensors(): colorToFollow(UNDEFINED), colorForward(UNDEFINED), colorRight(UNDEFINED), colorLeft(UNDEFINED) {}

Sensors::Sensors(SDL_Arduino_TCA9545 *_multiplexer): colorToFollow(UNDEFINED), colorForward(UNDEFINED), colorRight(UNDEFINED), colorLeft(UNDEFINED), multiplexer(_multiplexer) {}

// updates all color sensors' status
void Sensors::update () {
  colorForward = getColorV2(TCA9545_CONFIG_BUS1);
  colorRight = getColorV2(TCA9545_CONFIG_BUS0);
  colorLeft = getColorV2(TCA9545_CONFIG_BUS2);
}

// get the color detected by a color sensor
int Sensors::getColorV2 (int multiplexerOpt) {
  // connecting to the right color sensor
  multiplexer->write_control_register(multiplexerOpt);

  Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

  // sensor not found
  if (!tcs.begin()) {
    Serial.println("Could not find Color Sensor with setup ");
    Serial.println(multiplexerOpt);
    return UNDEFINED;
  }

  uint16_t clear, red, green, blue;
  float pR, pG, pB, S;

  // getting the color
  tcs.setInterrupt(false);
  delay(60);
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);

  // extracting and parsing data
  S = clear;
  pR = red/S;
  pG = green /S;
  pB = blue/S;

  int blackSensitivity = 500;
  if (multiplexerOpt == TCA9545_CONFIG_BUS1) {
    blackSensitivity = 350;
  }

  if (pR >= 0.42) {
    return RED;
  } else if (pG >= 0.42) {
    return GREEN;
  } else if (pB >= 0.42) {
    return BLUE;
  } else if ((0.3 * red + 0.59 * green + 0.11 * blue) < blackSensitivity) {
    return BLACK;
  } else {
    return UNDEFINED;
  }
}
