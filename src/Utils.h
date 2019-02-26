#ifndef Utils_h
#define Utils_h

#include "Arduino.h"

const int RED = -1;
const int BLUE = -2;
const int GREEN = -3;
const int BLACK = -4;
const int UNDEFINED = -5;

const int speedOfMvt = 80;
const int speedAtRotation = 150;

void print(String t);

void print(int n);

String idToName(int id);

int getRouteColor(int a, int b);

#endif
