
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

#define DEBUG_ENABLED false

class Log {
public:
  static void debug(String message) {
    if (DEBUG_ENABLED) {
      Serial.println(message);
    }
  }
};

#endif // LOGGER_H
