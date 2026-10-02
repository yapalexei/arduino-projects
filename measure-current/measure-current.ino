#include<ADS1115_WE.h> 
#include<Wire.h>
#define I2C_ADDRESS 0x48

ADS1115_WE adc = ADS1115_WE(I2C_ADDRESS);
int currentMillis = 0;
float maxVal = 0.0;
float minVal = 0.0;
float rawAverage = 0.0;
float periodTotal = 0.0;
float vPoints[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};


void setup() {
  Wire.begin();
  Serial.begin(250000);
  if(!adc.init()){
    Serial.println("ADS1115 not connected!");
  }
  adc.setVoltageRange_mV(ADS1115_RANGE_6144);
  // adc.setCompareChannels(ADS1115_COMP_0_GND);
  adc.setCompareChannels(ADS1115_COMP_0_1);
  adc.setConvRate(ADS1115_860_SPS);
  // adc.setConvRate(ADS1115_250_SPS);
}


void loop() {
  int currentMillis = millis();
  float voltage = readChannel(ADS1115_COMP_0_1);
  float maxValDelta = maxVal - voltage;

  if (voltage > maxVal) maxVal = voltage;
  if (voltage < minVal) minVal = voltage;

  // update min/max to values gradually
  if (currentMillis % 4 == 0) {
    maxVal = maxVal > 0.0 && maxVal > voltage ? maxVal - 1 : maxVal;
    minVal = minVal < 0.0 && minVal < voltage ? minVal + 1 : minVal;
  }

  // calc (16 point) rolling average of raw value
  vPoints[currentMillis % 16] = voltage;
  periodTotal = 0.0;
  for (int i = 0; i < 16; i++) {
    periodTotal += vPoints[i];
  }
  rawAverage = periodTotal / 16.0;
  // }

  // if (currentMillis % 2 == 0) {
    Serial.print("raw:");
    Serial.print(voltage);
    Serial.print(",");
    Serial.print("rawAverage:");
    Serial.print(rawAverage);
    Serial.print(",");
    Serial.print("max:");
    Serial.print(maxVal);
    Serial.print(",");
    Serial.print("min:");
    Serial.print(minVal);
    Serial.print(",");
    Serial.print("minMaxAvg:");
    Serial.print((maxVal + minVal) / 2.0);
    Serial.println("");
  // }
}


float readChannel(ADS1115_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){}
  voltage = adc.getResult_mV(); // alternative: getResult_mV for Millivolt
  return voltage;
}