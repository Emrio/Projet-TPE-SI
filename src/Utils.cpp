#include "Arduino.h"
#include "Point.h"

#include "Utils.h"

String preceding_text = "";

void print(String t) {
  if(t != preceding_text) {
    Serial.println(t);
    preceding_text = t;
  }
}

void print(int n) {
  String t = String(n);
  if(t != preceding_text) {
    Serial.println(t);
    preceding_text = t;
  }
}

String idToName(int id) {
  switch(id) {
    case 0:
      return "Salon";
    case 1:
      return "Cuisine";
    case 2:
      return "Salle de bain";
    case 3:
      return "Chambre";
    case 4:
      return "PtI1";
    case 5:
      return "PtI2";
    default:
      return "Inconnue";
  }
}

int getRouteColor(int a, int b) {
  if (a == 0 && b == 4 || a == 4 && b == 0) return RED;
  if (a == 1 && b == 4 || a == 4 && b == 1) return BLUE;
  if (a == 5 && b == 4 || a == 4 && b == 5) return GREEN;
  if (a == 5 && b == 2 || a == 2 && b == 5) return BLUE;
  if (a == 5 && b == 3 || a == 3 && b == 5) return RED;
  return 0;
}

int ABS(int x) {
    return x > 0 ? x : -x;
}

int link(Point* a, Point* b) {
    a->addNeighbor(b);
    b->addNeighbor(a);
}
