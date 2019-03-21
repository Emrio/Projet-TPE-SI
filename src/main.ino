#include <Wire.h>
#include <SoftwareSerial.h>
#include <SerialLCD.h>
#include <Adafruit_TCS34725.h>
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

  void nextStep(); // Called when hit a BLACK checkpoint
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

  int T = 0; // used in movement control
};

Core::Core() {
  motors = Motors();
  sensors = Sensors(&multiplexer);
  gps = GPS();
  idle = true;
  
  pinMode(2, INPUT);
}

void Core::tick() {
  print("New tick");

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

      printlcd(0, "Selected :     ");
      clearlinelcd(1);
      printlcd(1, idToName(selectIndex));
      delay(1000);
    } else {
      gps.setDestination(selectIndex);
      selectionType = 0;
      idle = false;
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
  delay(500);

  int colors[] = { -5, -5, -5 };

  int tries = 0;

  while (tries++ < 3) {
    sensors.update();
    colors[0] = colors[0] > -3 ? sensors.colorForward : colors[0];
    colors[1] = colors[1] > -3 ? sensors.colorLeft : colors[1];
    colors[2] = colors[2] > -3 ? sensors.colorRight : colors[2];

    delay(200);

    if (sensors.colorToFollow == colors[1]) {
      do {
        motors.setSpeed(speedAtRotation);
        motors.left();
        delay(100);
        motors.stop();
        sensors.update();
      } while (sensors.colorForward != sensors.colorToFollow);
      return;
    } else if (sensors.colorToFollow == colors[2]) {
      do {
        motors.setSpeed(speedAtRotation);
        motors.right();
        delay(100);
        motors.stop();
        sensors.update();
      } while (sensors.colorForward != sensors.colorToFollow);
      return;
    }

    motors.stop();
    motors.setSpeed(speedOfMvt);
    delay(50);
    motors.forward();
    delay(200);
    motors.stop();
    delay(1000);
  }

  motors.setSpeed(speedOfMvt);
  motors.forward();
  delay(400);
  motors.stop();
}

void Core::nextStep() {
  print("Next Step");
  gps.nextStep();
  if (gps.finished()) {
    print("Itinerary finished. Switching to end protocol...");
    idle = true;
    return;
  }
  print("Checkpoint hit. Changing current route...");
  print(gps.currentId() + ' ' + gps.nextId());
  sensors.colorToFollow = getRouteColor(gps.currentId(), gps.nextId());
  print("Now following color : ");
  print(sensors.colorToFollow);
  changeRouteProtocol();
}

void Core::routeCore() {
  move();

  // hit checkpoint
  if(sensors.colorForward == BLACK) {
    print("Checkpoint");
    nextStep();
  }
}

void Core::move() {
  bool G = 0;
  bool D = 0;

	if (sensors.colorForward == sensors.colorToFollow) {
    print("forward");
    motors.setSpeed(speedOfMvt);
    motors.forward();
    delay(200);
		G = 0;
		D = 0;
    T = 0;
	} else if (sensors.colorLeft == sensors.colorToFollow) {
		G = 1;
    T = 0;
    print("left");
    motors.setSpeed(speedAtRotation);
    motors.left();
    delay(150);
  } else if (sensors.colorRight == sensors.colorToFollow) {
		D = 1;
    T = 0;
    print("right");
    motors.setSpeed(speedAtRotation);
    motors.right();
    delay(150);
	} else if (G == 1) {
    T = 0;
    print("left");
    motors.setSpeed(speedAtRotation);
    motors.left();
    delay(150);
	} else if (D == 1) {
    T = 0;
    print("right");
    motors.setSpeed(speedAtRotation);
    motors.right();
    delay(150);
	} else {
    print("forward");
    motors.setSpeed(speedOfMvt);
    motors.forward();
    delay(150);
    T++;
  }
  motors.stop();
  delay(100);
}

Core core;

void setup () {
  Serial.begin(9600);
  Serial.println("Starting program...");
  Wire.begin();
  slcd.begin();
  multiplexer.TCA9545SetConfig();
  Serial.println("End of setup.");
}

void loop () {
  core.tick();
  delay(1);
}
