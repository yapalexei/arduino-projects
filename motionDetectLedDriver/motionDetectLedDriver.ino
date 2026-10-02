// this constant won't change:
const int buttonPin = 4;  // the pin that the pushbutton is attached to
const int ledPin = 2;     // the pin that the LED is attached to
const int bLedPin = 5;     // the pin that the LED is attached to

int pirState = 0;  // current state of the button
int brightness = 0;
const int steyOnPeriod = 10000; // ms
const int maxBrightness = 255;
const int minBrightness = 0;
boolean isThereMovement = false;
int curTime = 0;
int timeSinceMovement = 0;

void setup() {
  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);
  pinMode(bLedPin, OUTPUT);
  // initialize serial communication:
  
  Serial.begin(9600);
}

void incrementBrightness(int dir = 0) {
  brightness += dir;
  delay(5);
  analogWrite(ledPin, 255-brightness);
  analogWrite(bLedPin, brightness);
}

void checkStateAndSetBrightness() {
  curTime = millis();
  pirState = digitalRead(buttonPin) == HIGH;
  if (pirState) {
    isThereMovement = true;
    timeSinceMovement = curTime;
  }

  if (curTime - timeSinceMovement > steyOnPeriod) {
    isThereMovement = false;
  }

  if (isThereMovement) {
    if (brightness < maxBrightness) {
      incrementBrightness(1); 
    }
  } else {
    if (brightness > minBrightness) {
      incrementBrightness(-1);
    }
  }
  Serial.printf("%d\t%d\n", pirState, brightness);
}

void loop() {
  checkStateAndSetBrightness();
}
