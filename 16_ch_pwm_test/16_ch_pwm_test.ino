#include <DFRobot_VL53L0X.h>
#include "Wire.h"
#include "Adafruit_VL53L0X.h"
#include <Adafruit_PWMServoDriver.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define RIGHT_SERVO 7
#define LEFT_SERVO 8
#define BTN_PIN 16

const int LEFT_SERVO_FORWARD_FULL_SPEED = 360;
const int LEFT_SERVO_STOP_SPEED = 280;
const int LEFT_SERVO_BACKWARD_FULL_SPEED = 190;

const int RIGHT_SERVO_FORWARD_FULL_SPEED = 190;
const int RIGHT_SERVO_STOP_SPEED = 280;
const int RIGHT_SERVO_BACKWARD_FULL_SPEED = 370;

int speed = 30;

bool isButtonDownPrevState = false;
bool isRunning = false;

void setup() {
  Serial.begin(19200);
  Wire.begin();
  pwm.begin();
  
  // wait until serial port opens for native USB devices
  while (!Serial) {
    delay(1);
  }

  lox.begin(0x30);
  // if (!lox.begin(0x30)) {
  //   Serial.println(F("Failed to boot VL53L0X"));
  //   while (1);
  // }

  pinMode(BTN_PIN, INPUT);

  // start continuous ranging
  lox.startRangeContinuous();

  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  delay(10);
  pwm.setPWM(LEFT_SERVO, 0, LEFT_SERVO_STOP_SPEED);
  pwm.setPWM(RIGHT_SERVO, 0, RIGHT_SERVO_STOP_SPEED);
}

void go(int SERVO, int speed) {
  if (speed > 100 || speed < -100) { return; }

  if (SERVO == RIGHT_SERVO && speed > 0) {
    pwm.setPWM(RIGHT_SERVO, 0, RIGHT_SERVO_STOP_SPEED - speed);
  } else if (SERVO == RIGHT_SERVO && speed < 0) {
    pwm.setPWM(RIGHT_SERVO, 0, RIGHT_SERVO_STOP_SPEED - speed);
  } else if (SERVO == LEFT_SERVO && speed > 0) {
    pwm.setPWM(LEFT_SERVO, 0, LEFT_SERVO_STOP_SPEED + speed);
  } else if (SERVO == LEFT_SERVO && speed < 0) {
    pwm.setPWM(LEFT_SERVO, 0, LEFT_SERVO_STOP_SPEED + speed);
  } else {
    pwm.setPWM(RIGHT_SERVO, 0, RIGHT_SERVO_STOP_SPEED);
    pwm.setPWM(LEFT_SERVO, 0, LEFT_SERVO_STOP_SPEED);
  }
}

void scanForOpening() {
  int scanSpeed = 20;
  go(LEFT_SERVO, -scanSpeed);
  go(RIGHT_SERVO, -scanSpeed);
  delay(300);
  // for (int i = 0; i < scanSpeed; i++) {
    go(LEFT_SERVO, scanSpeed);
    go(RIGHT_SERVO, -scanSpeed);
    delay(300);
    if (lox.isRangeComplete() && lox.readRange() > 140) {
      go(LEFT_SERVO, 0);
      go(RIGHT_SERVO, 0);
      return;
    }
  // }  
  delay(300);
  // for (int i = 0; i < scanSpeed * 2; i++) {
    go(LEFT_SERVO, -scanSpeed);
    go(RIGHT_SERVO, scanSpeed);
    delay(100);
    if (lox.isRangeComplete() && lox.readRange() > 140) {
      go(LEFT_SERVO, 0);
      go(RIGHT_SERVO, 0);
      return;
    }
  // }  
}

void loop() {
  bool isButtonDown = digitalRead(BTN_PIN) == LOW;

  if (isButtonDown && !isButtonDownPrevState) {
    isButtonDownPrevState = true;
    isRunning = !isRunning;
    if (isRunning) {
      Serial.println("Running");
    } else {
      Serial.println("Stopped");
    }
  } else if (!isButtonDown) {
    isButtonDownPrevState = false;
  }

  uint16_t distance = lox.isRangeComplete() ? lox.readRange() : 100;
  // Serial.println(distance);
  if (isRunning) {
    
    go(RIGHT_SERVO, speed);
    go(LEFT_SERVO, speed);
    if (distance < 90) {
      Serial.println("Turning");
      go(RIGHT_SERVO, 0);
      go(LEFT_SERVO, 0);
      delay(10);
      scanForOpening();
    }
  } else {
    go(RIGHT_SERVO, 0);
    go(LEFT_SERVO, 0);
  }
  
  // go(RIGHT_SERVO, speed);
  // go(LEFT_SERVO, speed);
  // uint16_t distance = lox.isRangeComplete() ? lox.readRange() : 100;
  // if (distance < 90) {
  //   Serial.println("turning");
  //   go(RIGHT_SERVO, 0);
  //   go(LEFT_SERVO, 0);
  //   delay(10);
  //   scanForOpening();
  // }
}