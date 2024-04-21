#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <config/config.h>

#define AOUT_PIN 34
#define uS_TO_S_FACTOR \
  1000000 /* Conversion factor for micro seconds to seconds */
// sleep for 10 minutes
#define TIME_TO_SLEEP 600

// constants
const int dryValue = 3100;
const int wetValue = 1200;
const String plantUrl =
    String(("https://plant-watering-two.vercel.app/api/id/" +
            std::string(userId) + "/plant/" + std::string(plant1Id))
               .c_str());

void sendData(const String& url, int humidity, int lastWateringInMl);
void readDataAndSentToServer();

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

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");
  readDataAndSentToServer();
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {}

void readDataAndSentToServer() {
  int sensorValue = analogRead(AOUT_PIN);
  Serial.println(sensorValue);
  Serial.println("in percent");
  int humidity = map(sensorValue, dryValue, wetValue, 0, 100);
  Serial.println(humidity);
  sendData(plantUrl, humidity, 0);  // Send the data
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