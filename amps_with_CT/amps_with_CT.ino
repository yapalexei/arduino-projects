#include "PowerMon.h"
// #if defined(ESP32)
// #include <WiFiMulti.h>
// WiFiMulti wifiMulti;
// #define DEVICE "ESP32"
// #elif defined(ESP8266)
// #include <ESP8266WiFiMulti.h>
// ESP8266WiFiMulti wifiMulti;
// #define DEVICE "ESP8266"
// #endif

PowerMon pMon;
unsigned long curMillis = millis();
int count = 0;
String accum = "";

void pmOnEventCb(PMEvent eventType, String message, PowerMon *pm) {
  switch (eventType)
  {
  case INIT_START:
    Serial.println(message);
    break;
  case INIT:
    Serial.println(message);
    break;
  case ADC_FAILED:
    Serial.println(message);
    break;
  case INIT_FINISH:
    Serial.println(message);
    break;
  case READ:
    // Serial.printf("mV1:%.1f,mV2:%.1f,A1:%.1f,P1:%.1f,fps:%.1f\n", pm->getmV1(), pm->getmV2(), pm->getA1(), pm->getP1(), pm->getA2(), pm->getP2(), 1000.0/(millis() - curMillis));
    // Serial.print(pMon.toString());
    // curMillis = millis();
    break;
  case DONE:
    /* code */
    break;

  default:
    break;
  }
}

void setup() {
  Serial.begin(460800);
  pMon.init(pmOnEventCb);
  // WiFi.mode(WIFI_OFF);
  curMillis = micros();

}
// (1000/60)/(1000/3300) === 55
int diff = 0;
float fps = 0;
void loop() {
  pMon.loop();
  
  // if (!pMon.hasNewData()) return;
  
  diff = micros() - curMillis;
  fps = diff > 0 ? 1000000/diff : 0;

  accum += "fps:";
  accum += fps;
  accum += ",";
  accum += pMon.toString();
  
  // if (count % 100 == 0) {
    Serial.print(accum);
    count = 0;
    accum = "";
  // }
  
  count += 1;
  curMillis = micros();
  // delayMicroseconds(16667/24); // 24.167 samples per cycle
  
}

