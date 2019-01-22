struct Motors {

  int pwmA = 3,
      brakeA = 9,
      dirA = 12,
      pwmB = 11,
      brakeB = 8,
      dirB = 13,
      speed = 128;

  Motors() {
    pinMode(brakeA, OUTPUT);
    pinMode(dirA, OUTPUT);
    pinMode(brakeB, OUTPUT);
    pinMode(dirB, OUTPUT);
    digitalWrite(brakeA, LOW);
    digitalWrite(brakeB, LOW);
    digitalWrite(dirA, HIGH);
    digitalWrite(dirB, LOW);
  }

  void setSpeed(int _speed) {
    speed = _speed;
    setSpeed();
  }

  void setSpeed() {
    analogWrite(pwmA, 255);
    analogWrite(pwmB, 255);
  }

  void forward() {

    digitalWrite(dirA, HIGH);
    digitalWrite(dirB, LOW);
    setSpeed();

  }

  void backward() {

    digitalWrite(dirA, LOW);
    digitalWrite(dirB, HIGH);
    setSpeed();

  }

  void left(){

    digitalWrite(dirA, LOW);
    digitalWrite(dirB, LOW);
    setSpeed();

  }

  void right(){

    digitalWrite(dirA, HIGH);
    digitalWrite(dirB, HIGH);
    setSpeed();

  }

  void stop() {

    setSpeed(0);
    delay(50);

  }

};
