#include <GroveColorSensor.h>
#include <Wire.h>
#include <SDL_Arduino_TCA9545.h>
#include <SoftwareSerial.h>
#include <SerialLCD.h>
#define len(x) (sizeof(x) / sizeof(x[0]))
#define MAX_NODE 6
#define oo 2e4
#define RED "RED"
#define BLUE "BLUE"
#define GREEN "GREEN"
#define BLACK "BLACK"

String preceding_text = "";
int idDistributor = -6;

SDL_Arduino_TCA9545 multiplexer;
SerialLCD slcd(4,5);

void print(String t);
void print(int n);
String idToName(int id);

struct Point;

struct PointStack {
    Point *points[MAX_NODE];
    int cursor;

    PointStack();

    int length();
    bool empty();
    void push(Point *p);
    void pop();
    void remove(Point* p);
    Point* at(int index);
    bool has(Point* p);
    void reset();
    Point* lowestFscore();
};

struct Point {
    int id, g, f, x, y;
    PointStack neighbors;
    Point* from;

    Point() {}
    Point(int _x, int _y);

    void addNeighbor(Point *b);
    void setF(int _f);
    void setG(int _g);
    void setFrom(Point* p);
    int getG();
};

struct Astar {
    Point *start, *goal;
    PointStack itinerary, closedSet, openSet;

    Astar();

    int heuristic(Point* a, Point* b);
    PointStack buildPath(Point* destination);
    void find();
    void setStart(Point* _start);
    void setGoal(Point* _goal);
    PointStack getItinerary();
};

// links two points together
void link(Point* a, Point *b);

// absolute value of an integer
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

void PointStack::reset() {
  for(int i = 0; i < length(); i++) points[i] = 0;
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

Point::Point(int _x, int _y): x(_x), y(_y) {
    neighbors = PointStack();
    from = 0;
    g = oo;
    f = oo;

    Serial.println(idDistributor);
    id = idDistributor++;
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

    if (!start || !goal) return;

    openSet.reset();
    closedSet.reset();

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

// absolute value of a integer
int ABS(int x) {
    return x > 0 ? x : -x;
}

// links two points together
void link(Point* a, Point* b) {
    a->addNeighbor(b);
    b->addNeighbor(a);
}

struct Sensors {

  String color_to_follow, color_forward, color_right, color_left;

  Sensors() {}

  // update color properties
  void update() {
    color_forward = get_color(TCA9545_CONFIG_BUS0, color_forward);
    color_right = get_color(TCA9545_CONFIG_BUS2, color_right);
    color_left = get_color(TCA9545_CONFIG_BUS3, color_left);
  }

  // get a color from a color sensor using its multiplexer id
  String get_color(int multiplexer_opt, String color_before) {
    multiplexer.write_control_register(multiplexer_opt);

    GroveColorSensor sensor;
    sensor.ledStatus = 1;

    int r, g, b;
    sensor.clearInterrupt();
    sensor.readRGB(&r, &g, &b);

    int sum = r + g + b;

    if((float)r/sum > .7) {
      return RED;
    }
    else if((float)g/sum > .7) {
      return GREEN;
    }
    else if((float)b/sum > .7) {
      return BLUE;
    }
    else if((float)sum < 100) {
      return BLACK;
    }
    return color_before;

  }

};

struct Motors {

  int I1 = 8,
      I2 = 11,
      I3 = 12,
      I4 = 13,
      A = 9,
      B = 10,
      speed = 128;

  Motors() {
    pinMode(I1,OUTPUT);
    pinMode(I2,OUTPUT);
    pinMode(A,OUTPUT);

    pinMode(I3,OUTPUT);
    pinMode(I4,OUTPUT);
    pinMode(B,OUTPUT);
  }

  void forward() {

    analogWrite(A,speed);
    digitalWrite(I2,LOW);
    digitalWrite(I1,HIGH);

    analogWrite(B,speed);
    digitalWrite(I4,HIGH);
    digitalWrite(I3,LOW);

  }

  void backward() {

    analogWrite(A,speed);
    analogWrite(B,speed);
    digitalWrite(I4,LOW);
    digitalWrite(I3,HIGH);
    digitalWrite(I2,HIGH);
    digitalWrite(I1,LOW);

  }

  void left(){

    analogWrite(A,speed);
    analogWrite(B,speed);
    digitalWrite(I4,LOW);
    digitalWrite(I3,HIGH);
    digitalWrite(I2,HIGH);
    digitalWrite(I1,LOW);

  }

  void right(){

    analogWrite(A,speed);
    analogWrite(B,speed);
    digitalWrite(I4,LOW);
    digitalWrite(I3,HIGH);
    digitalWrite(I2,LOW);
    digitalWrite(I1,HIGH);

  }

  void stop() {

    digitalWrite(A,LOW);
    digitalWrite(B,LOW);
    delay(1000);

  }

};

struct Core {

  Motors motors;
  Sensors sensors;
  boolean idle;
  int cur_route_index;

  int button_pin = 2;
  int slider_pin = A0;
  int selection_type = 0, from = 0, to = 0;
  int select_index = 0;

  // Plan
  Point a, b, c, d, e, f;
  PointStack plan;
  Astar GPS;

  Core() {
    motors = Motors();
    sensors = Sensors();
    idle = true;

    pinMode(button_pin, INPUT);

    cur_route_index = -1;

    /*
      PLAN :
                    b
                    |
               a -- e --﹁
                         |
                    d -- f -- c
      COLORS :
        a - e : RED
        b - e : BLUE
        f - e : GREEN
        d - f : BLUE
        c - f : RED
    */

    // Creating the map
    a = Point(-1, 0); // Salon
    b = Point(0, 1); // Cuisine
    c = Point(2, -1); // SDB
    d = Point(0, -1); // Chambre
    e = Point(0, 0); // PT1
    f = Point(1, -1); // PT2

    plan.push(&a);
    plan.push(&b);
    plan.push(&c);
    plan.push(&d);
    plan.push(&e);
    plan.push(&f);

    link(plan.at(0), plan.at(4));
    link(plan.at(1), plan.at(4));
    link(plan.at(5), plan.at(4));
    link(plan.at(5), plan.at(3));
    link(plan.at(5), plan.at(2));

    GPS = Astar();

  }

  // mother function -> main core
  void tick() {
    if(idle) {
      motors.stop();
      select_dest();
    }
    else {
      slcd.setCursor(0,0);
      slcd.print("Itineraire en");
      slcd.setCursor(0, 1);
      slcd.print("cours...");

      sensors.update(); // update les couleurs
      route_core(); // se deplacer
    }
  }

  // route info UI
  void itinerary_dashboard() {

  }

  // route selection UI
  void select_dest() {

    print("Route selection mode");

    slcd.setCursor(0,0);
    slcd.print(selection_type == 0 ? "Select depart." : "Select dest.");
    slcd.setCursor(0,1);

    int buttonState = digitalRead(button_pin);
    int sliderState = analogRead(slider_pin);

    if (select_index != map(sliderState, 0, 1024, 0, 4)) {
      slcd.setCursor(0,1);
      slcd.print("                 ");
    }

    select_index = map(sliderState, 0, 1024, 0, 4);

    if(buttonState) {
      if(selection_type == 0) {
        from = select_index;
        selection_type = 1;
        print("Selected from: " + idToName(from) + "\n");

        slcd.setCursor(0,0);
        slcd.print("Selected :      ");
        slcd.setCursor(0,1);
        slcd.print("                 ");
        slcd.print(idToName(select_index).c_str());
        delay(1000);

      } else {
        to = select_index;
        selection_type = 0;
        idle = false;
        print("Selected to: " + idToName(to) + "\n");
        print("Calculating route...");

        GPS.setStart(plan.at(from));
        GPS.setGoal(plan.at(to));

        GPS.find();

        // Show the itinerary
        for (int i = GPS.getItinerary().length()-1; i >= 0; i--) {
            Serial.println(idToName(GPS.getItinerary().at(i)->id));
        }

        cur_route_index = 0;

        slcd.clear();

        nextStep(false);
      }
    }

    slcd.print(idToName(select_index).c_str());

    delay(100);
  }

  // called for positioning the robot in the right axis when hit BLACK checkpoint
  void changeRouteProtocol() {
    sensors.update(); // update les couleurs

    if(sensors.color_left == sensors.color_to_follow) {
      motors.left();
      delay(1000);
    }
    else if(sensors.color_right == sensors.color_to_follow) {
      motors.right();
      delay(1000);
    }
    motors.forward();
    delay(1000);
  }

  // called when the itinerary is totally finished -> turns the robot 180°
  void itineraryEndProtocol() {
    int timeSinceStarted = 0;
    while(true) {
      sensors.update(); // update les couleurs
      motors.left();
      if(sensors.color_forward == sensors.color_to_follow) {
        motors.stop();
        motors.backward();
        delay(1000);
        return;
      }
      timeSinceStarted++;
      if(timeSinceStarted > oo) {
        print("End protocol failed -> exiting");
        return;
      }
    }
  }

  // called when a BLACK checkpoint is hit -> updating itinerary and positioning the robot
  void nextStep(bool useChangeRouteProtocol) {
    cur_route_index++;
    if (cur_route_index == GPS.getItinerary().length()) {
      print("Itinerary finished. Switching to end protocol...");
      itineraryEndProtocol();
      cur_route_index = -1;
      idle = true;
      return;
    }
    print("Checkpoint hit. Changing current route...");
    sensors.color_to_follow = getRouteColor(GPS.getItinerary().at(GPS.getItinerary().length()-1 - cur_route_index + 1)->id, GPS.getItinerary().at(GPS.getItinerary().length()-1 - cur_route_index)->id);
    print(String("Now following color : ") + String(sensors.color_to_follow.c_str()));
    if(useChangeRouteProtocol)
      changeRouteProtocol();
  }

  // core function of the itinerary system
  void route_core() {
    move();

    // hit checkpoint
    if(sensors.color_forward == BLACK) {
      nextStep(true);
    }
  }

  // main move function -> makes the robot go forward and fixes trajectory live
  void move() {

    if(sensors.color_left == sensors.color_to_follow) {
      motors.left();
    }
    else if(sensors.color_right == sensors.color_to_follow) {
      motors.right();
    }
    else {
      motors.forward();
    }
    delay(100);

  }

};

Core core;

void setup() {
  Serial.begin(9600);
  print("Starting program...");
  Wire.begin();
  slcd.begin();
  multiplexer.TCA9545SetConfig();
  core = Core();
  print("End of setup.");
}

void loop() {
  core.tick();
}

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

String getRouteColor(int a, int b) {
  /*
      PLAN :
                    b
                    |
               a -- e --﹁
                         |
                    d -- f -- c
      COULEURS :
        a - e : RED
        b - e : BLUE
        f - e : GREEN
        d - f : BLUE
        c - f : RED
    */
    if (a == 0 && b == 4) return RED;
    if (a == 1 && b == 4) return BLUE;
    if (a == 5 && b == 4) return GREEN;
    if (a == 5 && b == 2) return BLUE;
    if (a == 5 && b == 3) return RED;
}
