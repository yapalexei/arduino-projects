#include "PowerMon.h"

PowerMon pMon;
unsigned long curMillis = millis();
int count = 0;
String accum = "";

void setup() {
  Serial.begin(460800);
  pMon.init();
  // WiFi.mode(WIFI_OFF);
  curMillis = micros();

}
// (1000/60)/(1000/3300) === 55
int diff = 0;
int16_t fps = 0;
void loop() {
  
  curMillis = micros();
  
  pMon.loop();
  
  diff = micros() - curMillis;
  fps = diff > 0 ? 1000000/diff : 0;
  
  
  
  
  accum += "/*";
  accum += (int)fps; accum += ",";
  accum += pMon.getmV1(); accum += ",";
  accum += pMon.getmV2(); accum += ",";
  accum += pMon.getA1(); accum += ",";
  accum += pMon.getA2(); accum += ",";
  accum += pMon.getP1(); accum += ",";
  accum += pMon.getP2(); accum += ",";
  accum += ESP.getCpuFreqMHz(); accum += ",";
  accum += ESP.getFreeHeap(); accum += "";
  accum += "*/\n";
  
  
  // Serial.print(accum);
  if (count % 100 == 0) {
    Serial.print(accum);
    count = 0;
    accum = "";
  }
  
  count += 1;
  
  // delayMicroseconds(100); // equivalent to 55 samples per 1 AC cycle (that happens @ 60Hz).
  // delayMicroseconds(303); // equivalent to 55 samples per 1 AC cycle (that happens @ 60Hz).
}

