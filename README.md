# Arduino-ESP32Wiimote

ESP32Wiimote is a Arduino library that connects with a Wii remote.

## Installation
1. Download the zip file
2. Move the zip file to your libraries directory.
3. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library.
4. Select the zip file.

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
      Serial.printf("%04x\n", button);
      if (button == ESP32Wiimote::BUTTON_A) {
        Serial.println("A button");
      }
  }
  delay(10);
}

```

#### Button
'button' is expressed as OR of bits:

```
  BUTTON_LEFT       = 0x0800,
  BUTTON_RIGHT      = 0x0400,
  BUTTON_UP         = 0x0200,
  BUTTON_DOWN       = 0x0100,
  BUTTON_A          = 0x0008,
  BUTTON_B          = 0x0004,
  BUTTON_PLUS       = 0x1000,
  BUTTON_HOME       = 0x0080,
  BUTTON_MINUS      = 0x0010,
  BUTTON_ONE        = 0x0002,
  BUTTON_TWO        = 0x0001
```

# Licence

   see [LICENSE.md](./LICENSE.md) 
