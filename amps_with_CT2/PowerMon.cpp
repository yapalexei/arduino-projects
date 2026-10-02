#include "esp32-hal.h"
#include "ADS1X15.h"
#include "PowerMon.h"

ADS1015 ads1(0x48);

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

constexpr int READY_PIN = 2;
float PF = 0.6;

int16_t results;
float factor = 1;

volatile bool new_data = false;
void IRAM_ATTR NewDataReadyISR() {
  new_data = true;
}

PowerMon::PowerMon() {
  this->ads1 = ads1;
}

boolean PowerMon::init() {
  boolean isFailed = false;
  pinMode(READY_PIN, INPUT);

  ads1.begin();
  ads1.setWireClock(2000000);
  ads1.setMode(1);
  ads1.setDataRate(7);
  ads1.requestADC_Differential_0_1();
  
  return isFailed;
}

float PowerMon::getmV1() { return mV1; }
float PowerMon::getA1() { return amps1; }
float PowerMon::getP1() { return amps1 * VAC * PF; }

float PowerMon::getmV2() { return mV2; }
float PowerMon::getA2() { return amps2; }
float PowerMon::getP2() { return amps2 * VAC * PF; }

boolean PowerMon::hasNewData() {
  return new_data;
}

int ch = 0;
void PowerMon::loop() {
  if (!ads1.isReady()) return;

  if (ch == 0) {
    mV1 = ads1.toVoltage(ads1.getValue());
  }
  if (ch == 1) {
    mV2 = ads1.toVoltage(ads1.getValue());
  }
  
  vPoints1[frame] = sq(mV1);
  vPoints2[frame] = sq(mV2);

  // if ((frame + 1) == (WINDOW_SIZE)) {
    mVSum1 = 0.0;
    mVSum2 = 0.0;

    for (int i = frame; i != (frame > 0 ? frame - 1 : WINDOW_SIZE - 1); i = (i + 1) % WINDOW_SIZE) {
      mVSum1 += vPoints1[i];
      mVSum2 += vPoints2[i];
    }

    mVAvg1 = mVSum1 / WINDOW_SIZE;
    mVAvg2 = mVSum2 / WINDOW_SIZE;

    amps1 = mVAvg1 > 0 ? sqrt(mVAvg1) * 10 : 0;
    amps2 = mVAvg2 > 0 ? sqrt(mVAvg2) * 10 : 0;

  // }
  // delayMicroseconds(303);
  frame = (frame + 1) % WINDOW_SIZE;

  if (ch == 0) {
    ch = 1;
    ads1.requestADC_Differential_2_3();
  } else {
    ch = 0;
    ads1.requestADC_Differential_0_1();
  }
}


int PowerMon::getValueHash() {
  return valHash;
}

String PowerMon::toString() {
  tostring = "";
  tostring += ESP.getCpuFreqMHz(); tostring += ",";
  tostring += ESP.getFreePsram(); tostring += ",";
  tostring += ESP.getFreeHeap(); tostring += ",";
  tostring += this->getA1(); tostring += ",";
  tostring += this->getA2(); tostring += ",";
  tostring += this->getmV1(); tostring += ",";
  tostring += this->getmV2(); tostring += ",";
  tostring += this->getP1(); tostring += ",";
  tostring += this->getP2();

  return tostring;
}
