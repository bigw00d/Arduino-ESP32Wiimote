// Copyright (c) 2020 Daiki Yasuda
//
// This is licensed under
// - Creative Commons Attribution-NonCommercial 3.0 Unported
// - https://creativecommons.org/licenses/by-nc/3.0/
// - Or see LICENSE.md
//
// The short of it is...
//   You are free to:
//     Share — copy and redistribute the material in any medium or format
//     Adapt — remix, transform, and build upon the material
//   Under the following terms:
//     NonCommercial — You may not use the material for commercial purposes.

#ifndef __ESP32_WIIMOTE_H__
#define __ESP32_WIIMOTE_H__

#include "esp_bt.h"
#include "TinyWiimote.h"

typedef struct {
    uint8_t xAxis;
    uint8_t yAxis;
    uint8_t zAxis;
} AccelState;

typedef enum {
    BUTTON_Z          = 0x00020000, // nunchuk
    BUTTON_C          = 0x00010000, // nunchuk
    BUTTON_PLUS       = 0x00001000,
    BUTTON_UP         = 0x00000800, // vertical orientation
    BUTTON_DOWN       = 0x00000400,
    BUTTON_RIGHT      = 0x00000200,
    BUTTON_LEFT       = 0x00000100,
    BUTTON_HOME       = 0x00000080,
    BUTTON_MINUS      = 0x00000010,
    BUTTON_A          = 0x00000008,
    BUTTON_B          = 0x00000004,
    BUTTON_ONE        = 0x00000002,
    BUTTON_TWO        = 0x00000001,
    NO_BUTTON         = 0x00000000
} ButtonState;

typedef struct {
        uint8_t xStick;
        uint8_t yStick;
        uint8_t xAxis;
        uint8_t yAxis;
        uint8_t zAxis;
// moved to ButtonState
//      uint8_t cBtn;
//      uint8_t zBtn;
} NunchukState;

enum
{
  FILTER_NONE                = 0x0000,
  FILTER_BUTTON              = 0x0001,
//FILTER_NUNCHUK_BUTTON      = 0x0002,
  FILTER_NUNCHUK_STICK       = 0x0004,
  FILTER_ACCEL               = 0x0008,
};

enum
{
  ACTION_IGNORE,
};

class ESP32Wiimote
{
public:
  ESP32Wiimote(int NUNCHUK_STICK_THRESHOLD = 1); // was 2

  void init(void);
  void task(void);
  int available(void);
  ButtonState getButtonState(void);
  AccelState getAccelState(void);
  NunchukState getNunchukState(void);
  void addFilter(int action, int filter);

private:

  typedef struct {
          size_t len;
          uint8_t data[];
  } queuedata_t;

  ButtonState _buttonState;
  ButtonState _oldButtonState;

  AccelState _accelState;
  AccelState _oldAccelState;

  NunchukState _nunchukState;
  NunchukState _oldNunchukState;

  int _nunStickThreshold;

  int _filter;

  static const TwHciInterface tinywii_hci_interface;
  static esp_vhci_host_callback_t vhci_callback;
  static xQueueHandle txQueue;
  static xQueueHandle rxQueue;

  static void createQueue(void);
  static void handleTxQueue(void);
  static void handleRxQueue(void);
  static esp_err_t sendQueueData(xQueueHandle queue, uint8_t *data, size_t len);
  static void notifyHostSendAvailable(void);
  static int notifyHostRecv(uint8_t *data, uint16_t len);
  static void hciHostSendPacket(uint8_t *data, size_t len);

};

#endif // __ESP32_WIIMOTE_H__
