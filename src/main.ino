#include <Wire.h>
#include <SoftwareSerial.h>
#include <SerialLCD.h>
#include <SDL_Arduino_TCA9545.h>

#include "Utils.h"
#include "Sensors.h"
#include "Motors.h"
#include "GPS.h"

const int speedOfMvt = 80;
const int speedAtRotation = 150;

SerialLCD slcd(4, 5);
SDL_Arduino_TCA9545 multiplexer;

struct Core {

  Motors motors;
  Sensors sensors;
  GPS gps;

  bool idle; // is the robot idle

  // type of selection in selectDest(), selection for slider, previous selection for slider, button press state
  int selectionType = 0, selectIndex = 0, previousSelectIndex = 0, buttonState = 0;

  Core() {
    motors = Motors();
    sensors = Sensors(&multiplexer);
    gps = GPS();

    idle = true;

    pinMode(2, INPUT);
  }

  // mother function -> main core
  void tick() {
    if(idle) {
      motors.stop();
      selectDest();
    } else {
      itineraryDashboard();

      sensors.update(); // update les couleurs
      routeCore(); // se deplacer
    }
  }

  void updateButtonState() {
    buttonState = digitalRead(2);
  }

  void updateSliderState(int maxSelectState) {
    int sliderState = analogRead(A0);
    previousSelectIndex = selectIndex;
    selectIndex = map(sliderState, 0, 1024, 0, maxSelectState);
    Serial.println(sliderState);
    Serial.println(selectIndex);
  }

  void printlcd(int line, String str) {
    slcd.setCursor(0, line);
    slcd.print(str.c_str());
  }

  void printlcd(int line, const char* str) {
    slcd.setCursor(0, line);
    slcd.print(str);
  }

  void clearlinelcd(int line) {
    slcd.setCursor(0, line);
    slcd.print("                 ");
  }

  // route info UI
  void itineraryDashboard() {

    print("Route info");

    updateSliderState(4);

    switch(selectIndex) {
      case 0:
        // Itinéraire en || cours...
        printlcd(0, "Itinéraire en");
        printlcd(1, "cours...");
        break;
      case 1:
        // Départ : ||  ________
        printlcd(0, "Depart :");
        printlcd(1, gps.departure());
        break;
      case 2:
        // Arrivé : ||  ________
        printlcd(0, "Destination :");
        printlcd(1, gps.destination());
        break;
      case 3:
        // Prochaine étape: ||  ________
        printlcd(0, "Prochaine etape:");
        clearlinelcd(1);
        printlcd(1, gps.next());
        break;
      case 4:
      default:
        // Trajet : ||  __/__ (__%)
        printlcd(0, "Trajet :");
        printlcd(1, gps.curRouteIndex() + "/" + String(gps.lengthOfRoute()) + " (" + String(gps.percentCompleted()) + "%%");
        break;
    }

    delay(100);
  }

  // route selection UI
  void selectDest() {

    print("Route selection mode");

    printlcd(0, selectionType == 0 ? "Select depart." : "Select dest.");

    updateButtonState();
    updateSliderState(4);

    if (selectIndex != previousSelectIndex)
      clearlinelcd(1);

    if(buttonState) {
      if(selectionType == 0) {
        gps.setDeparture(selectIndex);
        selectionType = 1;
        print(gps.departure());

        printlcd(0, "Selected :     ");
        clearlinelcd(1);
        printlcd(1, idToName(selectIndex));
        delay(1000);
      } else {
        gps.setDestination(selectIndex);
        selectionType = 0;
        idle = false;
        print(gps.destination());
        print("Calculating route...");

        gps.calculate();

        nextStep(false);
      }
    }

    printlcd(1, idToName(selectIndex));
    delay(10);
  }

  // called for positioning the robot in the right axis when hit BLACK checkpoint
  void changeRouteProtocol() {
    print("Route change");

    int tickSinceBeginning = 0;

    while (true) {
      if (++tickSinceBeginning < 500) {
        motors.setSpeed(speedOfMvt);
        motors.forward();
        sensors.update(); // update colors + gives some delay
        motors.stop();

        if(sensors.colorLeft == sensors.colorToFollow) {
          motors.setSpeed(speedAtRotation);
          motors.left();
          delay(1000);
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(100);
          return;
        } else if(sensors.colorRight == sensors.colorToFollow) {
          motors.setSpeed(speedAtRotation);
          motors.right();
          delay(1000);
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(100);
        } else if(sensors.colorForward == sensors.colorToFollow) {
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(200);
          return;
        }
      } else {
        motors.setSpeed(speedAtRotation);
        motors.left();
        sensors.update(); // update colors + gives some delay
        motors.stop();

        if(sensors.colorLeft == sensors.colorToFollow) {
          motors.setSpeed(speedAtRotation);
          motors.left();
          delay(1000);
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(100);
          return;
        } else if(sensors.colorRight == sensors.colorToFollow) {
          motors.setSpeed(speedAtRotation);
          motors.right();
          delay(1000);
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(100);
        } else if(sensors.colorForward == sensors.colorToFollow) {
          motors.setSpeed(speedOfMvt);
          motors.forward();
          delay(200);
          return;
        }
      }
    }
  }

  // called when the itinerary is totally finished -> turns the robot 180°
  void itineraryEndProtocol() {
    print("End protocol");
    int timeSinceStarted = 0;
    while(true) {
      sensors.update(); // update les couleurs
      motors.setSpeed(speedAtRotation);
      motors.left();
      if(sensors.colorForward == sensors.colorToFollow) {
        motors.stop();
        motors.setSpeed(speedOfMvt);
        motors.backward();
        delay(1000);
        motors.stop();
        return;
      }
      timeSinceStarted++;
      if(timeSinceStarted > 10e5) {
        print("End protocol failed -> exit");
        return;
      }
      delay(5);
    }
  }

  // called when a BLACK checkpoint is hit -> updating itinerary and positioning the robot
  void nextStep(bool useChangeRouteProtocol) {
    print("Next Step");
    gps.nextStep();
    if (gps.finished()) {
      print("Itinerary finished. Switching to end protocol...");
      itineraryEndProtocol();
      idle = true;
      return;
    }
    print("Checkpoint hit. Changing current route...");
    sensors.colorToFollow = getRouteColor(gps.currentId(), gps.nextId());
    print("Now following color : " + sensors.colorToFollow);
    if(useChangeRouteProtocol) changeRouteProtocol();
  }

  // core function of the itinerary system
  void routeCore() {
    move();

    // hit checkpoint
    if(sensors.colorForward == BLACK) {
      print("Checkpoint");
      nextStep(true);
    }
  }

  // main move function -> makes the robot go forward and fixes trajectory live
  void move() {
    if(sensors.colorLeft == sensors.colorToFollow) {
      print("left");
      motors.setSpeed(speedAtRotation);
      motors.left();
    } else if(sensors.colorRight == sensors.colorToFollow) {
      print("right");
      motors.setSpeed(speedAtRotation);
      motors.right();
    } else if(sensors.colorForward == sensors.colorToFollow) {
      print("forward");
      motors.setSpeed(speedOfMvt);
      motors.forward();
    } else {
      motors.setSpeed(speedAtRotation);
      motors.left();
    }
    delay(100);
    motors.stop();
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

  // Stoping motors because they kick off at the beginning...
  core.motors.stop();

  print("End of setup.");
}

void loop() {
  core.tick();
  delay(1);
}
