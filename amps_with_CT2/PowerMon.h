#ifndef POWER_MON_H
#define POWER_MON_H
#include "ADS1X15.h"
#include <Arduino.h>
enum PMEvent {
  INIT_START,
  INIT,
  INIT_FINISH,
  ADC_FAILED,
  READ_START,
  READ,
  READ_END,
  DONE,
};

class PowerMon {
  static const int WINDOW_SIZE = 60 * 2;
  static const int VAC = 120;
  using onEvent = void (*) (PMEvent event, String message, PowerMon *pm);
  private:
    int frame = 0;
    int valHash = 0;
    String tostring;
    ADS1015 ads1;
    ADS1015 ads2;
    float vPoints1[WINDOW_SIZE];
    float vPoints2[WINDOW_SIZE];
    float vPoints3[WINDOW_SIZE];

    float mVAvg1 = 0.0;
    float mVAvg2 = 0.0;
    float mVAvg3 = 0.0;

    float mVSum1 = 0.0;
    float mVSum2 = 0.0;
    float mVSum3 = 0.0;

    float amps1 = 0.0;
    float amps2 = 0.0;
    float amps3 = 0.0;

    float mV1 = 0.0;
    float mV2 = 0.0;
    float mV3 = 0.0;

  public:
    PowerMon();
    boolean init();
    int getValueHash();
    float getP1();
    float getA1();
    float getmV1();
    float getP2();
    float getA2();
    float getmV2();
    float getP3();
    float getA3();
    float getmV3();
    void loop();
    void loopV2();
    String toString();
    boolean hasNewData();
};

#endif