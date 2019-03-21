#include "Arduino.h"
#include "Utils.h"
#include "Point.h"
#include "Astar.h"
#include "GPS.h"

GPS::GPS(): curIndex(-1), departureId(-1), destinationId(-1), length(0) {}

String GPS::current() {
  return length == 0 ? "" : idToName(route[curIndex]);
}

String GPS::next() {
  return curIndex >= 4 ? "" : idToName(route[curIndex + 1]);
}

String GPS::departure() {
  return length == 0 ? "" : idToName(departureId);
}

String GPS::destination() {
  return length == 0 ? "" : idToName(destinationId);
}

int GPS::lengthOfRoute() {
  return length;
}

int GPS::percentCompleted() {
  return 100 * ((float)curIndex / length);
}

int GPS::curRouteIndex() {
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

  Astar astar = Astar();

  // TODO
  Point a = Point(1, "Salon", 0, 0);
  Point b = Point(2, "Cuisine", 1, 1);
  Point c = Point(3, "Salle de bain", 2, 1);
  Point d = Point(4, "Chambre", 2, -1);
  Point e = Point(5, "PtI1", 1, 0);
  Point f = Point(6, "PtI2", 2, 0);

  link(&a, &e);
  link(&b, &e);
  link(&f, &e);
  link(&f, &c);
  link(&f, &d);

  if (departureId == 1) {
    astar.setStart(&a);
  }

  switch(departureId) {
    case 1:
      astar.setStart(&a);
      break;
    case 2:
      astar.setStart(&b);
      break;
    case 3:
      astar.setStart(&c);
      break;
    case 4:
      astar.setStart(&d);
      break;
  }

  switch(destinationId) {
    case 1:
      astar.setGoal(&a);
      break;
    case 2:
      astar.setGoal(&b);
      break;
    case 3:
      astar.setGoal(&c);
      break;
    case 4:
      astar.setGoal(&d);
      break;
  }
  
  astar.find();

  length = astar.getItinerary().length();
  for (int i = length - 1; i >= 0; i--) {
    route[i] = astar.getItinerary().at(i);
  }

  curIndex = 0;

  print("Route print");
  for (int i = 0; i < lengthOfRoute(); i++) {
    print(route[i]);
  }
  print("End Route print");
}

void GPS::nextStep() {
  curIndex++;
  if (curIndex == length - 1) {
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
