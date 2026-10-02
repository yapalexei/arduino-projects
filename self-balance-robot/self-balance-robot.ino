#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <stdarg.h>
#include <ESP8266WiFi.h>

#define SSD1306_NO_SPLASH
#define SERIAL_BAUD 19200

#define M0 0
#define M1 2

#define M2 12
#define M3 13

#define PINA 1
#define PINB 3
#define NUMPWM 2
#define PWM 100
#define BAL_ENABLE_PIN 14

Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

bool isSensorLoaded = false;
bool isOledAvailable = false;
bool printSerialInLoop = false;
bool printDisplayInLoop = true;
int pwm = 0;
bool prevDoBalance = false;
bool doBalance = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  WiFi.mode(WIFI_OFF);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  setupDisplay();
  setupBalanceSensor();
  
  pinMode(D0, INPUT);
  
  pinMode(BAL_ENABLE_PIN, OUTPUT);
  digitalWrite(BAL_ENABLE_PIN, LOW);

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(M3, OUTPUT);
  

  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  digitalWrite(M2, LOW);
  digitalWrite(M3, LOW);
}

void onBalanceEnableChange() {
  doBalance = !doBalance;
  if (doBalance) {
    digitalWrite(BAL_ENABLE_PIN, HIGH);
  } else {
    digitalWrite(BAL_ENABLE_PIN, LOW);
  }
  Serialprintln("balance: %s", doBalance ? "On" : "Off");
}

void loop() {
  updateBtnState(D0, onBalanceEnableChange);
  checkInput();
  if (isOledAvailable && !printDisplayInLoop) {
    display.clearDisplay();
    display.display();
  }
  if (!isSensorLoaded) return;
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if (printSerialInLoop) {
    Serialprintln("AccX:%f,AccY:%f,AccZ:%f,GyroX:%f,GyroY:%f,GyroZ:%f", a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z);
  }

  if (doBalance) {
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    digitalWrite(M2, LOW);
    digitalWrite(M3, LOW);

    long y = (a.acceleration.y - 6.0) * 150;
    // int val = map(y, -5.0, 5.0, 0, 255);
    int val = y / 2;
    Serialprintln("val: %d y: %d", val, y);
    if (y > 25) {
      analogWrite(M0, val);
      // digitalWrite(M1, LOW);
      analogWrite(M3, val);
      // digitalWrite(M2, LOW);
    } else if (y < -25) {
      // digitalWrite(M0, LOW);
      analogWrite(M1, -val);
      // digitalWrite(M3, LOW);
      analogWrite(M2, -val);
    }
  }

  if (isOledAvailable && printDisplayInLoop) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Accel - m/s^2");
    dPrintln("%f, %f, %f", a.acceleration.x, a.acceleration.y, a.acceleration.z);
    display.println("Gyro - rps");
    dPrintln("%f, %f, %f", g.gyro.x, g.gyro.y, g.gyro.z);
    display.display();
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


void dPrintln(const char* input...) {
  va_list args;
  va_start(args, input);
  for (const char* i = input; *i != 0; ++i) {
    if (*i != '%') {
      display.print(*i);
      continue;
    }
    switch (*(++i)) {
      case '%': display.print('%'); break;
      case 's': display.print(va_arg(args, char*)); break;
      case 'd': display.print(va_arg(args, int), DEC); break;
      case 'b': display.print(va_arg(args, int), BIN); break;
      case 'o': display.print(va_arg(args, int), OCT); break;
      case 'x': display.print(va_arg(args, int), HEX); break;
      case 'f': display.print(va_arg(args, double), 2); break;
    }
  }
  display.println();
  va_end(args);
}

void Serialprintln(const char* input...) {
  va_list args;
  va_start(args, input);
  for (const char* i = input; *i != 0; ++i) {
    if (*i != '%') {
      Serial.print(*i);
      continue;
    }
    switch (*(++i)) {
      case '%': Serial.print('%'); break;
      case 's': Serial.print(va_arg(args, char*)); break;
      case 'd': Serial.print(va_arg(args, int), DEC); break;
      case 'b': Serial.print(va_arg(args, int), BIN); break;
      case 'o': Serial.print(va_arg(args, int), OCT); break;
      case 'x': Serial.print(va_arg(args, int), HEX); break;
      case 'f': Serial.print(va_arg(args, double), 2); break;
    }
  }
  Serial.println();
  va_end(args);
}

bool setupDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }
  display.display();
  delay(50);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);
  display.println("Found display!");
  display.display();
  delay(1000);
  isOledAvailable = true;
  return true;
}

bool setupBalanceSensor() {
  if (isOledAvailable) {
    display.println("Loading sensor...");
    display.display();
    delay(500);
  }
  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    if (isOledAvailable) {
      display.println("Sensor init failed");
      display.display();
    }
    delay(100);
    return false;
  }
  Serial.println("Found MPU-6050 sensor");
  if (isOledAvailable) {
    display.println("Found MPU-6050 sensor");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Starting program...");
    display.display();
    delay(1000);
  }
  isSensorLoaded = true;
  return true;
}

void checkInput() {
  if (Serial.available())  // if there is data comming
  {
    String command = Serial.readStringUntil('\n');  // read string until meet newline character

    if (command == "led") {
      if (digitalRead(LED_BUILTIN) == HIGH) {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("LED is turned ON");
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED is turned OFF");
      }
    } else if (command == "serial") {
      printSerialInLoop = !printSerialInLoop;
      Serialprintln("Serial: %s", printSerialInLoop ? "ON" : "OFF");
    } else if (command == "display") {
      printDisplayInLoop = !printDisplayInLoop;
      Serialprintln("display: %s", printDisplayInLoop ? "ON" : "OFF");
    } else if (command == "sensor-setup") {
      if (isSensorLoaded) {
        Serial.println("Sensor already setup. ignoring command.");
      } else {
        setupBalanceSensor();
      }
    } else if (command == "sensor-reset") {
      mpu.reset();
    } else if (command == "balance") {
      onBalanceEnableChange();
    } else if (command.indexOf("pwm") == 0 && command.length() >= 5) {
      String motor = command.substring(3, 4);
      String num = command.substring(4, command.indexOf("\n"));
      int mNum;
      if (motor) {
        mNum = motor.toInt();
      }
      if (num) {
        pwm = num.toInt();
      }
      Serialprintln("motor: %d pwm: %d", mNum, pwm);
      digitalWrite(M0, LOW);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      digitalWrite(M3, LOW);
      if (mNum == 0) {
        analogWrite(M0, pwm);
      } else if (mNum == 1) {
        analogWrite(M1, pwm);
      } else if (mNum == 2) {
        analogWrite(M2, pwm);
      } else if (mNum == 3) {
        analogWrite(M3, pwm);
      }
    } else if (command == "reboot") {
      Serial.println("rebooting ESP8266...");
      ESP.restart();
    } else if (command == "help") {
      Serial.println("Available commands:");
      Serial.println("'pwm[0|1|2|3][0-255]'  applies pwm to motor [motor #][pwm value]");
      Serial.println("'led'  \t\tturns LED_BUILTIN on/off");
      Serial.println("'serial'\t prints out sensor data");
      Serial.println("'display'\t prints out sensor data to OLED screen");
      Serial.println("'reboot'\t (soft) restarts the ESP8266 module");
      Serial.println("'help'  \t\tprints this help doc");
    }
  }
}
