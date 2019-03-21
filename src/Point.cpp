#include "Arduino.h"
#define MAX_NODE 8
#define oo 2e4

#include "Point.h"

/****************************************/
/************** POINTSTACK **************/
/****************************************/

PointStack::PointStack() {
    for(int i = 0; i < MAX_NODE; i++) points[i] = 0;
    cursor = 0;
}

int PointStack::length() {
    return cursor;
}

bool PointStack::empty() {
    return cursor == 0;
}

void PointStack::push(Point *p) {
    points[cursor++] = p;
}

void PointStack::pop() {
    points[cursor--] = 0;
}

void PointStack::remove(Point* p) {
    for(int i = 0; i < cursor; i++) if(points[i] == p) {
        for(int j = i+1; j < cursor; j++) points[j-1] = points[j];
        cursor--;
        return;
    }
}

Point* PointStack::at(int index) {
    return index <= cursor ? points[index] : 0;
}

bool PointStack::has(Point* p) {
    for (int i = 0; i < length(); i++) if (points[i] == p) return true;
    return false;
}

Point* PointStack::lowestFscore() {
    Point *withLowestF = points[0];
    for(int i = 0; i < cursor; i++) {
        withLowestF = &points[i]->f > &withLowestF->f ? points[i] : withLowestF;
    }
    return withLowestF;
}

/***********************************/
/************** POINT **************/
/***********************************/

Point::Point(int _id, String _name, int _x, int _y): id(_id), name(_name), x(_x), y(_y) {
    neighbors = PointStack();
    from = 0;
    g = oo;
    f = oo;
}
void Point::addNeighbor(Point *b) {
    neighbors.push(b);
}

void Point::setF(int _f) {
    f = _f;
}

void Point::setG(int _g) {
    g = _g;
}

int Point::getG() {
    return g;
}

void Point::setFrom(Point* p) {
    from = p;
}
