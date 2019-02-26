#include "Arduino.h"
#include "Utils.h"
#include "GPS.h"

GPS::GPS(): curIndex(-1), departureId(-1), destinationId(-1), length(0) {}

String GPS::current() {
  if (length == 0) {
    return "";
  }
  return idToName(route[curIndex]);
}

String GPS::next() {
  if (curIndex >= 4) {
    return "";
  }
  return idToName(route[curIndex + 1]);
}

String GPS::departure() {
  if (length == 0) {
    return "";
  }
  return idToName(departureId);
}

String GPS::destination() {
  if (length == 0) {
    return "";
  }
  return idToName(destinationId);
}

int GPS::lengthOfRoute() {
  return length;
}

int GPS::percentCompleted() {
  return 100 * ((float)curIndex / length);
}

int GPS::curRouteIndex() {
  print("cur");
  print(curIndex);
  return curIndex;
}

int GPS::currentId() {
  if (curIndex < 0) {
    return -1;
  }
  return route[curIndex];
}

int GPS::nextId() {
  return route[curIndex + 1]; // works because -1 + 1 = 0 ---> -1 if no route currently
}

void GPS::setDeparture(int id) {
  departureId = id;
}

void GPS::setDestination(int id) {
  destinationId = id;
}

void GPS::calculate() {
  if (departureId == -1 || destinationId == -1) return;
  if (departureId == destinationId) return;

  if (departureId == 0 && destinationId == 1) {
    length = 3;
    route[0] = 0;
    route[1] = 4;
    route[2] = 1;
  } else if (departureId == 0 && destinationId == 2) {
    length = 4;
    route[0] = 0;
    route[1] = 4;
    route[2] = 5;
    route[3] = 2;
  } else if (departureId == 0 && destinationId == 3) {
    length = 4;
    route[0] = 0;
    route[1] = 4;
    route[2] = 5;
    route[3] = 3;
  } else if (departureId == 1 && destinationId == 0) {
    length = 3;
    route[0] = 1;
    route[1] = 4;
    route[2] = 0;
  } else if (departureId == 1 && destinationId == 2) {
    length = 4;
    route[0] = 1;
    route[1] = 4;
    route[2] = 5;
    route[3] = 2;
  } else if (departureId == 1 && destinationId == 3) {
    length = 4;
    route[0] = 1;
    route[1] = 4;
    route[2] = 5;
    route[3] = 3;
  } else if (departureId == 2 && destinationId == 0) {
    length = 4;
    route[0] = 2;
    route[1] = 5;
    route[2] = 4;
    route[3] = 0;
  } else if (departureId == 2 && destinationId == 1) {
    length = 4;
    route[0] = 2;
    route[1] = 5;
    route[2] = 4;
    route[3] = 1;
  } else if (departureId == 2 && destinationId == 3) {
    length = 3;
    route[0] = 2;
    route[1] = 5;
    route[2] = 3;
  } else if (departureId == 3 && destinationId == 0) {
    length = 4;
    route[0] = 3;
    route[1] = 5;
    route[2] = 4;
    route[3] = 0;
  } else if (departureId == 3 && destinationId == 1) {
    length = 4;
    route[0] = 3;
    route[1] = 5;
    route[2] = 4;
    route[3] = 1;
  } else if (departureId == 3 && destinationId == 2) {
    length = 4;
    route[0] = 3;
    route[1] = 5;
    route[2] = 2;
  }
  curIndex = 0;
}

void GPS::nextStep() {
  curIndex++;
  if (curIndex == length) {
    length = 0;
    curIndex = -1;
    route[0] = -1;
    route[1] = -1;
    route[2] = -1;
    route[3] = -1;
  }
}

bool GPS::finished() {
  return length == 0;
}
