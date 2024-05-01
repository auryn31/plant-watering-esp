#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <config/config.h>
#include <plant/plant.h>

#include "pump/pump.h"

#define uS_TO_S_FACTOR \
  1000000 /* Conversion factor for micro seconds to seconds */
// sleep for 10 minutes
#define TIME_TO_SLEEP 3 * 600
// #define TIME_TO_SLEEP 5

void sendData(const String& url, int humidity, int lastWateringInMl);
void readDataAndSentToServer(String url);
int readHumidity(int plantPin, int dryValue, int wetValue);
String getPlantUrl(char* plantId);

String getPlantUrl(const char* plantId) {
  return String(("https://plant-watering-two.vercel.app/api/id/" +
                 std::string(Config::userId) + "/plant/" + std::string(plantId))
                    .c_str());
}

void calibrateSensor(int sensorPin) {
  delay(100);
  Serial.println("Calibrating sensor for pin " + String(sensorPin));
  delay(1000);
  int sensorValue1 = analogRead(sensorPin);
  Serial.println("Sensor value for pin " + String(sensorPin) + " is " +
                 sensorValue1);
  delay(5000);
  int sensorValue2 = analogRead(sensorPin);
  Serial.println("Sensor value for pin " + String(sensorPin) + " is " +
                 sensorValue2);
  delay(5000);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  // calibate sensor
  if (false) {
    for (PlantConfig plantConfig : plantConfigs) {
      calibrateSensor(plantConfig.sensorPin);
    }
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::ssid, Config::password);
    Serial.println("\nConnecting");
    // set pump to off to low
    for (PlantConfig plantConfig : plantConfigs) {
      pinMode(plantConfig.pumpPin, OUTPUT);
      digitalWrite(plantConfig.pumpPin, LOW);
    }

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
    for (PlantConfig plantConfig : plantConfigs) {
      try {
        const String plantUrl = getPlantUrl(plantConfig.plantId);
        const Plant plant = Plant::fetchPlant(plantUrl);
        const int humidity = readHumidity(
            plantConfig.sensorPin, plantConfig.dryValue, plantConfig.wetValue);
        Serial.println("Humidity for plant " + String(plantConfig.plantId) +
                       " is " + String(humidity));
        if (plant.desired_humidity > humidity &&
            plant.water_today < plant.max_ml_per_day &&
            plantConfig.should_be_watered) {
          // first send data to server to prevent watering without logging
          Plant::sendData(plantUrl, humidity, plant.ml_per_watering);
          Serial.println("Watering start...");
          Pump::pumpMl(plantConfig.pumpPin, plant.ml_per_watering);
          Serial.println("...watering done");
        } else {
          Serial.println("No watering...");
          Plant::sendData(plantUrl, humidity, 0);
          Serial.println("Humidity " + String(humidity) +
                         " was send to server.");
        }
      } catch (const char* msg) {
        Serial.println(msg);
        continue;
      }
      delay(500);
    }
    Serial.println("Going to sleep now");
    delay(1000);
    Serial.flush();
    esp_deep_sleep_start();
  }
}

void loop() {}

int readHumidity(int plantPin, int dryValue, int wetValue) {
  int sensorValue = analogRead(plantPin);
  Serial.println("Sensor value for pin " + String(plantPin) + " is " +
                 sensorValue);
  int humidity = map(sensorValue, dryValue, wetValue, 0, 100);
  Serial.println("Humidity for pin " + String(plantPin) + " is " + humidity);
  if (humidity < 0) {
    return 0;
  } else if (humidity > 100) {
    return 100;
  } else {
    return humidity;
  }
}