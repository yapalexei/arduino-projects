// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module
// Tested on Arduino Mega, Duemilanova, Uno, Due, Teensy, ESP-12
 
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif
 
// RH_ASK driver;
// RH_ASK driver(2000, 4, 5, 0); // ESP8266 or ESP32: do not use pin 11 or 2
// RH_ASK driver(2000, 3, 4, 0); // ATTiny, RX on D3 (pin 2 on attiny85) TX on D4 (pin 3 on attiny85), 
// RH_ASK driver(2000, PD14, PD13, 0); STM32F4 Discovery: see tx and rx on Orange and Red LEDS
RH_ASK driver(2000, 7);

void setup() {
  Serial.begin(9600);   // Debugging only
  if (!driver.init()) {
    Serial.println("init failed");
  }
  Serial.println("initilized receiver");
}

void convert(const String msg) {
  String cmd = "";
  for (int i = 0; i < msg.length(); i++) {
    Serial.println(msg[i]);
    switch (i) {
    0:
      if (msg[i] == "f") {
        cmd += "fr:250";
      } else if(msg[i] == "b") {
        cmd += "fr:-250";
      } else {
        cmd += "fr:0";
      }
      break;
    1:
      if (msg[i] == "f") {
        cmd += "br:250";
      } else if(msg[i] == "b") {
        cmd += "br:-250";
      } else {
        cmd += "br:0";
      }
      break;
    2:
      if (msg[i] == "f") {
        cmd += "fl:250";
      } else if(msg[i] == "b") {
        cmd += "fl:-250";
      } else {
        cmd += "fl:0";
      }
      break;
    3:
      if (msg[i] == "f") {
        cmd += "bl:250";
      } else if(msg[i] == "b") {
        cmd += "bl:-250";
      } else {
        cmd += "bl:0";
      }
      break;
    }
    Serial.println(cmd);
  }
}

void loop() {
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);
 
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      String msg = (char*)buf;
      convert(msg.substring(0, buflen));
    }
}
