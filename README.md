# ESP32Wiimote

ESP32Wiimote is a library that connects with a Wii remote.

## Installation
1. Download the zip file
2. Extract the zip file
3. Move the "ESP32Wiimote" folder that has been extracted to your libraries directory.
4. Start the Arduino Software (IDE), go to Sketch > Include Library. Verify that the "ESP32Wiimote" is available in the list.

## Example

```ESP32WiimoteDemo.ino.cpp
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
      // Serial.printf("button:%d\n", button);
      Serial.printf("%04x\n", button);
      if (button == ESP32Wiimote::BUTTON_A) {
        Serial.println("A button");
      }
  }
  delay(10);
}
```

# Licence

   see [LICENSE.md](./LICENSE.md) 
