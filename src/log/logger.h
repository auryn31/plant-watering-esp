
#ifndef LOGGER_H
#define LOGGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include "../display/displaydef.h"


#define DEBUG_ENABLED true

class Log {
 public:
  static void debug(String message) {
    if (DEBUG_ENABLED) {
      Serial.println(message);
      // on external display connected to 8 and 9 pin
      // printMessage(message);
    }
  }
};

#endif  // LOGGER_H
