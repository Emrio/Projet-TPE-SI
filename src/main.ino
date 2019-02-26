#include <Wire.h>
#include <SoftwareSerial.h>
#include <SerialLCD.h>
#include <SDL_Arduino_TCA9545.h>

#include "Utils.h"
#include "Sensors.h"
#include "Motors.h"
#include "GPS.h"

SerialLCD slcd(4, 5);
SDL_Arduino_TCA9545 multiplexer;

class Core {
public:
  Core(); // Dummy constructor
  void tick(); // Mother function -> main core

  void updateButtonState(); // Updates the state of the main button (pressed/unpressed)
  void updateSliderState(int maxSelectState); // Updates the position of the slider
  void printlcd(int line, String str);      //-| Prints to a line of the LCD screen
  void printlcd(int line, const char* str); //-|
  void clearlinelcd(int line); // Clears a line of the LCD screen

  void itineraryDashboard(); // Called during route, route info UI
  void selectDest(); // Called when idle, route selection UI

  void changeRouteProtocol(); // Protocol used when switching to a new color by positioning the robot in the correct axis when BLACK checkpoint is hit
  void itineraryEndProtocol(); // Protocol used when arrived to the destination, turns the robot by 180°

  void nextStep(bool useChangeRouteProtocol); // Called when hit a BLACK checkpoint
  void routeCore(); // Core of the robot's movements
  void move(); // makes the robot go forward and corrects the trajectory of the robot

private:
  Motors motors;
  Sensors sensors;
  GPS gps;

  bool idle; // is the robot idle

  // UI variables
  // type of selection in selectDest(), selection for slider, previous selection for slider, button press state
  int selectionType = 0, selectIndex = 0, previousSelectIndex = 0, buttonState = 0;

  bool turnRightOnNotFound = true;
};

Core::Core() {
  motors = Motors();
  sensors = Sensors(&multiplexer);
  gps = GPS();

  idle = true;

  pinMode(2, INPUT);
}

void Core::tick() {
  if(idle) {
    motors.stop();
    selectDest();
  } else {
    itineraryDashboard();
    sensors.update(); // update colors
    routeCore(); // move
  }
}

void Core::updateButtonState() {
  buttonState = digitalRead(2);
}

void Core::updateSliderState(int maxSelectState) {
  int sliderState = analogRead(A0);
  previousSelectIndex = selectIndex;
  selectIndex = map(sliderState, 5, 50, 0, maxSelectState);
  selectIndex = min(selectIndex, maxSelectState);
}

void Core::printlcd(int line, String str) {
  slcd.setCursor(0, line);
  slcd.print(str.c_str());
}

void Core::printlcd(int line, const char* str) {
  slcd.setCursor(0, line);
  slcd.print(str);
}

void Core::clearlinelcd(int line) {
  slcd.setCursor(0, line);
  slcd.print("                 ");
}

void Core::itineraryDashboard() {
  print("Route info");

  updateSliderState(4);

  switch(selectIndex) {
    case 0:
      // Itinéraire en || cours...
      printlcd(0, "Itineraire en    ");
      printlcd(1, "cours...");
      break;
    case 1:
      // Départ : ||  ________
      printlcd(0, "Depart :");
      printlcd(1, gps.departure());
      break;
    case 2:
      // Arrivé : ||  ________
      printlcd(0, "Destination :    ");
      printlcd(1, gps.destination());
      break;
    case 3:
      // Prochaine étape: ||  ________
      printlcd(0, "Prochaine etape: ");
      clearlinelcd(1);
      printlcd(1, gps.next());
      break;
    case 4:
    default:
      // Trajet : ||  __/__ (__%)
      printlcd(0, "Trajet :         ");
      printlcd(1, String(gps.curRouteIndex()) + "/" + String(gps.lengthOfRoute()) + " (" + String(gps.percentCompleted()) + "%)");
      break;
  }

  delay(100);
}

void Core::selectDest() {
  print("Route selection mode");
  printlcd(0, selectionType == 0 ? "Select depart." : "Select dest.");
  updateButtonState();
  updateSliderState(3);

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

      sensors.colorToFollow = getRouteColor(gps.currentId(), gps.nextId());
    }
  }

  printlcd(1, idToName(selectIndex));
  delay(10);
}

void Core::changeRouteProtocol() {
  print("Route change");

  int tickSinceBeginning = 0;

  while (true) {
    if (++tickSinceBeginning < 500) {
      motors.setSpeed(speedOfMvt);
      motors.forward();
      delay(100);
      sensors.update(); // update colors + gives some delay
      motors.stop();

      if(sensors.colorLeft == sensors.colorToFollow) {
        motors.setSpeed(speedAtRotation);
        motors.left();
        delay(150);
        motors.setSpeed(speedOfMvt);
        motors.forward();
        delay(150);
        return;
      } else if(sensors.colorRight == sensors.colorToFollow) {
        motors.setSpeed(speedAtRotation);
        motors.right();
        delay(150);
        motors.setSpeed(speedOfMvt);
        motors.forward();
        delay(150);
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
        delay(150);
        motors.setSpeed(speedOfMvt);
        motors.forward();
        delay(100);
        return;
      } else if(sensors.colorRight == sensors.colorToFollow) {
        motors.setSpeed(speedAtRotation);
        motors.right();
        delay(150);
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

void Core::itineraryEndProtocol() {
  print("End protocol");

  int timeSinceStarted = 0;
  while(true) {
    sensors.update(); // update colors
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

void Core::nextStep(bool useChangeRouteProtocol) {
  print("Next Step");
  gps.nextStep();
  if (gps.finished()) {
    print("Itinerary finished. Switching to end protocol...");
    itineraryEndProtocol();
    idle = true;
    return;
  }
  print("Checkpoint hit. Changing current route...");
  print(gps.currentId() + ' ' + gps.nextId());
  sensors.colorToFollow = getRouteColor(gps.currentId(), gps.nextId());
  print("Now following color : ");
  print(sensors.colorToFollow);
  if(useChangeRouteProtocol) changeRouteProtocol();
}

void Core::routeCore() {
  move();

  // hit checkpoint
  if(sensors.colorForward == BLACK) {
    print("Checkpoint");
    nextStep(true);
  }
}

void Core::move() {
  if(sensors.colorForward == sensors.colorToFollow) {
    motors.setSpeed(speedOfMvt);
    motors.forward();
    delay(200);
  } else if(sensors.colorLeft == sensors.colorToFollow) {
    motors.setSpeed(speedAtRotation);
    motors.left();
    turnRightOnNotFound = false;
  } else if(sensors.colorRight == sensors.colorToFollow) {
    motors.setSpeed(speedAtRotation);
    motors.right();
    turnRightOnNotFound = true;
  } else {
    motors.setSpeed(speedAtRotation);
    if (turnRightOnNotFound) motors.right();
    else motors.left();
  }
  delay(100);
  motors.stop();
}

Core core;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting program...");
  Wire.begin();
  slcd.begin();
  multiplexer.TCA9545SetConfig();
  core = Core();
  Serial.println("End of setup.");
}

void loop() {
  core.tick();
  delay(1);
}
