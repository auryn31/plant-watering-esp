#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <config/config.h>
#include <plant/plant.h>

#include "esp_wifi.h"
#include "log/logger.h"
#include "pump/pump.h"

#define uS_TO_S_FACTOR \
  1000000ULL /* Conversion factor for micro seconds to seconds */
// sleep for 10 minutes
#define TIME_TO_SLEEP 60 
// #define TIME_TO_SLEEP 5
#define MAX_WAIT_FOR_WIFI 20000

void sendData(const String& url, int humidity, int lastWateringInMl);
void readDataAndSentToServer(String url);
int readHumidity(int plantPin, int dryValue, int wetValue);
String getPlantUrl(char* plantId);
void handlePlant(PlantConfig plantConfig);
void turnOffPump(int pumpPin);
void gotoSleep();

String getPlantUrl(const char* plantId) {
  return String(("https://plant-watering-two.vercel.app/api/id/" +
                 std::string(Config::userId) + "/plant/" + std::string(plantId))
                    .c_str());
}

void calibrateSensor(int sensorPin) {
  delay(100);
  Log::debug("Calibrating sensor for pin " + String(sensorPin));
  delay(1000);
  int sensorValue1 = analogRead(sensorPin);
  Log::debug("Sensor value for pin " + String(sensorPin) + " is " +
             sensorValue1);
  delay(5000);
  int sensorValue2 = analogRead(sensorPin);
  Log::debug("Sensor value for pin " + String(sensorPin) + " is " +
             sensorValue2);
  delay(5000);
}

void setup() {
  if (DEBUG_ENABLED) {
    delay(1000);
    Serial.begin(9600);
    delay(100);
    // Log::initDisplay();
    while (!Serial);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  // calibate sensor
  if (false) {
    for (PlantConfig plantConfig : plantConfigs) {
      calibrateSensor(plantConfig.sensorPin);
    }
  } else {
    // normal mode
    esp_wifi_start();
    esp_wifi_connect();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::ssid, Config::password);
    WiFi.reconnect();
    Log::debug("Connecting");
    // set pump to off to low
    for (PlantConfig plantConfig : plantConfigs) {
      turnOffPump(plantConfig.pumpPin);
    }

    unsigned long start = millis();
    // && (millis() - start) < MAX_WAIT_FOR_WIFI
    while (WiFi.status() != WL_CONNECTED) {
      Log::debug("Wifi not connected yet... \n" + millis());
      delay(100);
    }

    Log::debug("Connected to the WiFi network");
    Log::debug("Local IP: \n" + WiFi.localIP().toString());

    for (PlantConfig plantConfig : plantConfigs) {
      // delay to not overwhelm server
      delay(500);
      try {
        handlePlant(plantConfig);
      } catch (const char* msg) {
        Log::debug(msg);
        continue;
      }
    }
    gotoSleep();
  }
}

void loop() {}

void gotoSleep() {
  Log::debug("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
             " Seconds");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Log::debug("Going to sleep now");
  delay(500);
  // seems like an issue with esp32 c3 if not connected to usb
  if (DEBUG_ENABLED) {
    Serial.flush();
  }
  esp_wifi_disconnect();
  WiFi.disconnect();
  // esp_wifi_stop();
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  esp_deep_sleep_start();
}

void turnOffPump(int pumpPin) {
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
}

void handlePlant(PlantConfig plantConfig) {
  const String plantUrl = getPlantUrl(plantConfig.plantId);
  const Plant plant = Plant::fetchPlant(plantUrl);
  const int humidity = readHumidity(plantConfig.sensorPin, plantConfig.dryValue,
                                    plantConfig.wetValue);
  Log::debug("Humidity for plant " + String(plantConfig.plantId) + " is " +
             String(humidity));
  if (plant.desired_humidity > humidity &&
      plant.water_today < plant.max_ml_per_day &&
      plantConfig.should_be_watered) {
    // first send data to server to prevent watering without logging
    Plant::sendData(plantUrl, humidity, plant.ml_per_watering);
    Log::debug("Watering start...");
    Pump::pumpMl(plantConfig.pumpPin, plant.ml_per_watering);
    Log::debug("...watering done");
  } else {
    Log::debug("No watering...");
    Log::debug("Send no watering to server");
    Plant::sendData(plantUrl, humidity, 0);
    Log::debug("Humidity " + String(humidity) + " was maybe send to server.");
  }
}

int readHumidity(int plantPin, int dryValue, int wetValue) {
  int sensorValue = analogRead(plantPin);
  Log::debug("Sensor value for pin " + String(plantPin) + " is " + sensorValue);
  int humidity = map(sensorValue, dryValue, wetValue, 0, 100);
  Log::debug("Humidity for pin " + String(plantPin) + " is " + humidity);
  if (humidity < 0) {
    return 0;
  } else if (humidity > 100) {
    return 100;
  } else {
    return humidity;
  }
}