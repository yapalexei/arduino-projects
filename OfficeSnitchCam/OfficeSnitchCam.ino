/**
 * Send image from camera to Telegram
 *
 * BE SURE TO SET "TOOLS > CORE DEBUG LEVEL = INFO"
 * to turn on debug messages
 */
// WiFi credentials
#define WIFI_SSID ""
#define WIFI_PASS ""

// replace with your bot token and chat id
#define TELEGRAM_TOKEN ""
#define TELEGRAM_CHAT ""

#define PIR_PIN 16

#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/telegram.h>

using eloq::camera;
using eloq::wifi;
using eloq::telegram;

int pirState = 0;  // current state of the pir
const int stayOnPeriod = 10000; // ms
boolean isThereMovement = false;
int curTime = 0;
int timeSinceMovement = 0;


void checkState();

void setup() {
  delay(3000);
  Serial.begin(115200);
  Serial.println("___TELEGRAM IMAGE___");

  // camera settings
  // replace with your own model!
  camera.pinout.aithinker();
  camera.brownout.disable();
  camera.resolution.vga();
  camera.quality.high();

  // init camera
  while (!camera.begin().isOk())
      Serial.println(camera.exception.toString());
  Serial.println("Camera OK");

  // connect to WiFi
  while (!wifi.connect().isOk())
    Serial.println(wifi.exception.toString());
  Serial.println("Wifi OK");

  // connect to Telegram API
  while (!telegram.begin().isOk())
    Serial.println(telegram.exception.toString());
  Serial.println("Telegram OK");
  
  pinMode(PIR_PIN, INPUT);
  
}

void send() {
  camera_fb_t *frame = esp_camera_fb_get();
  if (!frame) return;
  delay(100);
  // if (!camera.capture().isOk()) {
  //     Serial.println("failed: camera.capture");
  //     Serial.println(camera.exception.toString());
  //     return;
  // }
  Serial.printf("frame buffer size: %d\n", frame->len);
  if (frame->len < 100) {
    Serial.printf("frame buffer is not right: %d", frame->len);
    return;
  }
  Serial.println("Motion detected!");
  if (telegram.to(TELEGRAM_CHAT).send(frame).isOk()) {
    Serial.println("Photo sent to Telegram");
  } else {
    Serial.println("some exception occured");
    Serial.println(telegram.exception.toString());
  }
}

void checkState() {
  curTime = millis();
  pirState = digitalRead(PIR_PIN) == HIGH;
  if (pirState && !isThereMovement) {
    isThereMovement = true;
    timeSinceMovement = curTime;
    send();
  }

  // reset after stayOnPeriod
  if (curTime - timeSinceMovement > stayOnPeriod) {
    isThereMovement = false;
  }

}

void loop() {
  checkState();
}
