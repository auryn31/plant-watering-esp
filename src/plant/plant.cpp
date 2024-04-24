#include "plant.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

Plant Plant::fetchPlant(String plantUrl) {
  HTTPClient http;
  http.begin(plantUrl);  // URL to the JSON data
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);

    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload);
    JsonObject obj = doc["plant"];

    Plant plant;
    plant.id = obj["id"].as<String>();
    plant.token_id = obj["token_id"].as<String>();
    plant.name = obj["name"].as<String>();
    plant.ml_per_watering = obj["ml_per_watering"];
    plant.max_ml_per_day = obj["max_ml_per_day"];
    plant.desired_humidity = obj["desired_humidity"];
    plant.created_at = obj["created_at"].as<String>();
    plant.last_watering_value_pushed =
        obj["last_watering_value_pushed"].as<String>();
    plant.water_today = obj["water_today"];
    plant.last_humidity = obj["last_humidity"];

    plant.printDetails();
    return plant;
  } else {
    Serial.println("Failed to fetch data");
  }
  http.end();
  throw "Failed to fetch data";
};

void Plant::sendData(const String& url, int humidity, int lastWateringInMl) {
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