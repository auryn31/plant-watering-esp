#include "pump.h"

#include "../config/config.h"

void Pump::pumpMl(int pumpPin, int ml) {
  pinMode(pumpPin, OUTPUT);

  int timeToPumpInS =
      ml / PumpConfig::mlPerSecond + PumpConfig::timeTilWaterInS;
  Serial.println("Pumping " + String(ml) + " ml for " + String(timeToPumpInS) +
                 " s");
  int timeToPumpInMs = timeToPumpInS * 1000;
  int timePerLoop = 20;
  int loops = timeToPumpInMs / timePerLoop;

  // use of pwm to reduce pump amount per sencond to allow slower flow
  for (int i = 0; i < loops; i++) {
    digitalWrite(pumpPin, HIGH);
    delay(10);
    digitalWrite(pumpPin, LOW);
    delay(10);
  }
  digitalWrite(pumpPin, LOW);
};