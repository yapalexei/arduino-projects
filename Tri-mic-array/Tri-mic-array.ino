
#include <Arduino.h>

#define MIC_A_PIN 34
#define MIC_B_PIN 35
#define MIC_C_PIN 39

#define MIC_RMS_SIZE 10

int micAValue = 0;
int micBValue = 0;
int micCValue = 0;

void setup() {
  Serial.begin(115200);
}

float rmsA = 0;
float rmsB = 0;
float rmsC = 0;

void loop() {
  micAValue = analogRead(MIC_A_PIN);
  micBValue = analogRead(MIC_B_PIN);
  micCValue = analogRead(MIC_C_PIN);
  Serial.printf(">micA:%d\n", micAValue);
  Serial.printf(">micB:%d\n", micBValue);
  Serial.printf(">micC:%d\n", micCValue);
  // Serial.printf(">micAB:%d\n", micAValue - micBValue);
  // Serial.printf(">micAC:%d\n", micAValue - micCValue);
  // Serial.printf(">micBC:%d\n", micBValue - micCValue);
  // // calc rms
  // rmsA = sqrt((micAValue * micAValue) / MIC_RMS_SIZE);
  // rmsB = sqrt((micBValue * micBValue) / MIC_RMS_SIZE);
  // rmsC = sqrt((micCValue * micCValue) / MIC_RMS_SIZE);
  // Serial.printf(">rmsA:%.2f\n", rmsA);
  // Serial.printf(">rmsB:%.2f\n", rmsB);
  // Serial.printf(">rmsC:%.2f\n", rmsC);
  delay(1);
}
