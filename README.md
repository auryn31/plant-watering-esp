# ESP Code for plant watering system

This is the EPS code for my plant watering system.

## Dev

- create a file `./src/config/config.h`
- Add the following parameter:

```cpp
#include <string>

class PlantConfig {
 public:
  PlantConfig(const char* plantId, const int sensorPin, const int pumpPin,
              const bool should_be_watered = false, const int dryValue = 3100,
              const int wetValue = 1200) {
    this->plantId = plantId;
    this->sensorPin = sensorPin;
    this->pumpPin = pumpPin;
    this->should_be_watered = should_be_watered;
    this->dryValue = dryValue;
    this->wetValue = wetValue;
  }
  const char* plantId;
  int sensorPin;
  int pumpPin;
  bool should_be_watered;
  int dryValue;
  int wetValue;
};

class Config {
 public:
  // config of wifi
  static constexpr const char* ssid = "xxx";
  static constexpr const char* password = "xxx";

  static constexpr const char* userId = "xxx";
};

// config of pump
class PumpConfig {
 public:
  static const int mlPerSecond = 18;
  static const int timeTilWaterInS = 1;
};

// adc2 and wifi wont work together
const PlantConfig plantConfigs[] = {
    PlantConfig("xxx", 0, 5, true, 2800,
                1780),  // rose 
};
```

## Links

[Frontend](https://github.com/auryn31/plant-watering)

[Video Pt 1](https://youtu.be/qXmGnc0GFfs)
[Video Pt 2](https://youtu.be/qXmGnc0GFfs)
