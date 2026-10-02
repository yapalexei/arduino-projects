#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <AwesomeClickButton.h>
#include "one_button.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define TOTAL_V 10

SoftwareSerial ESPserial(2, 3); // RX | TX
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float VOLT_POINT = 12.8; // 128/15 == 8.53
int TEN_V_POINTS = 316;
float vMarks[] = {3.3, 5.0};
int vMSize = sizeof(vMarks) / sizeof(vMarks[0]);
int ppv = 128/TOTAL_V;
int prevRaw = 0;

void setup() {
  Serial.begin(9600);
  ESPserial.begin(115200);
  ESPserial.println("AT+IPR=9600");
  delay(500);
  ESPserial.end();
  // // Start the software serial for communication with the ESP8266
  ESPserial.begin(9600);

  Serial.println("Ready");
  ESPserial.println("AT+GMR");

  
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.cp437(true);
  display.setCursor(0,0);

  pinMode(A0, INPUT);

  pinMode(D6, INPUT);
}

void drawBarGraph(float val) {
  display.drawRect(0, 28, 127, 4, SSD1306_WHITE);
  
  int res = val * ppv;
  display.fillRect(0, 28, res, 4, SSD1306_WHITE);
  display.setTextSize(1);
    
  for (int counter = 0; counter < vMSize; ++counter) {
    int point = ppv*vMarks[counter];
    display.drawFastVLine(point, 27, 4, SSD1306_WHITE);
    if (counter > 0) {
      display.setCursor(point - 2, 18);
      display.print(vMarks[counter], 0);
    } else {
      display.setCursor(point - 9, 18);
      display.print(vMarks[counter], 1);
    }
  }
}

bool btnState = false;
bool btnStatePrv = false;

void updateBtnState(const uint8_t pin, void (*onStateChange) ()) {
  int btnPinState = digitalRead(pin);
  if (btnPinState == LOW && btnState == btnStatePrv) {
    btnState = !btnState;
    onStateChange();
  }
  if (btnPinState == HIGH) {
    btnStatePrv = btnState;
  }
}

void invertDisplay() {
  display.invertDisplay(btnState);
}

void loop() {

  if (btnState) {
    delay(30);
    Serial.println("btn state active");
  }

  updateBtnState(D6, invertDisplay);  

  int raw = analogRead(A0) - 17;
  if (raw < 0) raw = 0;
  raw = (prevRaw + raw) / 2;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  float val = ((raw*100) / TEN_V_POINTS)/10.0;
  String vPrefix = "V";
  vPrefix.concat(val);
  display.print(val, 1);
  display.print("V ");
  display.setTextSize(1);
  String rPrefix = "("; rPrefix.concat(raw); rPrefix.concat(")");
  display.print(rPrefix);

  drawBarGraph(val);

  display.display();
  delay(100);
  prevRaw = raw;
}
