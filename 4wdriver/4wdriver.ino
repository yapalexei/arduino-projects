#include <Arduino.h>
#define WHEEL_CH0 0
#define WHEEL0_PIN_A 32 
#define HALL_WHEEL0_A_PIN 14

#define WHEEL_CH1 1
#define WHEEL0_PIN_B 33
#define HALL_WHEEL0_B_PIN 15

#define WHEEL_CH2 2
#define WHEEL1_PIN_A 25
#define HALL_WHEEL1_A_PIN 16

#define WHEEL_CH3 3
#define WHEEL1_PIN_B 26
#define HALL_WHEEL1_B_PIN 17

#define WHEEL_CH4 4
#define WHEEL2_PIN_A 27
#define HALL_WHEEL2_A_PIN 19

#define WHEEL_CH5 5
#define WHEEL2_PIN_B 18
#define HALL_WHEEL2_B_PIN 23

#define WHEEL_CH6 6
#define WHEEL3_PIN_A 12
#define HALL_WHEEL3_A_PIN 34

#define WHEEL_CH7 7
#define WHEEL3_PIN_B 13
#define HALL_WHEEL3_B_PIN 35

#define MOTOR_REVS_TO_ONE_WHEEL_REV 120

#define HZ 15000


enum wheel {
  fl = 0,
  fr = 4,
  bl = 2,
  br = 6,
};

float targetSpeeds[] = {
  0, 0, 0, 0, 0, 0, 0, 0
};

float curSpeeds[] = {
  0, 0, 0, 0, 0, 0, 0, 0
};

float changeRate[] = {
  0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
};

const float defaultRate = 4;

bool setSpeed(wheel ch, float speed, float rate = -1) {
  targetSpeeds[ch] = speed;
  // if (rate > 0 && rate <= 4) {
  //   changeRate[ch] = rate;
  // }
  // changeRate[ch] = rate;
  return true;
}
// can only be 0,2,4,6
void updateSpeed(wheel ch) {
  float newSpeed = 0;
  float curSpeed = 0;
  if (curSpeeds[ch] > 0) {
    curSpeed = curSpeeds[ch];
  } else if (curSpeeds[ch+1] < 0) {
    curSpeed = curSpeeds[ch+1];
  }
  
  if (curSpeed < targetSpeeds[ch]) {
    if (curSpeed < targetSpeeds[ch]) {
      newSpeed = curSpeed + changeRate[ch];
      if (newSpeed > targetSpeeds[ch]) {
        newSpeed = targetSpeeds[ch];
      }
    }
  } else if (curSpeed > targetSpeeds[ch]) {
    if (curSpeed > targetSpeeds[ch]) {
      newSpeed = curSpeed - changeRate[ch];
      if (newSpeed < targetSpeeds[ch]) {
        newSpeed = targetSpeeds[ch];
      }
    }
  }
  
  if (curSpeed == targetSpeeds[ch]) {
    newSpeed = curSpeed;
  }

  if (targetSpeeds[ch] == 0) {
    if (abs(newSpeed) < changeRate[ch]) {
      newSpeed = 0;
    }
  }

  if (newSpeed > 0) {
    ledcWrite(ch, newSpeed);
    curSpeeds[ch] = newSpeed;
    ledcWrite(ch+1, 0.0);
    curSpeeds[ch+1] = 0.0;
  } else if (newSpeed < 0) {
    ledcWrite(ch, 0);
    curSpeeds[ch] = 0.0;
    ledcWrite(ch+1, -newSpeed);
    curSpeeds[ch+1] = newSpeed;
  } else {
    ledcWrite(ch, 0);
    curSpeeds[ch] = 0.0;
    ledcWrite(ch+1, 0);
    curSpeeds[ch+1] = 0.0;
  }
}

int tCount = 2000;
int inc = tCount / 8;
int count = 0;
unsigned long curMicros = micros();

// Front left
bool wheel0HallAState = false;
bool wheel0HallBState = false;
int wheel0Moving = 0;
int wheel0Direction = 0;
int wheel0Count = 0;
int wheel0CountPrev = 0;
int wheel0Micros = micros();
int wheel0MsPR = 0;
int wheel0CurrentRPM = 0;
int wheel0CountChange = 0;

// Back left
bool wheel1HallAState = false;
bool wheel1HallBState = false;
int wheel1Moving = 0;
int wheel1Direction = 0;
int wheel1Count = 0;

// Front right
bool wheel2HallAState = false;
bool wheel2HallBState = false;
int wheel2Moving = 0;
int wheel2Direction = 0;
int wheel2Count = 0;

// Back right
bool wheel3HallAState = false;
bool wheel3HallBState = false;
int wheel3Moving = 0;
int wheel3Direction = 0;
int wheel3Count = 0;

// Front left
void IRAM_ATTR wheel0HallA() {
  wheel0HallAState = digitalRead(HALL_WHEEL0_A_PIN);
  if (wheel0HallAState == HIGH) {
    if (wheel0Moving == 0) {
      wheel0Moving = -1;
      wheel0Direction = -1;
      wheel0Count += wheel0Direction;
      wheel0Micros = micros();
    }
  } else {
    if (wheel0Moving == -1) {
      wheel0Moving = 0;
      wheel0MsPR = micros() - wheel0Micros;
    }
  }
  if (wheel0Count < 0) {
    wheel0Count = 120;
  }
}

void IRAM_ATTR wheel0HallB() {
  wheel0HallBState = digitalRead(HALL_WHEEL0_B_PIN);
  if (wheel0HallBState == HIGH) {
    if (wheel0Moving == 0) {
      wheel0Moving = 1;
      wheel0Direction = 1;
      wheel0Count += wheel0Direction;
      wheel0Micros = micros();
    }
  } else {
    if (wheel0Moving == 1) {
      wheel0Moving = 0;
      wheel0MsPR = micros() - wheel0Micros;
    }
  }
  if (wheel0Count > MOTOR_REVS_TO_ONE_WHEEL_REV) {
    wheel0Count = 0;
  }
}

// Back left
void IRAM_ATTR wheel1HallA() {
  wheel1HallAState = digitalRead(HALL_WHEEL1_A_PIN);
  if (wheel1HallAState == HIGH) {
    if (wheel1Moving == 0) {
      wheel1Moving = -1;
      wheel1Direction = -1;
      wheel1Count += wheel1Direction;
    }
    curMicros = micros();
  } else {
    if (wheel1Moving == -1) wheel1Moving = 0;
  }
  if (wheel1Count < 0) {
    wheel1Count = 120;
  }
}
void IRAM_ATTR wheel1HallB() {
  wheel1HallBState = digitalRead(HALL_WHEEL1_B_PIN);
  if (wheel1HallBState == HIGH) {
    if (wheel1Moving == 0) {
      wheel1Moving = 1;
      wheel1Direction = 1;
      wheel1Count += wheel1Direction;
    }
    curMicros = micros();
  } else {
    if (wheel1Moving == 1) wheel1Moving = 0;
  }
  if (wheel1Count > MOTOR_REVS_TO_ONE_WHEEL_REV) {
    wheel1Count = 0; 
  }
}

// Front right
void IRAM_ATTR wheel2HallA() {
  wheel2HallAState = digitalRead(HALL_WHEEL2_A_PIN);
  if (wheel2HallAState == HIGH) {
    if (wheel2Moving == 0) {
      wheel2Moving = -1;
      wheel2Direction = -1;
      wheel2Count += wheel2Direction;
    }
    curMicros = micros();
  } else {
    if (wheel2Moving == -1) wheel2Moving = 0;
  }
  if (wheel2Count < 0) {
    wheel2Count = 120;
  }
}
void IRAM_ATTR wheel2HallB() {
  wheel2HallBState = digitalRead(HALL_WHEEL2_B_PIN);
  if (wheel2HallBState == HIGH) {
    if (wheel2Moving == 0) {
      wheel2Moving = 1;
      wheel2Direction = 1;
      wheel2Count += wheel2Direction;
    }
    curMicros = micros();
  } else {
    if (wheel2Moving == 1) wheel2Moving = 0;
  }
  if (wheel2Count > MOTOR_REVS_TO_ONE_WHEEL_REV) {
    wheel2Count = 0; 
  }
}

// Back right
void IRAM_ATTR wheel3HallA() {
  wheel3HallAState = digitalRead(HALL_WHEEL3_A_PIN);
  if (wheel3HallAState == HIGH) {
    if (wheel3Moving == 0) {
      wheel3Moving = -1;
      wheel3Direction = -1;
      wheel3Count += wheel3Direction;
    }
    curMicros = micros();
  } else {
    if (wheel3Moving == -1) wheel3Moving = 0;
  }
  if (wheel3Count < 0) {
    wheel3Count = 120;
  }
}
void IRAM_ATTR wheel3HallB() {
  wheel3HallBState = digitalRead(HALL_WHEEL3_B_PIN);
  if (wheel3HallBState == HIGH) {
    if (wheel3Moving == 0) {
      wheel3Moving = 1;
      wheel3Direction = 1;
      wheel3Count += wheel3Direction;
    }
    curMicros = micros();
  } else {
    if (wheel3Moving == 1) wheel3Moving = 0;
  }
  if (wheel3Count > MOTOR_REVS_TO_ONE_WHEEL_REV) {
    wheel3Count = 0; 
  }
}

bool readCmdWithValueAfterColen(String cmd) {
  int colonIndex = cmd.indexOf(':');
  if (colonIndex > 0) {
    String cmdName = cmd.substring(0, colonIndex);
    String cmdValue = cmd.substring(colonIndex+1);
    if (cmdName == "fl") {
      return setSpeed(wheel::fl, cmdValue.toFloat());
    } else if (cmdName == "fr") {
      return setSpeed(wheel::fr, cmdValue.toFloat());
    } else if (cmdName == "bl") {
      return setSpeed(wheel::bl, cmdValue.toFloat());
    } else if (cmdName == "br") {
      return setSpeed(wheel::br, cmdValue.toFloat());
    }
  }
  return false;
}

void readCmdListSeparatedByComma(String cmdStr) {
  if (cmdStr.length() <= 2) return;
  int commaIndex = cmdStr.indexOf(',');
  if (commaIndex > 0) {
    String cmd1 = cmdStr.substring(0, commaIndex);
    readCmdWithValueAfterColen(cmd1);
    String newCmdStr = cmdStr.substring(commaIndex+1, cmdStr.length());
    readCmdListSeparatedByComma(newCmdStr);
  } else {
    readCmdWithValueAfterColen(cmdStr);
  }
}

void handleSerial() {
  while (Serial.available() > 0) {
    readCmdListSeparatedByComma(Serial.readStringUntil('\n'));
  }
  updateSpeed(wheel::fl);
  updateSpeed(wheel::fr);
  updateSpeed(wheel::bl);
  updateSpeed(wheel::br);
}

void setup() {
	Serial.begin(115200);
  Serial.println("Booting...");

  ledcSetup(WHEEL_CH0, HZ, 8); ledcAttachPin(WHEEL0_PIN_A, WHEEL_CH0);
  ledcSetup(WHEEL_CH1, HZ, 8); ledcAttachPin(WHEEL0_PIN_B, WHEEL_CH1);
  ledcSetup(WHEEL_CH2, HZ, 8); ledcAttachPin(WHEEL1_PIN_A, WHEEL_CH2);
  ledcSetup(WHEEL_CH3, HZ, 8); ledcAttachPin(WHEEL1_PIN_B, WHEEL_CH3);
  ledcSetup(WHEEL_CH4, HZ, 8); ledcAttachPin(WHEEL2_PIN_A, WHEEL_CH4);
  ledcSetup(WHEEL_CH5, HZ, 8); ledcAttachPin(WHEEL2_PIN_B, WHEEL_CH5);
  ledcSetup(WHEEL_CH6, HZ, 8); ledcAttachPin(WHEEL3_PIN_A, WHEEL_CH6);
  ledcSetup(WHEEL_CH7, HZ, 8); ledcAttachPin(WHEEL3_PIN_B, WHEEL_CH7);


  attachInterrupt(HALL_WHEEL0_A_PIN, wheel0HallA, CHANGE);
  attachInterrupt(HALL_WHEEL0_B_PIN, wheel0HallB, CHANGE);
  attachInterrupt(HALL_WHEEL1_A_PIN, wheel1HallA, CHANGE);
  attachInterrupt(HALL_WHEEL1_B_PIN, wheel1HallB, CHANGE);
  attachInterrupt(HALL_WHEEL2_A_PIN, wheel2HallA, CHANGE);
  attachInterrupt(HALL_WHEEL2_B_PIN, wheel2HallB, CHANGE);
  attachInterrupt(HALL_WHEEL3_A_PIN, wheel3HallA, CHANGE);
  attachInterrupt(HALL_WHEEL3_B_PIN, wheel3HallB, CHANGE);
}

void loop() {
  handleSerial();
  int nowMicros = micros();
  if (nowMicros - wheel0Micros > 10000) {
    if (wheel0Direction != 0) wheel0Direction = 0;
    if (wheel1Direction != 0) wheel1Direction = 0;
    if (wheel2Direction != 0) wheel2Direction = 0;
    if (wheel3Direction != 0) wheel3Direction = 0;
    // wheel0CurrentRPM = wheel0CountChange/120.0*100.0;
    // Serial.print("wheel0CountChange:");
    // Serial.print(wheel0CountChange);
    // Serial.print(",");
    // Serial.print("wheel0CurrentRPM:");
    // Serial.print(wheel0CurrentRPM);
    // Serial.print(",");
  }
  
  // setSpeed(wheel::fl, 250);
  // updateSpeed(wheel::fl);

  // setSpeed(wheel::fr, 250);
  // updateSpeed(wheel::fr);

  // setSpeed(wheel::bl, 250);
  // updateSpeed(wheel::bl);

  // setSpeed(wheel::br, 250);
  // updateSpeed(wheel::br);
  
  // wheel0CountChange = abs(abs(wheel0CountPrev) - abs(wheel0Count));
  
  // Serial.print("BR:");
  // Serial.print(wheel0Count);
  // Serial.print(",");
  // Serial.print("FR:");
  // Serial.print(wheel1Count);
  // Serial.print(",");
  // Serial.print("BL:");
  // Serial.print(wheel2Count);
  // Serial.print(",");
  // Serial.print("FL:");
  // Serial.print(wheel3Count);
  
  Serial.print(curSpeeds[0]);
  Serial.print(", ");
  Serial.print(curSpeeds[1]);
  Serial.print(", ");
  Serial.print(curSpeeds[2]);
  Serial.print(", ");
  Serial.print(curSpeeds[3]);
  Serial.print(", ");
  Serial.print(curSpeeds[4]);
  Serial.print(", ");
  Serial.print(curSpeeds[5]);
  Serial.print(", ");
  Serial.print(curSpeeds[6]);
  Serial.print(", ");
  Serial.print(curSpeeds[7]);

  Serial.println("");
  
}
