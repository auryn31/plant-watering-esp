#include "Displaydef.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void startDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  } else {
    Serial.println(F("SSD1306 allocation success"));
    // printMessage("SSD1306 allocation success");
  }
}

void printMessage(String message) {
  startDisplay();
  Serial.println("Display: " + message); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Debug: " + message);
  display.display();
  delay(50);
}