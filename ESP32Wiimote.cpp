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

#include "Arduino.h"
#include "ESP32Wiimote.h"
#include "TinyWiimote.h"

ESP32Wiimote::ESP32Wiimote(void)
{
    ;
}

void ESP32Wiimote::init(void)
{
    TinyWiimoteInit();
}

void ESP32Wiimote::task(void)
{
  TinyWiimoteHandle();
}

int ESP32Wiimote::available(void)
{
    return TinyWiimoteAvailable();
}

uint16_t ESP32Wiimote::getButtonState(void)
{
  uint16_t button = 0;
  twii_recv_data_st getData = TinyWiimoteRead();
  button = (getData.data[TWII_OFFSET_BTNS1] << 8) | getData.data[TWII_OFFSET_BTNS2];
  return button;
}

