
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

#define IN4 15 // d8
// #define IN4 16 // d0
#define IN3 14 // d5
#define IN2 12 // d6
#define IN1 13 // d7

int targetSpeedA = 0;
int curSpeedA1 = 0;
int curSpeedA2 = 0;
int targetSpeedB = 0;
int curSpeedB3 = 0;
int curSpeedB4 = 0;

int maxSpeed = 150;
int accel = 8;
int deccel = 8;

bool isUpdating = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool ledState = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html, body {
        padding: 0;
        margin: 0;
        width: 100vw;
        height: 100vh;
      }
      body {
        display: flex;
        justify-content: center;
        align-items: center;
      }
      .arrows {
        display: grid;
        grid-template-rows: 1fr 1fr 1fr;
        align-items: center;
        font-size: 45px;
        justify-items: center;
      }
      .arrows > * {
        text-align: center;
      }
      .m {
        display: flex;
        gap: 100px;
      }
      .active.u, .active.r, .active.d, .active.l {
        color: rgb(0, 225, 0);
        text-shadow: 0px 0px 10px #e90000
      }
      button {
        width: 100px;
        height: 100px;
        font-size: 60px;
        background: transparent;
        border-radius: 20px;
      }
    </style>
  </head>
  <body>
    <div id="status" class="arrows">
      <button class="u">/\</button>
      <div class="m">
        <button class="l">&lt;</button>
        <button class="r">&gt;</button>
      </div>
      <button class="d">\/</button>
    </div>
    <script>
      const gateway = `ws://${window.location.hostname}/ws`;
      // const gateway = `ws://192.168.1.114/ws`;
      let websocket;
      window.addEventListener('load', onLoad);
      function initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        websocket = new WebSocket(gateway);
        websocket.onopen    = onOpen;
        websocket.onclose   = onClose;
        websocket.onmessage = onMessage; // <-- add this line
      }
      const updateStatus = (code, s) => {
        if (websocket.readyState != WebSocket.OPEN) {
          console.log('websocket not connected yet');
          return
        }
        if (!elStatus[code]) return
        elStatus[code].s = s;
        if (s) elStatus[code].el.classList.add('active')
        else elStatus[code].el.classList.remove('active')
        websocket.send(`${code}:${s ? 1 : 0}`);
        console.log('sent');
      };
      function onOpen(event) {
        console.log('Connection opened');
        el_l.addEventListener('click', () => updateStatus(37, true));
        
        el_u.addEventListener('click', () => updateStatus(38, true));
        
        el_r.addEventListener('click', () => updateStatus(39, true));
        
        el_d.addEventListener('click', () => updateStatus(40, true));
        

        el_l.addEventListener('touchend', () => updateStatus(37, false));
        el_l.addEventListener('mouseup', () => updateStatus(37, false));
        el_u.addEventListener('touchend', () => updateStatus(38, false));
        el_u.addEventListener('mouseup', () => updateStatus(38, false));
        el_r.addEventListener('touchend', () => updateStatus(39, false));
        el_r.addEventListener('mouseup', () => updateStatus(39, false));
        el_d.addEventListener('touchend', () => updateStatus(40, false));
        el_d.addEventListener('mouseup', () => updateStatus(40, false));
      }
      function onClose(event) {
        console.log('Connection closed');
        setTimeout(initWebSocket, 2000);
      }
      function onMessage(event) {
      }
      function onLoad(event) {
        initWebSocket();
      }
      el_l = document.querySelector('.l')
      el_u = document.querySelector('.u')
      el_r = document.querySelector('.r')
      el_d = document.querySelector('.d')
      const elStatus = {
        37: { s: 0, el: el_l }, // left
        38: { s: 0, el: el_u }, // up
        39: { s: 0, el: el_r }, // right
        40: { s: 0, el: el_d }, // down
      }

      window.addEventListener('keydown', (e) => updateStatus(e.keyCode, true));
      window.addEventListener('keyup', (e) => updateStatus(e.keyCode, false));
    </script>
  </body>
</html>
)rawliteral";


void setup() {
	Serial.begin(115200);
  Serial.println("Booting");
  pinMode(LED_BUILTIN, OUTPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname("esp-bot");
  bool isFailed = false;

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);

  ArduinoOTA.setHostname("lil-esp-tank");
  ArduinoOTA.setPassword("pass");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    isUpdating = true;
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    isUpdating = false;
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
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
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  initWebSocket();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void notifyClients() {
  ws.textAll(String(ledState));
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    // if (strcmp((char*)data, "toggle") == 0) {
    // Serial.println((char*)data);
    // notifyClients(); // maybe later
    // }
    if (strcmp((char*)data, "38:1") == 0) { // up
      setSpeedA(maxSpeed);
      setSpeedB(maxSpeed);
      return;
    }
    if (strcmp((char*)data, "38:0") == 0) { // up
      setSpeedA(0);
      setSpeedB(0);
      return;
    }

    if (strcmp((char*)data, "40:1") == 0) { // down
      setSpeedA(-maxSpeed);
      setSpeedB(-maxSpeed);
      return;
    }
    if (strcmp((char*)data, "40:0") == 0) { // down
      setSpeedA(0);
      setSpeedB(0);
      return;
    }

    if (strcmp((char*)data, "37:1") == 0) { // left
      setSpeedA(maxSpeed);
      setSpeedB(-maxSpeed);
      return;
    }
    if (strcmp((char*)data, "37:0") == 0) { // left
      setSpeedA(0);
      setSpeedB(0);
      return;
    }
    
    if (strcmp((char*)data, "39:1") == 0) { // right
      setSpeedB(maxSpeed);
      setSpeedA(-maxSpeed);
      return;
    }
    if (strcmp((char*)data, "39:0") == 0) { // right
      setSpeedB(0);
      setSpeedA(0);
      return;
    }
  }
}
void setSpeedA(int speed) {
  targetSpeedA = speed;
}
String updateSpeedA() {
  int newSpeed = 0;
  int curSpeed = 0;
  if (curSpeedA1 > 0) {
    curSpeed = curSpeedA1;
  } else if (curSpeedA2 < 0) {
    curSpeed = curSpeedA2;
  }
  
  if (targetSpeedA > 0) {
    if (curSpeed < targetSpeedA) {
      newSpeed = curSpeed + accel;
      if (newSpeed > targetSpeedA) { newSpeed = targetSpeedA; }
    }
  } else if (targetSpeedA < 0) {
    if (curSpeed > targetSpeedA) {
      newSpeed = curSpeed - accel;
      if (newSpeed < targetSpeedA) { newSpeed = targetSpeedA; }
    }
  }
  
  if (curSpeed == targetSpeedA) {
    newSpeed = curSpeed;
  }

  if (targetSpeedA == 0) {
    if (curSpeed > 0) {
      newSpeed = curSpeed - deccel;
    }
    if (curSpeed < 0) {
      newSpeed = curSpeed + deccel;
    }
    if (abs(newSpeed) < deccel) {
      newSpeed = 0;
    }
  }

  if (newSpeed > 0) {
    analogWrite(IN1, newSpeed);
    curSpeedA1 = newSpeed;
    analogWrite(IN2, 0);
    curSpeedA2 = 0;
  } else if (newSpeed < 0) {
    analogWrite(IN1, 0);
    curSpeedA1 = 0;
    analogWrite(IN2, -newSpeed);
    curSpeedA2 = newSpeed;
  } else {
    analogWrite(IN1, 0);
    curSpeedA1 = 0;
    analogWrite(IN2, 0);
    curSpeedA2 = 0;
  }

  String out = "targetSpeedA:";
  out += targetSpeedA;
  out += ",curSpeedA:";
  out += curSpeed;
  out += ",newSpeedA:";
  out += newSpeed;
  return out;
}

void setSpeedB(int speed) {
  targetSpeedB = speed;
}
String updateSpeedB() {
  int newSpeed = 0;
  int curSpeed = 0;
  if (curSpeedB3 > 0) {
    curSpeed = curSpeedB3;
  } else if (curSpeedB4 < 0) {
    curSpeed = curSpeedB4;
  }
  
  if (targetSpeedB > 0) {
    if (curSpeed < targetSpeedB) {
      newSpeed = curSpeed + accel;
      if (newSpeed > targetSpeedB) { newSpeed = targetSpeedB; }
    }
  } else if (targetSpeedB < 0) {
    if (curSpeed > targetSpeedB) {
      newSpeed = curSpeed - accel;
      if (newSpeed < targetSpeedB) { newSpeed = targetSpeedB; }
    }
  }

  if (curSpeed == targetSpeedB) {
    newSpeed = curSpeed;
  }

  if (targetSpeedB == 0) {
    if (curSpeed > 0) {
      newSpeed = curSpeed - deccel;
    }
    if (curSpeed < 0) {
      newSpeed = curSpeed + deccel;
    }
    if (abs(newSpeed) < deccel) {
      newSpeed = 0;
    }
  }

  if (newSpeed > 0) {
    analogWrite(IN3, newSpeed);
    curSpeedB3 = newSpeed;
    analogWrite(IN4, 0);
    curSpeedB4 = 0;
  } else if (newSpeed < 0) {
    analogWrite(IN3, 0);
    curSpeedB3 = 0;
    analogWrite(IN4, -newSpeed);
    curSpeedB4 = newSpeed;
  } else {
    analogWrite(IN3, 0);
    curSpeedB3 = 0;
    analogWrite(IN4, 0);
    curSpeedB4 = 0;
  }

  String out = "targetSpeedB:";
  out += targetSpeedB;
  out += ",curSpeedB:";
  out += curSpeed;
  out += ",newSpeedB:";
  out += newSpeed;
  return out;
}

// void setSpeedA(int speed) {
//   if (isUpdating) {
//     analogWrite(IN1, 0);
//     analogWrite(IN2, 0);
//     return;
//   }
//   if (speed >= 0 && speed < 256) {
//     analogWrite(IN1, speed);
//     analogWrite(IN2, 0);
//   } else if (speed < 0 && speed > -255) {
//     analogWrite(IN1, 0);
//     analogWrite(IN2, -speed);
//   }  
// }

// void setSpeedB(int speed) {
//   if (isUpdating) {
//     analogWrite(IN3, 0);
//     analogWrite(IN4, 0);
//     return;
//   }
//   if (speed >= 0 && speed < 256) {
//     analogWrite(IN3, speed);
//     analogWrite(IN4, 0);
//   } else if (speed < 0 && speed > -256) {
//     analogWrite(IN3, 0);
//     analogWrite(IN4, -speed);
//   }
// }

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  ArduinoOTA.handle();

  String rA = updateSpeedA();
  String rB = updateSpeedB();
  Serial.println(rA + "," + rB);

}
