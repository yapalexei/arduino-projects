
#define MAX_RESOLUTION_VGA //  640x480
#include "esp32cam.h"
#include "esp32cam/JpegDecoder.h"
#include "esp32cam/motion/Detector.h"
#include "esp32cam/motion/SimpleChange.h"
#include "esp32cam/motion/roi/Rect.h"
#include "esp32cam/motion/roi/Circle.h"
#include "esp32cam/http/LiveFeed.h"
#include <ESP32Servo.h>

Servo myservo;

Eloquent::Esp32cam::Cam cam;
Eloquent::Esp32cam::JpegDecoder decoder;
Eloquent::Esp32cam::Motion::SimpleChange algorithm;
Eloquent::Esp32cam::Motion::Detector detector(algorithm);
Eloquent::Esp32cam::Http::LiveFeed feed(cam);
Eloquent::Esp32cam::Motion::RoI::Rect leftFar(0, 0, 160, 480, 0.5);
Eloquent::Esp32cam::Motion::RoI::Rect leftCenter(160, 0, 160, 480, 0.5);
Eloquent::Esp32cam::Motion::RoI::Rect rightCenter(320, 0, 160, 480, 0.5);
Eloquent::Esp32cam::Motion::RoI::Rect rightFar(480, 0, 160, 480, 0.5);

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("Init");
    // ESP32PWM::allocateTimer(0);
	  // ESP32PWM::allocateTimer(1);
	  // ESP32PWM::allocateTimer(2);
	  // ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);    // standard 50 hz servo
	  myservo.attach(4);

    cam.xiao(); // custom method by alexei
    // cam.aithinker(); // this is for the traditional esp32-cam boards
    cam.highQuality();
    cam.vga();
    cam.highestSaturation();
    cam.disableAutomaticWhiteBalance();
    cam.disableAutomaticExposureControl();
    cam.disableGainControl();

    // See 10_Image_Wise_Motion_Detection for details
    detector.trainFor(30);
    detector.retrainAfter(33ULL * 60);
    detector.triggerAbove(0.3);
    detector.denoise();
  
    algorithm.differBy(10);
    algorithm.smooth(0.9);
    algorithm.onlyUpdateBackground();


    while (!cam.begin()) {
      Serial.println(cam.getErrorMessage());
    }
}

int MAX = 10;
int waitPeriod = MAX;
int direction = 0; // -1 left; 0 center; 1 right;

void loop() {
    if (!cam.capture()) {
        Serial.println(cam.getErrorMessage());
        return;
    }

    if (!decoder.decode(cam)) {
        Serial.println(decoder.getErrorMessage());
        return;
    }

    if (!detector.update(decoder.luma)) {
        Serial.println(detector.getErrorMessage());
        return;
    }

    if (detector.triggered()) {
      Serial.println("Motion detected");
    }

    if (waitPeriod > 0) {
      waitPeriod -= 1;
    }

    if (waitPeriod <= 0 && direction != 0) {
      myservo.write(90);
      direction = 0;
      delay(30);
    }
    

    if (detector.triggeredIn(leftFar) && waitPeriod <= 0) {
      myservo.write(160);
      direction = -2;
      waitPeriod = MAX;
    }
    if (detector.triggeredIn(leftCenter) && waitPeriod <= 0) {
      myservo.write(135);
      direction = -1;
      waitPeriod = MAX;
    }
    if (detector.triggeredIn(rightCenter) && waitPeriod <= 0) {
      myservo.write(45);
      direction = 1;
      waitPeriod = MAX;
    }
    if (detector.triggeredIn(rightFar) && waitPeriod <= 0) {
      myservo.write(20);
      direction = 2;
      waitPeriod = MAX;
    }
}
