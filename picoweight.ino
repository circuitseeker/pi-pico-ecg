#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFi.h>
#include "HX711.h"

// Pin Definitions
#define DHTPIN 2
#define DHTTYPE DHT11
#define PULSE_SENSOR_PIN 6
#define ECG_OUT_PIN 3
#define LO_PLUS_PIN 4
#define LO_MINUS_PIN 5

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
HX711 scale;

const char* ssid = "123456789";
const char* password = "123456789";
const char* server = "api.thingspeak.com";
const String apiKey = "LP3R0NHAIG5C8Q2I"; // Replace with your Write API Key

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PULSE_SENSOR_PIN, INPUT);
  pinMode(ECG_OUT_PIN, INPUT);
  pinMode(LO_PLUS_PIN, OUTPUT);
  digitalWrite(LO_PLUS_PIN, HIGH);
  pinMode(LO_MINUS_PIN, OUTPUT);
  digitalWrite(LO_MINUS_PIN, LOW);
  
  scale.begin(18, 19); // Setup HX711 to GP18 (D18) and GP19 (D19)

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int pulse = analogRead(PULSE_SENSOR_PIN);
  int ecgValue = digitalRead(ECG_OUT_PIN);
  float weight = getWeight(); // Read weight from load cell

  if (!isnan(h) && !isnan(t)) {
    sendToThingSpeak(t, h, pulse, ecgValue, weight);
    delay(20000); // Send data every 20 seconds
  }
}

float getWeight() {
  if (scale.is_ready()) {
    return scale.get_units(10); // Get weight reading
  } else {
    Serial.println("Error: Check connections");
    return -1; // Return -1 to indicate an error
  }
}

void sendToThingSpeak(float temperature, float humidity, int pulseRate, int ecgValue, float weight) {
  if (!client.connect(server, 80)) {
    Serial.println("Connection failed");
    return;
  }

  String postStr = apiKey;
  postStr += "&field1=";
  postStr += String(temperature);
  postStr += "&field2=";
  postStr += String(humidity);
  postStr += "&field3=";
  postStr += String(pulseRate);
  postStr += "&field4=";
  postStr += String(ecgValue);
  postStr += "&field5=";
  postStr += String(weight); // Add weight data
  postStr += "\r\n\r\n";

  client.print("POST /update HTTP/1.1\n");
  client.print("Host: api.thingspeak.com\n");
  client.print("Connection: close\n");
  client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
  client.print("Content-Type: application/x-www-form-urlencoded\n");
  client.print("Content-Length: ");
  client.print(postStr.length());
  client.print("\n\n");
  client.print(postStr);

  delay(500); // Wait for the server to respond

  client.stop();
}
 // Rest of the code remains unchanged
}

