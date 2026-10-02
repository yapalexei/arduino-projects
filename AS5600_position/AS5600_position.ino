//
//    FILE: AS5600_position.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/AS5600
//
//  Examples may use AS5600 or AS5600L devices.
//  Check if your sensor matches the one used in the example.
//  Optionally adjust the code.


#include "AS5600.h"


AS5600 as5600;   //  use default Wire


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);

  Wire.begin();

  as5600.begin(2);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.

  Serial.println(as5600.getAddress());

  if (!as5600.detectMagnet()) {
    Serial.println("place magnet over sensor");
  }
  //  as5600.setAddress(0x40);  //  AS5600L only

  int b = as5600.isConnected();
  Serial.print("Connect: ");
  Serial.println(b);

  delay(1000);
}

int position = 0;
int prevPosition = 0;
int rotations = 0;
int period = 60; // ms
float minutes_pp = period / 1000.0 / 60.0;
const int positionsPR = 4000.0;
void loop()
{
  static uint32_t lastTime = 0;

  //  set initial position
  position = as5600.getCumulativePosition();

  //  update every 100 ms
  //  should be enough up to ~200 RPM
  rotations = as5600.getRevolutions();
  if (millis() - lastTime >= period)
  {
    lastTime = millis();
    Serial.printf("%.1f\t%.1f\t", (float) (position - prevPosition)/positionsPR, (float) (position - prevPosition)/positionsPR/minutes_pp);
    Serial.print(position);
    Serial.print("\t");
    Serial.println(rotations);
    prevPosition = position;
  }

  //  just to show how reset can be used
  if (rotations >= 1000 || rotations <= -1001)
  {
    as5600.resetPosition();
    prevPosition = 0;
  }
}


//  -- END OF FILE --
