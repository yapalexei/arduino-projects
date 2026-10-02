#include <WS2812FX.h>

#define LED_COUNT 4
#define LED_PIN 2

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(80);
  ws2812fx.setSpeed(500);
  ws2812fx.setMode(FX_MODE_SCAN);
  ws2812fx.setColor(10,205,205);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}