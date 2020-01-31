#include "ESP32Wiimote.h"

ESP32Wiimote wiimote;

void setup()
{
    Serial.begin(115200);
    wiimote.init();
}

void loop()
{
  wiimote.task();
  if (wiimote.available() > 0) {
      uint16_t button = wiimote.getButtonState();
      Serial.printf("%04x\n", button);
      if (button == ESP32Wiimote::BUTTON_A) {
        Serial.println("A button");
      }
  }
  delay(10);
}
