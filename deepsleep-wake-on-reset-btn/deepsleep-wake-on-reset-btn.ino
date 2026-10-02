#include "Arduino.h"
#include <ESP8266WiFi.h>
// Required for LIGHT_SLEEP_T delay mode
extern "C" {
#include "user_interface.h"
}
 
const char* ssid = ""; // fill me with your own
const char* password = ""; // fill me with your own
 
//The setup function is called once at startup of the sketch
void setup() {
  delay(5000); // do some stuff before going to sleep

  Serial.begin(115200);
  while(!Serial) { }

  Serial.println();
  Serial.println("Start device in normal mode!");
 
  WiFi.mode(WIFI_STA);
 
  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
 
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(LED_BUILTIN, OUTPUT);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
  digitalWrite(LED_BUILTIN, HIGH);
  delay(150);
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(150);
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(150);
  digitalWrite(LED_BUILTIN, LOW);
  delay(150);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Deep sleep mode for 10 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  delay(5000);
  Serial.println("I'm awake, but I'm going into deep sleep mode for 10 seconds");
  ESP.deepSleep(10e6, RF_DISABLED);
}

void loop() {
  
}