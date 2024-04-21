#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <wifi/wifi_settings.h>

#define AOUT_PIN 34

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);  // Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

const int dryValue = 3100;
const int wetValue = 1200;
const String plantUrl =
    "https://plant-watering-two.vercel.app/api/id/"
    "6ffa8fc2-3dec-4590-a593-f770c6e39d80/plant/"
    "d914cdb9-641c-4259-a532-70520055d2ef";

void sendData(const String& url, int humidity, int lastWateringInMl);

void loop() {
  int sensorValue = analogRead(AOUT_PIN);
  Serial.println(sensorValue);
  Serial.println("in percent");
  int humidity = map(sensorValue, dryValue, wetValue, 0, 100);
  Serial.println(humidity);
  sendData(plantUrl, humidity, 0);  // Send the data

  // wait minute 
  delay(60000);
}

void sendData(const String& url, int humidity, int lastWateringInMl) {
  if (WiFi.status() ==
      WL_CONNECTED) {  // Check if we're still connected to the WiFi

    HTTPClient http;  // Declare an object of class HTTPClient
    http.begin(url);  // Specify request destination
    http.addHeader("Content-Type",
                   "application/json");  // Specify content-type header

    // Create the JSON document
    StaticJsonDocument<200> jsonDoc;  // Create an instance of JsonDocument

    jsonDoc["humidity"] = humidity;
    jsonDoc["last_watering_in_ml"] = lastWateringInMl;

    // Serialize JSON document into a string
    String jsonObject;
    serializeJson(jsonDoc, jsonObject);

    // Send the request
    int httpResponseCode = http.PUT(jsonObject);  // Make a POST request

    if (httpResponseCode > 0) {            // Check the returning code
      String response = http.getString();  // Get the response
      Serial.println(httpResponseCode);    // Print the response code
      Serial.println(response);            // Print the response return payload
    } else {
      Serial.print("Error on sending PUT: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Close connection
  } else {
    Serial.println("Error in WiFi connection");
  }
}