#include <Arduino.h>

class Plant {
 public:
  String id;
  String token_id;
  String name;
  int ml_per_watering;
  int max_ml_per_day;
  int desired_humidity;
  String created_at;
  int water_today;
  int last_humidity;
  String last_watering_value_pushed;

  static Plant fetchPlant(String plantUrl);
  static void sendData(const String& url, int humidity, int lastWateringInMl);

  void printDetails() {
    Serial.println("Plant Details:");
    Serial.println("ID: " + id);
    Serial.println("Token ID: " + token_id);
    Serial.println("Name: " + name);
    Serial.println("ML per Watering: " + String(ml_per_watering));
    Serial.println("Max ML per Day: " + String(max_ml_per_day));
    Serial.println("Desired Humidity: " + String(desired_humidity));
    Serial.println("Created At: " + created_at);
  }
};