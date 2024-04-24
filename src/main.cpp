#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <config/config.h>
#include <plant/plant.h>

#define AOUT_PIN 34
#define uS_TO_S_FACTOR \
  1000000 /* Conversion factor for micro seconds to seconds */
// sleep for 10 minutes
#define TIME_TO_SLEEP 600
// #define TIME_TO_SLEEP 5

// constants
const int dryValue = 3100;
const int wetValue = 1200;

void sendData(const String& url, int humidity, int lastWateringInMl);
void readDataAndSentToServer(String url);
int readHumidity(int plantPin);
String getPlantUrl(char* plantId);

String getPlantUrl(const char* plantId) {
  return String(("https://plant-watering-two.vercel.app/api/id/" +
                 std::string(userId) + "/plant/" + std::string(plantId))
                    .c_str());
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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
      const int humidity = readHumidity(plantConfig.pin);
      int wateringAmount = 0;
      // TODO: check if last pumping is at least 30 min ago
      if (plant.desired_humidity > humidity && plant.water_today < plant.max_ml_per_day) {
        wateringAmount = plant.ml_per_watering;
        // first send data to server to prevent watering without logging
        Plant::sendData(plantUrl, humidity, wateringAmount);
        Serial.println("Watering start...");
        digitalWrite(plantConfig.pumpPin, HIGH);
        // TODO: to be calulated how many seconds are how many ml
        delay(1000);
        digitalWrite(plantConfig.pumpPin, LOW);
        Serial.println("...watering done");
      }
    } catch (const char* msg) {
      Serial.println(msg);
      continue;
    }
  }
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {}

int readHumidity(int plantPin) {
  int sensorValue = analogRead(AOUT_PIN);
  int humidity = map(sensorValue, dryValue, wetValue, 0, 100);
  Serial.println("Humidity for pin " + String(plantPin) + " is " + humidity);
  return humidity;
}