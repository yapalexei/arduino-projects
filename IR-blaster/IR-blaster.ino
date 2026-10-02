#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

bool LED_STATE = false;
// const uint16_t LED_PIN = LED_BUILTIN;
const uint16_t LED_PIN = 12;
const uint16_t kRecvPin = 4;
const uint16_t kIrLed = 15;
const uint16_t address = 0x4; // LG TV
const uint16_t powerOffCommand = 0x8; // LG TV
const uint16_t BTN_PIN = 13;
int BTN_STATE = 0;

IRrecv irrecv(kRecvPin);
IRsend irsend(kIrLed);

decode_results results; // for capturing other codes

const char* ssid = ""; // fill me with your own
const char* password = ""; // fill me with your own

const long utcOffsetInSeconds = -28800;
const long updateIntervalInMs = 60000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, updateIntervalInMs);
int sHour = 8; // 8:30am
int sMinute = 30;
int eHour = 19; // 7:30pm
int eMinute = 30;

void checkWifiStatus() {
  int status = WiFi.status();
    switch (status) {
      case WL_IDLE_STATUS:
        Serial.println("WL_IDLE_STATUS");
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("WL_NO_SSID_AVAIL");
        break;
      case WL_SCAN_COMPLETED:
        Serial.println("WL_SCAN_COMPLETED");
        break;
      case WL_CONNECTED:
        Serial.println("WL_CONNECTED");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("WL_CONNECT_FAILED");
        break;
      case WL_CONNECTION_LOST:
        Serial.println("WL_CONNECTION_LOST");
        break;
      case WL_WRONG_PASSWORD:
        Serial.println("WL_WRONG_PASSWORD");
        break;
      case WL_DISCONNECTED:
        Serial.println("WL_DISCONNECTED");
        break;
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin(); // Start the IR transmitter
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    checkWifiStatus();
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
  }

  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);
  digitalWrite(LED_PIN, HIGH);
  delay(100);

  timeClient.begin();
  timeClient.update();
  // while (!timeClient.isTimeSet()) {
  //   Serial.println("waiting to get and set current time.");
  // }
  // Serial.print("The time is: ");
  // Serial.println(timeClient.getFormattedTime());
  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("Connection Failed! Rebooting...");
  //   delay(5000);
  //   ESP.restart();
  // }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("IR-blaster");

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
int hour = 0;
int minute = 0;
bool isToggled = false;
int toggleTime = 0;
void loop() {
  ArduinoOTA.handle();
  hour = timeClient.getHours();
  minute = timeClient.getMinutes();
  BTN_STATE = digitalRead(BTN_PIN);
  if (BTN_STATE == LOW) {
    digitalWrite(LED_PIN, HIGH);
    irrecv.disableIRIn();
    irsend.sendNEC(irsend.encodeNEC(address, powerOffCommand));

    Serial.print("sending ir PWR code");
    irrecv.enableIRIn();
  } else {
    // timeClient.update();
    // Serial.print(timeClient.getFormattedTime());
    // Serial.print(" - H: ");
    // Serial.print(timeClient.getHours());
    // Serial.print(" M: ");
    // Serial.println(timeClient.getMinutes());
    if(timeClient.isTimeSet()) {
      if (isToggled && toggleTime != (hour*100) + minute) {
        // reset isToggled
        isToggled = false;
      }
      if ((hour == sHour && minute == sMinute || hour == eHour && minute == eMinute) && !isToggled) {
        isToggled = true;
        toggleTime = (hour*100) + minute;
        irsend.sendNEC(irsend.encodeNEC(address, powerOffCommand));
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
      }
    }
    // if (irrecv.decode(&results)) {
    //   digitalWrite(LED_PIN, LOW);
    //   serialPrintUint64(results.value, HEX);
    //   Serial.println("");
    //   irrecv.resume();  // Receive the next value
    // }
  }
  delay(300);
  digitalWrite(LED_PIN, LOW);
}