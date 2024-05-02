#include "plant.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "../log/logger.h"
#define MAX_RETRIES 3

Plant Plant::fetchPlant(String plantUrl) {
  int retries = 0;
  while (retries < MAX_RETRIES) {
    retries++;
    try {
      HTTPClient http;
      http.begin(plantUrl);  // URL to the JSON data
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Log::debug(payload);

        JsonDocument doc;
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
        Log::debug("Failed to fetch data");
        throw "Failed to fetch data";
      }
      http.end();
    } catch (const char* msg) {
      Log::debug("Failed to fetch data. Retrying...");
      delay(500);
    }
  };
  throw "Failed to fetch data";
};

void Plant::sendData(const String& url, int humidity, int lastWateringInMl) {
  if (WiFi.status() ==
      WL_CONNECTED) {  // Check if we're still connected to the WiFi
    int retries = 0;
    while (retries < MAX_RETRIES) {
      retries++;

      HTTPClient http;  // Declare an object of class HTTPClient
      http.begin(url);  // Specify request destination
      http.addHeader("Content-Type",
                     "application/json");  // Specify content-type header

      // Create the JSON document
      JsonDocument jsonDoc;

      jsonDoc["humidity"] = humidity;
      jsonDoc["last_watering_in_ml"] = lastWateringInMl;

      // Serialize JSON document into a string
      String jsonObject;
      serializeJson(jsonDoc, jsonObject);

      // Send the request
      int httpResponseCode = http.PUT(jsonObject);

      if (httpResponseCode > 0) {
        String response = http.getString();  // Get the response
        Log::debug("Response: " + String(httpResponseCode));
        Log::debug(response);
        http.end();
        return;
      } else {
        Log::debug("Error on sending PUT: " + String(httpResponseCode));
        http.end();
      }
      delay(500);
    }

  } else {
    Log::debug("Error in WiFi connection");
  }
}