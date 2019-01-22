#define MAX_NODE 8
#define oo 2e4

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

/**
 *  @function link()  : Add each Point to the neighborhood of the other
 *  @param    a       : A pointer to a first Point object
 *  @param    b       : A pointer to a second Point object
 */
void link(Point* a, Point *b);

/**
 *  @function ABS()  : Absolute value of a number
 *  @param    x      : A number
 *  @return   int
 */
int ABS(int x);

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

/**********************************************/
/************** GLOBAL FUNCTIONS **************/
/**********************************************/

int ABS(int x) {
    return x > 0 ? x : -x;
}

void link(Point* a, Point* b) {
    a->addNeighbor(b);
    b->addNeighbor(a);
}

/***********************************************/
/************* MAIN PROGRAM BLOCK **************/
/***********************************************/

void setup() {

    Serial.begin(9600);

    // Summoning the A* pathfinder
    Astar astar = Astar();

    // Creating the points
    Point a = Point(1, "P1", 0, 0);
    Point b = Point(2, "P2", 1, 0);
    Point c = Point(3, "P3", 2, 0);
    Point d = Point(4, "P4", 1, 1);

    // Linking them
    link(&a, &b);
    link(&b, &c);
    link(&b, &d);

    /*
              P1 -- P2 -- P3
                     |
                    P4
    */

    // Setting starting and goal points
    astar.setStart(&a);
    astar.setGoal(&d);

    // FIND !!!
    astar.find();

    // Show the itinerary
    for (int i = astar.getItinerary().length()-1; i >= 0; i--) {
        Serial.println(astar.getItinerary().at(i)->name);
    }

}

void loop() {}
