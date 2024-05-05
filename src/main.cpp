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
#define TIME_TO_SLEEP 600
// #define TIME_TO_SLEEP 5
// wait max 30 seconds for wifi
#define MAX_WAIT_FOR_WIFI 30000

void sendData(const String& url, int humidity, int lastWateringInMl);
void readDataAndSentToServer(String url);
int readHumidity(int plantPin, int dryValue, int wetValue);
String getPlantUrl(char* plantId);
void handlePlant(PlantConfig plantConfig);
void turnOffPump(int pumpPin);
void gotoSleep();
void runPlantReadings();
void checkWifiAndInternetAndRestartIfBroken();
Ticker tkSec;

enum Mode { PLANT, CALIBRATE };

const Mode mode = PLANT;

String getPlantUrl(const char* plantId) {
  return String(("https://plant-watering-two.vercel.app/api/id/" +
                 std::string(Config::userId) + "/plant/" + std::string(plantId))
                    .c_str());
}

void calibrateSensor(PlantConfig plantConfig) {
  int sensorValue1 = analogRead(plantConfig.sensorPin);
  Log::debug("Dry calibration value " + String(plantConfig.dryValue));
  Log::debug("Wet calibration value " + String(plantConfig.wetValue));
  int result = readHumidity(plantConfig.sensorPin, plantConfig.dryValue,
                            plantConfig.wetValue);
  Log::debug("Result is " + String(result));
  delay(200);
}

void checkWifiAndInternetAndRestartIfBroken() {
  Log::debug("Checking if wifi is connected...");
  unsigned long start = millis();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      Log::debug("Wifi not connected yet... \n" + millis());
      delay(100);
      if ((millis() - start) > MAX_WAIT_FOR_WIFI) {
        Log::debug("Wifi not connecting, restart ESP \n" + millis());
        ESP.restart();
      }
    }
  }
  Log::debug("Ping google.com to check internet...");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://www.google.com");
    int httpCode = http.GET();
    if (httpCode > 0) {
      Log::debug("Internet is working fine");
    } else {
      Log::debug("Internet is not working fine, restart ESP");
      ESP.restart();
    }
    http.end();
  }
}

void setup() {
  if (DEBUG_ENABLED) {
    delay(1000);
    Serial.begin(9600);
    delay(100);
    while (!Serial);
  }

  // digitalWrite(LED_BUILTIN, HIGH);
  if (mode == CALIBRATE) {
    Log::debug("Calibrating sensors...");
    while (true) {
      for (PlantConfig plantConfig : plantConfigs) {
        calibrateSensor(plantConfig);
      }
    }
  }

  if (mode == PLANT) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::ssid, Config::password);

    Log::debug("Turning off all pumps...");
    for (PlantConfig plantConfig : plantConfigs) {
      turnOffPump(plantConfig.pumpPin);
    }

    Log::debug("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      Log::debug("Wifi not connected yet... \n" + millis());
      delay(100);
    }

    Log::debug("Connected to the WiFi network");
    Log::debug("Local IP: \n" + WiFi.localIP().toString());
    Log::debug("Run once and then every " + String(TIME_TO_SLEEP) + " seconds");
    runPlantReadings();
    tkSec.attach(TIME_TO_SLEEP, runPlantReadings);
  }
}
void runPlantReadings() {
  checkWifiAndInternetAndRestartIfBroken();
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
}

void loop() {}

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
      plant.water_today < plant.max_ml_per_day && plant.watering_allowed) {
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