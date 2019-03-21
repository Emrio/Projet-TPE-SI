#ifndef Point_h
#define Point_h
#define MAX_NODE 8

#include "Arduino.h"

struct Point;

/**
 *  PointStack : Vector of Point objects for Arduino
 *  Used for neighbors, openSet and closedSet
 */
struct PointStack {
    Point *points[MAX_NODE];
    int cursor;

    /**
     *  @function PointStack()  : Creates a new stack of points
     */
    PointStack();

    /**
     *  @function length()  : Returns the length of the stack
     *  @return   int
     */
    int length();

    /**
     *  @function empty()  : Tests if the stack is empty or not
     *  @return   boolean
     */
    bool empty();

    /**
     *  @function push()  : Adds an element to the stack at the top of the stack
     *  @param    p       : A pointer to a Point object
     */
    void push(Point *p);

    /**
     *  @function pop()  : Removes the upper-most element from the stack
     */
    void pop();

    /**
     *  @function remove()  : Removes an element from the stack
     *  @param    p         : A pointer to a Point object
     */
    void remove(Point* p);

    /**
     *  @function at()  : Returns the pointer in a given position in the stack
     *  @param    index : The index of the pointer in the stack
     *  @return   *Point
     */
    Point* at(int index);

    /**
     *  @function has()  : Tests if a given Point is in the stack
     *  @param    p      : The pointer of the Point
     *  @return   boolean
     */
    bool has(Point* p);

    /**
     *  @function lowestFscore()  : Returns the pointer with the lowest F score in the stack
     *  @return   *Point
     */
    Point* lowestFscore();
};

struct Point {
    int id, g, f, x, y;
    String name;
    PointStack neighbors;
    Point* from;

    /**
     *  @function Point()  : Creates a new point
     *  @param    _id      : A unique ID for the point
     *  @param    _name    : The name of the point
     *  @param    _x       : The x coordinate of the point
     *  @param    _y       : The y coordinate of the point
     */
    Point(int _id, String _name, int _x, int _y);

    /**
     *  @function addNeighbor()  : Adds a neighbor to the neighborhood of the Point
     *  @param    b              : A pointer to a Point object
     */
    void addNeighbor(Point *b);

    /**
     *  @function setF()  : Sets the f score
     *  @param    _f      : The new score
     */
    void setF(int _f);

    /**
     *  @function setG()  : Sets the g score
     *  @param    _g      : The new score
     */
    void setG(int _g);

    /**
     *  @function setFrom()  : Sets the Point this point is from
     *  @param    p          : A pointer to a Point object
     */
    void setFrom(Point* p);

    /**
     *  @function getG()  : Returns the current g score
     *  @return   int
     */
    int getG();
};

#endif
