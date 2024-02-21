#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define PIN_ECHO D2
#define PIN_TRIGGER D3
#define SERVER_IP "192.168.219.131"
#define SERVER_PORT 8001
#define ENDPOINT "/test/sensorEstacionamiento.php"

const char *ssid = "RedOcta";
const char *password = "12345678";

bool ocupado = false;
unsigned long lastTime = 0;
const unsigned long timerDelay = 10000;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIGGER, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to network...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void SensorUltra();
void peticionHTTP();

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= timerDelay) {
    SensorUltra();
    peticionHTTP();
    lastTime = currentTime;
  }
}

void SensorUltra() {
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH);
  float distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10) {
    ocupado = true;
  } else {
    ocupado = false;
  }
}

void peticionHTTP() {
  WiFiClient client;
  HTTPClient http;

  if (WiFi.status() == WL_CONNECTED) {
    String serverPath = String("http://") + SERVER_IP + ":" + SERVER_PORT + ENDPOINT + "?ocupado=" + String(ocupado ? 1 : 0) + "&cajon=1";
    Serial.print("Connecting to server: ");
    Serial.println(serverPath);

    if (http.begin(client, serverPath)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.print("HTTP status: ");
        Serial.println(httpCode);
        String payload = http.getString();
        Serial.println("Response: ");
        Serial.println(payload);
      } else {
        Serial.print("HTTP error code: ");
        Serial.println(httpCode);
      }
      http.end();
    } else {
      Serial.println("Failed to connect to server");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}
