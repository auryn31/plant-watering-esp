# ESP Code for plant watering system

This is the EPS code for my plant watering system.

## Dev

- create a file `./src/config/config.h`
- Add the following parameter:

```cpp
#include <string>

class PlantConfig {
  public:
    PlantConfig(const char* plantId, const int sensorPin, const int pumpPin){
      this->plantId = plantId;
      this->pin = pin;
      this->pumpPin = pumpPin;
    }
    const char* plantId;
    int pin;
    int pumpPin;
};

const char* ssid = "xxx";
const char* password = "xxx";
const char* userId = "xxx";
const PlantConfig plantConfigs[] = {PlantConfig("xxx", 32, 19)};
```

## Links


[Frontend](https://github.com/auryn31/plant-watering)

[Video](https://youtu.be/qXmGnc0GFfs)
