#include <Adafruit_PWMServoDriver.h>
 
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVO_STOP  280 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN  600 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX  2400 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates
#define DELAY 10

#define RIGHT_SERVO 7
#define LEFT_SERVO 8

int LEFT_SERVO_FORWARD_FULL_SPEED = 360;
int LEFT_SERVO_STOP_SPEED = 280;
int LEFT_SERVO_BACKWARD_FULL_SPEED = 190;

int RIGHT_SERVO_FORWARD_FULL_SPEED = 190;
int RIGHT_SERVO_STOP_SPEED = 280;
int RIGHT_SERVO_BACKWARD_FULL_SPEED = 370;

#define MAX_SPEED 100



void setup() {
  Serial.begin(115200);
  Serial.println("16 channel PWM test!");
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  delay(10);
  pwm.setPWM(LEFT_SERVO, 0, LEFT_SERVO_STOP_SPEED);
  pwm.setPWM(RIGHT_SERVO, 0, RIGHT_SERVO_STOP_SPEED);
}

void loop() {

  // for (int i = 3000; i < 4096; i++) {
  //   pwm.setPWM(RIGHT_SERVO, i, 0);
  //   Serial.printf("RIGHT %d\n", i);
  //   delay(10);
  // }

  // pwm.setPWM(RIGHT_SERVO, RIGHT_SERVO_STOP_SPEED, 0);
  // delay(2000);

  for (int i = 0; i < 500; i++) {
    int val = 3000 + i;
    if (i < 500) {
      pwm.setPWM(RIGHT_SERVO, val, 0);
      Serial.printf("right %d\n", val);
    }
    //  pwm.setPWM(RIGHT_SERVO, 0, i);
    // Serial.printf("right %d\n", val);
    delay(10);
  }

  pwm.setPWM(RIGHT_SERVO, RIGHT_SERVO_STOP_SPEED, 0);
  delay(2000);

  for (int i = 0; i < 500; i++) {
    int val = (3800 + i);
    // int val = (3750 + i % 4096);
    if (val < 4096) {
      pwm.setPWM(RIGHT_SERVO, val, 0);
      Serial.printf("left %d\n", val);
    } else {
      pwm.setPWM(RIGHT_SERVO, 0, val % 4096);
      Serial.printf("left %d\n", val);
    }
    //  pwm.setPWM(RIGHT_SERVO, 0, i);
    // Serial.printf("right %d\n", val);
    delay(10);
  }

  pwm.setPWM(RIGHT_SERVO, RIGHT_SERVO_STOP_SPEED, 0);
  delay(2000);
}
