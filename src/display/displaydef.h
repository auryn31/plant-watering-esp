#ifndef displaydef_H
#define displaydef_H

#include <Adafruit_SSD1306.h>

void startDisplay();
void printMessage(String message);
extern Adafruit_SSD1306 display;

#endif