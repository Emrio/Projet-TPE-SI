#include "Arduino.h"
#include "Utils.h"
#include "Point.h"
#define oo 2e4

#include "Astar.h"

/************************************/
/************** A STAR **************/
/************************************/

Astar::Astar() {
}

void Astar::setStart(Point* _start) {
    start = _start;
}

void Astar::setGoal(Point* _goal) {
    goal = _goal;
}

PointStack Astar::getItinerary() {
    return itinerary;
}

int Astar::heuristic(Point* a, Point* b) {
    return sqrt(sq(a->x-b->x)+sq(a->y-b->y));
}

PointStack Astar::buildPath(Point* destination) {
    PointStack route;
    Point* current = destination;
    while (current != 0) {
        route.push(current);
        current = current->from;
    }
    return route;
}

void Astar::find() {

    PointStack closedSet, openSet;
    openSet.push(start);

    start->setG(0);
    start->setF(heuristic(start, goal));

    while(!openSet.empty()) {
        Point* current = openSet.lowestFscore();

        if(current == goal) {
            itinerary = buildPath(current);
            return;
        }

        openSet.remove(current);
        closedSet.push(current);

        for (int i = 0; i < current->neighbors.length(); i++) {
            Point* neighbor = current->neighbors.at(i);

            if (closedSet.has(neighbor)) continue;

            int tentativeG = current->getG() + heuristic(current, neighbor);

            if (!openSet.has(neighbor)) openSet.push(neighbor);
            else if (tentativeG >= neighbor->getG()) continue;

            neighbor->setFrom(current);
            neighbor->setG(tentativeG);
            neighbor->setF(neighbor->getG() + heuristic(neighbor, goal));
        }
    }
}

