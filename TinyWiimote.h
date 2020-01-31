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

#ifndef _TINY_WIIMOTE_H_
#define _TINY_WIIMOTE_H_

#define RECIEVED_DATA_MAX_LEN     (50)
struct twii_recv_data_st {
  uint8_t number;
  uint8_t data[RECIEVED_DATA_MAX_LEN];
  uint8_t len;
};
#define TWII_OFFSET_BTNS1 (2)
#define TWII_OFFSET_BTNS2 (3)

void TinyWiimoteInit();
void TinyWiimoteHandle();
int TinyWiimoteAvailable(void);
twii_recv_data_st TinyWiimoteRead(void);


#endif // _TINY_WIIMOTE_H_
