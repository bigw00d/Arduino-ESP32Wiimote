# ESP32Wiimote

ESP32Wiimote is an Arduino library that connects with a Wii remote.

## Requirement

- ESP32 board
- Arduino IDE (Version: 1.8.5)
- Wii Remote (RVL-CNT-01)

## Installation
1. Download the zip file.
2. Move the zip file to your libraries directory.
3. In the Arduino IDE, navigate to Sketch > Include Library > Add .ZIP Library.
4. Select the zip file.

## Examples

   see [ESP32WiimoteDemo.ino](./examples/ESP32WiimoteDemo)

- Caution: Nunchuck keeps outputting a lot of data for acceleration sensing
- You can Ignore changes with 'add filter(ACTION_IGNORE,...)'

## Usage 

1. To connect, press the 1 and 2 buttons on Wii Remote

1. The LED1 will be on when they have finished connecting  
<img width="30%" src="./remocon_led1_on.png" />  

## Licence

   see [LICENSE.md](./LICENSE.md) 
