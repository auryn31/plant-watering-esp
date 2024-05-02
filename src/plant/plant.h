#include <Arduino.h>
#include "../log/logger.h"

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
    Log::debug("Plant Details:");
    Log::debug("ID: " + id);
    Log::debug("Token ID: " + token_id);
    Log::debug("Name: " + name);
    Log::debug("ML per Watering: " + String(ml_per_watering));
    Log::debug("Max ML per Day: " + String(max_ml_per_day));
    Log::debug("Desired Humidity: " + String(desired_humidity));
    Log::debug("Created At: " + created_at);
  }
};