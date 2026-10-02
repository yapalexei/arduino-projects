#include <Adafruit_ADS1X15.h>
#include <stdlib.h>

// The ratio is exactly 1000:1. So 1 amp passing through the center hole will induce a current of 1 mliamp in the secondary.
// If you connect a 10 Ohm resistor across the secondary., you would get 10 .mllivolts across the resistor.
// 100 amps would produce 1 volt. A 1/4 watt resistor is big enough.

Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

const float multiplier = 0.1875F;
// 16bit resolution === 65,536 points

void setup(void) {
  // Serial.begin(19200);
  // Serial.begin(115200);
  // Serial.begin(250000);
  Serial.begin(460800);

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);  // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV per sample  32,768 +/-
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  // ads.setDataRate(RATE_ADS1115_250SPS);
  // ads.setDataRate(RATE_ADS1115_475SPS);
  ads.setDataRate(RATE_ADS1115_860SPS);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1)
      ;
  }
  delay(100);
}
float vPointsTop[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
float vPoints[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
float vPointsBottom[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

float vAverageTop = 0.0;
float vAverage = 0.0;
float vAverageBottom = 0.0;

float periodTotalTop = 0.0;
float periodTotal = 0.0;
float periodTotalBottom = 0.0;

float voltagePP = 0.0;
int WINDOW_SIZE = 1;
int curIndex = 0;

char buffer[10] = "";
String rows = "";
int decimals = 3;
void loop(void) {
  int currentMillis = millis();
  // int16_t count = ads.readADC_SingleEnded(0);
  int16_t count = ads.readADC_Differential_0_1();
  float voltage = (count * multiplier);
  // float voltage = ads.computeVolts(count); // same as above. But maybe more accurate?
  
  // VBoth
  vPoints[curIndex] = voltage;
  // Vmax
  vPointsTop[curIndex] = voltage > 0 ? voltage : 0;
  // Vmin - seems too small so ignoring
  vPointsBottom[curIndex] = voltage < 0 ? voltage : 0;

  periodTotalTop = 0.0;
  periodTotal = 0.0;
  periodTotalBottom = 0.0;
  rows += "T:"; rows += "300,";
  rows += "B:"; rows += "-200,";

  // sum values for this iteration WINDOW_SIZE
  if (WINDOW_SIZE > 1) {
    for (int i = curIndex; i != (curIndex > 0 ? curIndex - 1 : WINDOW_SIZE - 1); i = (i + 1) % WINDOW_SIZE) {
      periodTotal += vPoints[i];
      periodTotalTop += vPointsTop[i];
      periodTotalBottom += vPointsBottom[i];
    }
    // divide sums to get average for this iteration WINDOW_SIZE
    vAverageBottom = periodTotalBottom / (float)WINDOW_SIZE;
    rows += "mvB:"; dtostrf(vAverageBottom, 4, decimals, buffer); rows += buffer; rows += ",";
    
    vAverageTop = periodTotalTop / (float)WINDOW_SIZE;
    rows += "mvT:"; dtostrf(vAverageTop, 4, decimals, buffer); rows += buffer; rows += ",";
    
    vAverage = periodTotal / (float)WINDOW_SIZE;
    rows += "avg:"; dtostrf(vAverage, 4, decimals, buffer); rows += buffer; rows += ",";

    voltagePP = vAverageTop - vAverageBottom;
    rows += "mVpp:"; dtostrf(voltagePP, 4, decimals, buffer); rows += buffer; rows += ",";
  } else {
    vAverageTop = vPointsTop[0];
  }

  // rows += "A:"; dtostrf(voltagePP, 4, decimals, buffer); rows += buffer; rows += ",";
  rows += "mvT:"; dtostrf(vAverageTop, 4, decimals, buffer); rows += buffer; rows += ",";
  rows += "mV:"; dtostrf(voltage, 4, decimals, buffer); rows += buffer; rows += ",";
  rows += "count:"; rows += count;
  rows += "\n";


  curIndex += 1;
  if (currentMillis % 16) {
  // if (curIndex == WINDOW_SIZE) {
    curIndex = 0;
    Serial.print(rows);
    rows = "";
  }
  delayMicroseconds(16666); // 60fps;
  // delayMicroseconds(8333); // 120fps
  // delayMicroseconds(4167); // 240fps
  // delay(16);
}


