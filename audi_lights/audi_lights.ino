#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

// Wi-Fi connection parameters
const char * wifi_ssid = ""; // fill me with your own
const char * wifi_password = ""; // fill me with your own
const char * hostName = "audiLightshow";

const int shutoffPeriod = 1800000; // 30 minutes

// pins
const int powerButtonPin = 0;
const int headlightButtonPin = 13;
const int breaklightButtonPin = 14;
const int button1Pin = 13;
const int button2Pin = 14;

const int headlightLeftLEDPin = 4;
const int headlightRightLEDPin = 5;
const int cabinlightLEDPin = 16;

const int breaklightLEDsPin = 12;
const int otherLEDPin2 = 15; // not used as led light pin anymore.

const int wifiConnectionStatusLedPin = 12;

const int btnPins[] = {
  headlightButtonPin,
  breaklightButtonPin,
  powerButtonPin,
};
const int btnPinCount = sizeof(btnPins)/sizeof(int);

const int ledPins[] = {
  headlightLeftLEDPin,
  headlightRightLEDPin,
  breaklightLEDsPin,
  cabinlightLEDPin,
};
const int ledPinCount = sizeof(ledPins)/sizeof(int);

int offSequence[5] = {LOW, LOW, LOW, LOW, LOW};
int sequenceFrames[][ledPinCount] = {
  { HIGH, HIGH, HIGH, HIGH },
  { HIGH, LOW, LOW, LOW },
  { LOW, HIGH, LOW, LOW },
  { LOW, LOW, HIGH, LOW },
  { LOW, LOW, LOW, HIGH },
  { LOW, LOW, LOW, LOW }
};
int sequences = sizeof(sequenceFrames)/sizeof(int);

int progressLights[5][ledPinCount] = {
  { LOW, HIGH, HIGH, HIGH },
  { LOW, LOW, HIGH, HIGH },
  { LOW, LOW, LOW, HIGH },
  { LOW, LOW, LOW, LOW },
  { LOW, LOW, LOW, LOW }
};

///////////////////////////////
// ANIMATION SEQUENCES START //
///////////////////////////////

int animList[][6][ledPinCount] = {
  {
    {HIGH, LOW, LOW, LOW},
    {LOW, HIGH, LOW, LOW},
    {HIGH, LOW, LOW, LOW},
    {LOW, HIGH, LOW, LOW},
    {HIGH, LOW, LOW, LOW},
    {LOW, HIGH, LOW, LOW}
  },
  {
    {HIGH, HIGH, HIGH, LOW},
    {HIGH, HIGH, LOW, LOW},
    {HIGH, HIGH, LOW, LOW},
    {HIGH, HIGH, LOW, LOW},
    {HIGH, HIGH, LOW, LOW},
    {HIGH, HIGH, LOW, LOW}
  }
};
int animListSize = sizeof(animList)/sizeof(int);
int currentAnim = 0;
///////////////////////////////
//  ANIMATION SEQUENCES END  //
///////////////////////////////

// initial states
bool headlightButtonState = false;
bool prevHeadlightButtonState = false;
bool breaklightButtonState = false;
bool prevBreaklightButtonState = false;

bool btnStates[] = {
  false, // button 0 state
  false, // button 1 state
  false, // prev button 0 state
  false, // prev button 1 state
};

int lightSeq = 0;

int turnOnWifi = HIGH; // means off

void playSequenceFrame(int sequenceFrame[ledPinCount], boolean invert = false) {
  for (int i = 0; i < ledPinCount; i++) {
    if (invert) {
      digitalWrite(ledPins[i], !sequenceFrame[i]);
    } else {
      digitalWrite(ledPins[i], sequenceFrame[i]);
    }
  }
}

void flashTimes(int times, int del = 200, bool nextLoopDelay = true, int lightLEDPin = headlightLeftLEDPin) {
  for (int i = 0; i < times; i++) {
    digitalWrite(lightLEDPin, HIGH);
    delay(del);
    digitalWrite(lightLEDPin, LOW);
    if (nextLoopDelay) delay(del);
  }
}

bool checkBtnStateAndReturnBtnState(int btnIndex = 0) {
  int btnPinState = digitalRead(btnPins[btnIndex]);
  if (btnPinState == LOW && btnStates[btnIndex] == btnStates[btnIndex + 2]) {
    btnStates[btnIndex] = !btnStates[btnIndex];
    if (btnIndex == 1) {
      currentAnim = (currentAnim + 1) % animListSize;
    }
  }
  if (btnPinState) {
    btnStates[btnIndex + 2] = btnStates[btnIndex];
  }
  return btnStates[btnIndex];
}

void playLightSequence(int anim[][ledPinCount], int count = 1, int delayMulti = 1) {
  int frames = sizeof(anim)/sizeof(int);
  for (int loop = 0; loop < count; loop++) {
    for (int i = 0; i < frames; i++) {
      playSequenceFrame(anim[i]);
      delay(10 * delayMulti);
      playSequenceFrame(offSequence);
      delay(10 * delayMulti);
    }
  }
}

void wifiUpdateLightSequence() {
  playLightSequence(animList[0], 6, 3);
}

int connectionIndicatorLEDRate = 1000;
void connectToWiFi() {
  wifiUpdateLightSequence();
  Serial.println("Turning Wifi On...");
  Serial.printf("Connecting to '%s'\n", wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  connectionIndicatorLEDRate = connectionIndicatorLEDRate / 2;
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  // for some reason the below code doesn't actually work as I thought
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int val = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", val);
    playSequenceFrame(offSequence);
    if (val >= 90) {
      playSequenceFrame(progressLights[0]);
    }
    else if (val >= 80) {
      playSequenceFrame(progressLights[1]);
    }
    else if (val >= 60) {
      playSequenceFrame(progressLights[2]);
    }
    else if (val >= 40) {
      playSequenceFrame(progressLights[3]);
    }
    else if (val >= 20) {
      playSequenceFrame(progressLights[4]);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.setHostname(hostName);
  ArduinoOTA.begin();
  playSequenceFrame(offSequence);
}

void setupPins() {
  for (int i = 0; i < btnPinCount; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < ledPinCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting...");

  setupPins();

  turnOnWifi = digitalRead(headlightButtonPin);
  if (turnOnWifi == LOW) {
    connectToWiFi();
  } else {
    playLightSequence(sequenceFrames);
  }
}

void loop() {
  if (turnOnWifi == LOW) {
    ArduinoOTA.handle();
    int time = millis() % connectionIndicatorLEDRate;
    if (time < (connectionIndicatorLEDRate / 2) && digitalRead(headlightLeftLEDPin) != HIGH) {
      digitalWrite(headlightLeftLEDPin, HIGH);
    } else if(digitalRead(headlightLeftLEDPin) != LOW) {
      digitalWrite(headlightLeftLEDPin, LOW);
    }
  } else {
    checkBtnStateAndReturnBtnState(1);
    checkBtnStateAndReturnBtnState(0);
    if (btnStates[1]) {
      playLightSequence(animList[currentAnim]);
    } else {
      playSequenceFrame(offSequence);
    }
  }
}
