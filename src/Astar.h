#ifndef Astar_h
#define Astar_h

#include "Arduino.h"

struct Astar {
    Point *start, *goal;
    PointStack itinerary;

    /**
     *  @function Astar()  : Creates a new A* pathfinder algorithm object
     */
    Astar();

    /**
     *  @function heuristic()  : Returns the heuritstic value between two points by calculating the distance between them
     *  @param    a            : A first pointer to a point object
     *  @param    b            : A second pointer to a point object
     *  @return   int
     */
    int heuristic(Point* a, Point* b);

    /**
     *  @function buildPath()  : Returns a stack of points corresponding to the route from the goal to the start
     *  @param    destination  : A pointer to a point object which corresponds to the last point of the route
     *  @return   PointStack
     */
    PointStack buildPath(Point* destination);

    /**
     *  @function find()  : Uses the A* algorithm to find the shortest path between the starting point and the goal
     *  You can then get the itinerary by using `getItinerary()`
     */
    void find();

    /**
     *  @function setStart()  : Sets the starting point of the path
     *  @param    _start      : A pointer to a Point object
     */
    void setStart(Point* _start);

    /**
     *  @function setGoal()  : Sets the stoping point of the path
     *  @param    _goal      : A pointer to a Point object
     */
    void setGoal(Point* _goal);

    /**
     *  @function getItinerary()  : Returns the itinerary corresponding to the last `find()` call
     *  @return   PointStack
     */
    PointStack getItinerary();
};

#endif
