#ifndef Utils_h
#define Utils_h
#include "Point.h"


#include "Arduino.h"

const int RED = -1;
const int BLUE = -2;
const int GREEN = -3;
const int BLACK = -4;
const int UNDEFINED = -5;

const int speedOfMvt = 80;
const int speedAtRotation = 100;

void print(String t);

void print(int n);

String idToName(int id);

int getRouteColor(int a, int b);

/**
 *  @function link()  : Add each Point to the neighborhood of the other
 *  @param    a       : A pointer to a first Point object
 *  @param    b       : A pointer to a second Point object
 */
int link(Point* a, Point *b);

/**
 *  @function ABS()  : Absolute value of a number
 *  @param    x      : A number
 *  @return   int
 */
int ABS(int x);

#endif
