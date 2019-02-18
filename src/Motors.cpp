#include "Arduino.h"
#include "Utils.h"
#include "Motors.h"

Motors::Motors(): pwmA(3), brakeA(9), dirA(12), pwmB(11), brakeB(8), dirB(13), speed(128) {
  pinMode(brakeA, OUTPUT);
  pinMode(dirA, OUTPUT);
  pinMode(brakeB, OUTPUT);
  pinMode(dirB, OUTPUT);
  digitalWrite(brakeA, LOW);
  digitalWrite(brakeB, LOW);
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, LOW);
}

void Motors::setSpeed(int _speed) {
  speed = _speed;
  setSpeed();
}

void Motors::setSpeed() {
  analogWrite(pwmA, speed);
  analogWrite(pwmB, speed);
}

void Motors::forward () {
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, LOW);
  setSpeed();
}

void Motors::backward () {
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, HIGH);
  setSpeed();

}

void Motors::left () {
  digitalWrite(dirA, LOW);
  digitalWrite(dirB, LOW);
  setSpeed();

}

void Motors::right () {
  digitalWrite(dirA, HIGH);
  digitalWrite(dirB, HIGH);
  setSpeed();
}

void Motors::stop () {
  digitalWrite(brakeA, HIGH);
  digitalWrite(brakeB, HIGH);
  setSpeed(0);
  delay(20);
}
