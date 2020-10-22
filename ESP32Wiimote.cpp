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

//#define CONFIG_CLASSIC_BT_ENABLED 1

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"
#include "esp_bt.h"
#include <HardwareSerial.h>

#include "time.h"
#include "sys/time.h"

#include "ESP32Wiimote.h"
#include "TinyWiimote.h"

#define WIIMOTE_VERBOSE 0

#if WIIMOTE_VERBOSE
#define VERBOSE_PRINT(...) Serial.printf(__VA_ARGS__)
#define VERBOSE_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define VERBOSE_PRINT(...) do {} while(0)
#define VERBOSE_PRINTLN(...) do {} while(0)
#endif

//#define UNVERBOSE_PRINT(...) Serial.printf(__VA_ARGS__)
#define UNVERBOSE_PRINT(...) do {} while(0)

#define RX_QUEUE_SIZE 32
#define TX_QUEUE_SIZE 32
xQueueHandle ESP32Wiimote::rxQueue = NULL;
xQueueHandle ESP32Wiimote::txQueue = NULL;

const TwHciInterface ESP32Wiimote::tinywii_hci_interface = {
  ESP32Wiimote::hciHostSendPacket
};

esp_vhci_host_callback_t ESP32Wiimote::vhci_callback;

ESP32Wiimote::ESP32Wiimote(int NUNCHUK_STICK_THRESHOLD)
{
    _nunStickThreshold = NUNCHUK_STICK_THRESHOLD;
    _filter = FILTER_NONE;
}

void ESP32Wiimote::notifyHostSendAvailable(void) {
  VERBOSE_PRINT("notifyHostSendAvailable\n");
  if(!TinyWiimoteDeviceIsInited()){
    TinyWiimoteResetDevice();
  }
}

void ESP32Wiimote::createQueue(void) {
  txQueue = xQueueCreate(TX_QUEUE_SIZE, sizeof(queuedata_t*));
  if (txQueue == NULL){
    VERBOSE_PRINTLN("xQueueCreate(txQueue) failed");
    return;
  }
  rxQueue = xQueueCreate(RX_QUEUE_SIZE, sizeof(queuedata_t*));
  if (rxQueue == NULL){
    VERBOSE_PRINTLN("xQueueCreate(rxQueue) failed");
    return;
  }
}

void ESP32Wiimote::handleTxQueue(void) {
  if(uxQueueMessagesWaiting(txQueue)){
    bool ok = esp_vhci_host_check_send_available();
    VERBOSE_PRINT("esp_vhci_host_check_send_available=%d", ok);
    if(ok){
      queuedata_t *queuedata = NULL;
      if(xQueueReceive(txQueue, &queuedata, 0) == pdTRUE){
        esp_vhci_host_send_packet(queuedata->data, queuedata->len);
        UNVERBOSE_PRINT("SEND => %s\n", format2Hex(queuedata->data, queuedata->len));
        free(queuedata);
      }
    }
  }
}

void ESP32Wiimote::handleRxQueue(void) {
  if(uxQueueMessagesWaiting(rxQueue)){
    queuedata_t *queuedata = NULL;
    if(xQueueReceive(rxQueue, &queuedata, 0) == pdTRUE){
      handleHciData(queuedata->data, queuedata->len);
      free(queuedata);
    }
  }
}

esp_err_t ESP32Wiimote::sendQueueData(xQueueHandle queue, uint8_t *data, size_t len) {
    VERBOSE_PRINTLN("sendQueueData");
    if(!data || !len){
        VERBOSE_PRINTLN("no data");
        return ESP_OK;
    }
    queuedata_t * queuedata = (queuedata_t*)malloc(sizeof(queuedata_t) + len);
    if(!queuedata){
        VERBOSE_PRINTLN("malloc failed");
        return ESP_FAIL;
    }
    queuedata->len = len;
    memcpy(queuedata->data, data, len);
    UNVERBOSE_PRINT("RECV <= %s\n", format2Hex(queuedata->data, queuedata->len));
    if (xQueueSend(queue, &queuedata, portMAX_DELAY) != pdPASS) {
        VERBOSE_PRINTLN("xQueueSend failed");
        free(queuedata);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void ESP32Wiimote::hciHostSendPacket(uint8_t *data, size_t len) {
  sendQueueData(txQueue, data, len);
}

int ESP32Wiimote::notifyHostRecv(uint8_t *data, uint16_t len) {
  VERBOSE_PRINT("notifyHostRecv:");
  for (int i = 0; i < len; i++)
  {
    VERBOSE_PRINT(" %02x", data[i]);
  }
  VERBOSE_PRINTLN("");

  if(ESP_OK == sendQueueData(rxQueue, data, len)){
    return ESP_OK;
  }else{
    return ESP_FAIL;
  }
}

void ESP32Wiimote::init(void)
{
    TinyWiimoteInit(tinywii_hci_interface);
    createQueue();
    vhci_callback.notify_host_recv = notifyHostRecv;
    vhci_callback.notify_host_send_available = notifyHostSendAvailable;    

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (!btStart()) {
        Serial.printf( "btStart() failed\n");
        return;
    }

    esp_err_t ret;
    if ((ret = esp_vhci_host_register_callback(&vhci_callback)) != ESP_OK) {
        return;
    }
}

void ESP32Wiimote::task(void)
{
  if(!btStarted()){
    return;
  }
  handleTxQueue();
  handleRxQueue();
}

int ESP32Wiimote::available(void)
{
    int offs = 0;
    int buttonIsChanged = false;
//  int nunchukButtonIsChanged = false;
    int accelIsChanged = false;
    int nunchukStickIsChanged = false;
    uint8_t cBtn = 0;
    uint8_t zBtn = 0;

    if (! TinyWiimoteAvailable())
        return 0;

    TinyWiimoteData rd = TinyWiimoteRead();

    if (rd.len < 4) // 
        return 0;
    if (rd.data[0] != 0xA1) // no data input
        return 0;
      
    // update old states
    _oldButtonState  = _buttonState;
    _oldAccelState   = _accelState;
    _oldNunchukState = _nunchukState;

    if ((rd.data[1] >= 0x30) && (rd.data[1] <= 0x37)) // data report with button data
        offs = 2;

    if (offs) // update button state
        _buttonState = (ButtonState)((rd.data[offs + 0] << 8) | rd.data[offs + 1]);

    // get accelerometer offset
    switch (rd.data[1])
    {
    case 0x31: offs = 4; break; // Core Buttons and Accelerometer
    case 0x35: offs = 4; break; // Core Buttons and Accelerometer with 16 Extension Bytes
    default:   offs = 0;
    }

    if (offs) // update accelerometer
    {
        _accelState.xAxis  = rd.data[offs + 0];
        _accelState.yAxis  = rd.data[offs + 1];
        _accelState.zAxis  = rd.data[offs + 2];

        // check accel change
        if (_filter & FILTER_ACCEL) {
            ; // ignore
        }
        else {
            accelIsChanged = true;
        }
    }
    else
    {
        _accelState.xAxis  = 0;
        _accelState.yAxis  = 0;
        _accelState.zAxis  = 0;
    }

    // get extension offset
    switch (rd.data[1])
    {
    case 0x32: offs = 4; break; // Core Buttons with 8 Extension bytes
    case 0x35: offs = 7; break; // Core Buttons and Accelerometer with 16 Extension Bytes
    default:   offs = 0;
    }

    if (offs) // update nunchuk state
    {
        _nunchukState.xStick = rd.data[offs + 0];
        _nunchukState.yStick = rd.data[offs + 1];
        _nunchukState.xAxis  = rd.data[offs + 2];
        _nunchukState.yAxis  = rd.data[offs + 3];
        _nunchukState.zAxis  = rd.data[offs + 4];

        // update nunchuk buttons
        cBtn = ((rd.data[offs + 5] & 0x02) >> 1) ^ 0x01;
        zBtn =  (rd.data[offs + 5] & 0x01)       ^ 0x01;
    }
    else
    {
        _nunchukState.xStick = 0;
        _nunchukState.yStick = 0;
        _nunchukState.xAxis  = 0;
        _nunchukState.yAxis  = 0;
        _nunchukState.zAxis  = 0;
    }
    

    // add nunchuk buttons
    if (cBtn)
        _buttonState = (ButtonState)((int)_buttonState | BUTTON_C);
    if (zBtn)
        _buttonState = (ButtonState)((int)_buttonState | BUTTON_Z);

    // check button change
    if (_filter & FILTER_BUTTON) {
        ; // ignore
    }
    else if (_buttonState != _oldButtonState) {
        buttonIsChanged = true;
    }

    // check nunchuk stick change
    if (offs)
    {
        int nunXStickDelta = (int)(_nunchukState.xStick) - _oldNunchukState.xStick;
        int nunYStickDelta = (int)(_nunchukState.yStick) - _oldNunchukState.yStick;
        int nunStickDelta = (nunXStickDelta*nunXStickDelta + nunYStickDelta*nunYStickDelta);
        if (_filter & FILTER_NUNCHUK_STICK) {
            ; // ignore
        }
        else if (nunStickDelta >= _nunStickThreshold) {
            nunchukStickIsChanged = true;
        }

//      // check nunchuk button change
//      if (_filter & FILTER_NUNCHUK_BUTTON) {
//          ; // ignore
//      }
//      else if (
//        (_pNunchukState->cBtn != _pOldNunchukState->cBtn)
//        || (_pNunchukState->zBtn != _pOldNunchukState->zBtn)
//        ) {
//          nunchukButtonIsChanged = true;
//      }

        // check accel change
        if (_filter & FILTER_ACCEL) {
            ; // ignore
        }
        else {
            accelIsChanged = true;
        }
    }

    return
        ( buttonIsChanged
        | nunchukStickIsChanged
//      | nunchukButtonIsChanged
        | accelIsChanged
        );
}

ButtonState ESP32Wiimote::getButtonState(void)
{
  return _buttonState;
}

AccelState ESP32Wiimote::getAccelState(void)
{
    return _accelState;
}

NunchukState ESP32Wiimote::getNunchukState(void)
{
  return _nunchukState;
}

void ESP32Wiimote::addFilter(int action, int filter) {
  if (action == ACTION_IGNORE) {
    _filter = _filter | filter;

    if (filter & FILTER_ACCEL)
      TinyWiimoteReqAccelerometer(false);
  }
}

