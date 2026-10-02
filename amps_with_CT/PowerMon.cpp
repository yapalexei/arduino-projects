#include "esp32-hal.h"
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include "PowerMon.h"
#include <Wire.h>

Adafruit_ADS1015 ads1;

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

constexpr int READY_PIN = 2;

volatile bool new_data = false;
void IRAM_ATTR NewDataReadyISR() {
  new_data = true;
}

PowerMon::PowerMon() {
  this->ads1 = ads1;
}

boolean PowerMon::init(onEvent cb) {
  onEventCb = cb;
  boolean isFailed = false;
  pinMode(READY_PIN, INPUT);
  onEventCb(INIT_START, "PowerMon v0.1", this);
  onEventCb(INIT, F("adc-1:"), this);
  if (!ads1.begin(0x48, &Wire)) {
    onEventCb(ADC_FAILED, F("failed to start\n"), this);
    isFailed = true;
  } else {
    onEventCb(INIT, F("connected\n"), this);
  }
  
  Wire.setClock(500000);
  // Wire.setClock(400000);
  attachInterrupt(digitalPinToInterrupt(READY_PIN), NewDataReadyISR, FALLING);

  ads1.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, /*continuous=*/true);
  // ads1.setGain(GAIN_TWOTHIRDS);  // gain +/- 6.144V  1 bit = 0.1875mV (default) 0.0001875
  ads1.setGain(GAIN_FOUR);  // gain +/- 6.144V  1 bit = 0.1875mV (default) 0.0001875
  ads1.setDataRate(RATE_ADS1015_3300SPS);
  return isFailed;
}

float PowerMon::getP1() { return amps1 * VAC; }
float PowerMon::getA1() { return amps1; }
float PowerMon::getmV1() { return mV1; }
float PowerMon::getP2() { return amps2 * VAC; }
float PowerMon::getA2() { return amps2; }
float PowerMon::getmV2() { return mV2; }

boolean PowerMon::hasNewData() {
  return new_data;
}

int16_t results;
void PowerMon::loop() {
  if (!new_data) {
    return;
  }
  // if (!ads1.conversionComplete()) {
  //   return;
  // }
  
  // read from ADC 1 - NOTE: setting mV2 to read from 0_1 so the input terminals line up physically
  results = ads1.getLastConversionResults();
  mV1 = ads1.computeVolts(results);

  // ads1.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, /*continuous=*/true);
  // ads1.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/false);
  delayMicroseconds(303);
  // mV2 = 0;
  // mV1 = ads1.computeVolts(ads1.readADC_Differential_0_1())*1000;
  // mV2 = ads1.computeVolts(ads1.readADC_Differential_2_3())*1000;

  // the rest of the logic applies the RMS algorithm for each channel (1-4).
  // vPoints1[frame] = abs(mV1) > 0.0005 ? sq(mV1) : 0;
  // vPoints2[frame] = abs(mV2) > 0.0005 ? sq(mV2) : 0;
  
  vPoints1[frame] = sq(mV1) * 100;
  // vPoints2[frame] = sq(mV2);

  if ((frame + 1) == (WINDOW_SIZE)) {
    mVSum1 = 0.0;
    // mVSum2 = 0.0;

    for (int i = frame; i != (frame > 0 ? frame - 1 : WINDOW_SIZE - 1); i = (i + 1) % WINDOW_SIZE) {
      mVSum1 += vPoints1[i];
      // mVSum2 += vPoints2[i];
    }

    mVAvg1 = mVSum1 / WINDOW_SIZE;
    // mVAvg2 = mVSum2 / WINDOW_SIZE;

    amps1 = mVAvg1 > 0 ? sqrt(mVAvg1) : 0;
    // amps2 = mVAvg2 > 0 ? sqrt(mVAvg2) : 0;

    // onEventCb(READ, F("new value"), this);
    // valHash = millis();
  }

  frame = (frame + 1) % WINDOW_SIZE;
  
}


int PowerMon::getValueHash() {
  return valHash;
}

String PowerMon::toString() {
  tostring = "mV1:"; tostring += mV1; tostring += ",";
  // tostring += "mV2:"; tostring += mV2; tostring += ",";
  tostring += "A1:"; tostring += amps1; tostring += ",";
  tostring += "P1:"; tostring += (amps1 * VAC); tostring += ",";
  tostring += "min:0,";
  tostring += "DRAM:"; tostring += ESP.getFreeHeap(); tostring += "\n";
  return tostring;
}
