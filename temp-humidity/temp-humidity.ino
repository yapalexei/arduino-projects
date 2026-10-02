/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers
 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98
 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSans9pt7b.h>

#define SSD1306_128_32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

int sleep_time_in_seconds = 5;

Adafruit_AHTX0 aht;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setContrast(Adafruit_SSD1306 *display, uint8_t contrast)
{
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(contrast);
}

//void testdrawtriangle(void) {
//  display.clearDisplay();
//
//  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=5) {
//    display.drawTriangle(
//      display.width()/2  , display.height()/2-i,
//      display.width()/2-i, display.height()/2+i,
//      display.width()/2+i, display.height()/2+i, SSD1306_WHITE);
//    display.display();
//    delay(1);
//  }
//
//  delay(2000);
//}
//
//void testfilltriangle(void) {
//  display.clearDisplay();
//
//  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=5) {
//    // The INVERSE color is used so triangles alternate white/black
//    display.fillTriangle(
//      display.width()/2  , display.height()/2-i,
//      display.width()/2-i, display.height()/2+i,
//      display.width()/2+i, display.height()/2+i, SSD1306_INVERSE);
//    display.display();
//    delay(1);
//  }
//
//  delay(2000);
//}
//
//void testdrawchar(void) {
//  display.clearDisplay();
//
//  display.setTextSize(1);      // Normal 1:1 pixel scale
//  display.setTextColor(SSD1306_WHITE); // Draw white text
//  display.setCursor(0, 0);     // Start at top-left corner
//  display.cp437(true);         // Use full 256 char 'Code Page 437' font
//
//  // Not all the characters will fit on the display. This is normal.
//  // Library will draw what it can and the rest will be clipped.
//  for(int16_t i=0; i<256; i++) {
//    if(i == '\n') display.write(' ');
//    else          display.write(i);
//  }
//
//  display.display();
//  delay(2000);
//}
//
//void testdrawstyles(void) {
//  display.clearDisplay();
//
//  display.setTextSize(1);             // Normal 1:1 pixel scale
//  display.setTextColor(SSD1306_WHITE);        // Draw white text
//  display.setCursor(0,0);             // Start at top-left corner
//  display.println(F("Hello, world!"));
//
//  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
//  display.println(3.141592);
//
//  display.setTextSize(2);             // Draw 2X-scale text
//  display.setTextColor(SSD1306_WHITE);
//  display.print(F("0x")); display.println(0xDEADBEEF, HEX);
//
//  display.display();
//  delay(2000);
//}
//
//void testscrolltext(void) {
//  display.clearDisplay();
//
//  display.setTextSize(2); // Draw 2X-scale text
//  display.setTextColor(SSD1306_WHITE);
//  display.setCursor(10, 0);
//  display.println(F("scroll"));
//  display.display();      // Show initial text
//  delay(100);
//
//  // Scroll in various directions, pausing in-between:
//  display.startscrollright(0x00, 0x0F);
//  delay(2000);
//  display.stopscroll();
//  delay(1000);
//  display.startscrollleft(0x00, 0x0F);
//  delay(2000);
//  display.stopscroll();
//  delay(1000);
//  display.startscrolldiagright(0x00, 0x07);
//  delay(2000);
//  display.startscrolldiagleft(0x00, 0x07);
//  delay(2000);
//  display.stopscroll();
//  delay(1000);
//}


void setup() {
  #if defined(ESP8266)
  WiFi.persistent(false);  //disable saving wifi config into SDK flash area
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();  //disable AP & station by calling "WiFi.mode(WIFI_OFF)" & put modem to sleep
  #endif
  Serial.begin(74880);
  Wire.begin(2, 0);
  if (!aht.begin(&Wire)) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  // Draw pixels in white
//  display.drawPixel(0, 0, SSD1306_WHITE);
//  display.drawPixel(127, 0, SSD1306_WHITE);
//  display.drawPixel(127, 31, SSD1306_WHITE);
//  display.drawPixel(0, 31, SSD1306_WHITE);
  display.display();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  setContrast(&display, 0);
}

void renderTemp(sensors_event_t humidity, sensors_event_t temp) {
  display.setCursor(20, 1);
  display.print((temp.temperature * 1.8) + 32); display.println(" F");
  display.setCursor(20, 16);
  display.print(humidity.relative_humidity); display.print(" %");
  display.display();
  display.dim(true);
  delay(5000);
  display.clearDisplay();
  display.display();
}

void loop() {
  #if defined(ESP8266)
  WiFi.persistent(false);  //disable saving wifi config into SDK flash area
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();  //disable AP & station by calling "WiFi.mode(WIFI_OFF)" & put modem to sleep
  #endif
  display.clearDisplay();
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  renderTemp(humidity, temp);

  ESP.deepSleep(5 * 1000000, WAKE_RF_DISABLED);
  delay(100);
}
