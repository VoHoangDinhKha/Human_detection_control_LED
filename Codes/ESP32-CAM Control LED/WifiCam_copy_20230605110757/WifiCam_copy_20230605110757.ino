#include "WifiCam.hpp"
#include <WiFi.h>
#include <WebSocketsServer.h>

static const char* WIFI_SSID = "PCOM91";
static const char* WIFI_PASS = "0251828225Oi";

esp32cam::Resolution initialResolution;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Chân GPIO để kết nối và điều khiển LED
const int LED_PIN_1 = 2;
const int LED_PIN_2 = 4;
const int LED_PIN_3 = 12;
const int LED_PIN_4 = 13;

bool ledStates[] = {false, false, false, false};

void updateLEDs() {
  digitalWrite(LED_PIN_1, ledStates[0] ? HIGH : LOW);
  digitalWrite(LED_PIN_2, ledStates[1] ? HIGH : LOW);
  digitalWrite(LED_PIN_3, ledStates[2] ? HIGH : LOW);
  digitalWrite(LED_PIN_4, ledStates[3] ? HIGH : LOW);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_TEXT:
      uint8_t value = atoi((char*)payload);
      Serial.print("Received value: ");
      Serial.println(value);

      if (value >= 0 && value <= 15) {
        // Update the LED states based on the received value
        for (int i = 0; i < 4; i++) {
          byte mask = 1 << i;  // Create a bitmask for each LED
          ledStates[i] = (value & mask) != 0;  // Check if the corresponding bit is set
          1001
        }
        updateLEDs();      
        delay 
        upda 
      }
      break;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT);
  Serial.println();
  delay(2000);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failure");
    delay(5000);
    ESP.restart();
  }
  Serial.println("WiFi connected");

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(360, 240);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }

  Serial.println("camera starting");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  addRequestHandlers();
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}
