
// ask_transmitter.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to transmit messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) transmitter with an TX-C1 module
// Tested on Arduino Mega, Duemilanova, Uno, Due, Teensy, ESP-12

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h>  // Not actually used but needed to compile
#endif

RH_ASK driver(2000, 7, 6);
int top_r = 0;
int bot_r = 0;
int top_l = 0;
int bot_l = 0;
char msg[6] = "ssss\n";

void setup() {
  Serial.begin(9600);  // Debugging only
  if (!driver.init()) {
    Serial.println("init failed");
  }

  pinMode(3, INPUT);  // top_r
  pinMode(1, INPUT);  // bot_r
  pinMode(9, INPUT);  // top_l
  pinMode(8, INPUT);  // bot_l

  Serial.println("initilized controller");
}

void sendMessage(char *stringMsg) {
  // Serial.println(stringMsg);
  driver.send((uint8_t *)stringMsg, strlen(stringMsg));
  driver.waitPacketSent();
}

void loop() {
  top_r = digitalRead(3);
  bot_r = digitalRead(1);
  top_l = digitalRead(9);
  bot_l = digitalRead(8);



  msg[0] = 's';  // fr
  msg[1] = 's';  // br
  msg[2] = 's';  // fl
  msg[3] = 's';  // bl
  // msg[0] = top_r == HIGH ? 'f' : 's'; // fr
  // msg[1] = bot_r == HIGH ? 'b' : 's'; // br
  // msg[2] = top_l == HIGH ? 'f' : 's'; // fl
  // msg[3] = bot_l == HIGH ? 'b' : 's'; // bl

  // all forwards
  if (top_l == HIGH && top_r == HIGH && bot_l == LOW && bot_r == LOW) {
    msg[0] = msg[1] = msg[2] = msg[3] = 'f';
  } else
    // all backwards
    if (top_l == LOW && top_r == LOW && bot_l == HIGH && bot_r == HIGH) {
      msg[0] = msg[1] = msg[2] = msg[3] = 'b';
    } else
      // slide left
      if (top_l == HIGH && top_r == LOW && bot_l == HIGH && bot_r == LOW) {
        msg[0] = 'f';
        msg[1] = 'b';
        msg[2] = 'b';
        msg[3] = 'f';
      } else
        // slide right
        if (top_l == LOW && top_r == HIGH && bot_l == LOW && bot_r == HIGH) {
          msg[0] = 'b';
          msg[1] = 'f';
          msg[2] = 'f';
          msg[3] = 'b';
        } else
          // turn right
          if (top_l == HIGH && top_r == LOW && bot_l == LOW && bot_r == HIGH) {
            msg[0] = 'b';  // fr
            msg[1] = 'b';  // br
            msg[2] = 'f';  // fl
            msg[3] = 'f';  // bl
          } else
            // turn left
            if (top_l == LOW && top_r == HIGH && bot_l == HIGH && bot_r == LOW) {
              msg[0] = 'f';  // fr
              msg[1] = 'f';  // br
              msg[2] = 'b';  // fl
              msg[3] = 'b';  // bl
            }
  Serial.println(String(msg));

  // if (msg != "ssss") {
  // msg_prev = msg;
  // msg += "\n";
  // char str_array[msg.length()];
  // msg.toCharArray(str_array, msg.length());
  // sendMessage(strtok(str_array, ""));
  sendMessage(msg);
  // }
  // msg = "";
}