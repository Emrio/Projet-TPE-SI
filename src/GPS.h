#ifndef GPS_h
#define GPS_h

#include "Arduino.h"

class GPS {

public:
  GPS();

  String current();
  String next();
  String departure();
  String destination();

  int lengthOfRoute();
  int percentCompleted();
  int curRouteIndex();
  int currentId();
  int nextId();

  void setDeparture(int id);
  void setDestination(int id);

  void calculate();
  void nextStep();

  bool finished();

private:
  short int curIndex; // The point we are from in each segment
  short int departureId;
  short int destinationId;
  unsigned short int length;
  short int route[4];

};

#endif
