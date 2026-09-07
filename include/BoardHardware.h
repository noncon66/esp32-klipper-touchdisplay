#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

#include "SafeGT911.h"

class BoardHardware {
 public:
  static constexpr uint16_t width = 480;
  static constexpr uint16_t height = 480;
  static constexpr uint16_t bufferLines = 30;

  BoardHardware();

  bool begin();
  void service(uint32_t now);
  void setBrightness(uint8_t value);
  void blackoutFor(uint32_t durationMs);

  bool touchOnline() const { return touch_.online; }
  bool touchPressed() const { return touch_.pressed; }
  uint16_t touchX() const { return touch_.x; }
  uint16_t touchY() const { return touch_.y; }
  uint8_t touchAddress() const { return touch_.address; }
  uint32_t touchErrors() const { return touch_.errors; }
  uint8_t brightness() const { return brightness_; }

 private:
  static constexpr uint8_t backlightPin = 38;
  static BoardHardware *activeInstance_;

  Arduino_ESP32RGBPanel bus_;
  Arduino_ST7701_RGBPanel gfx_;
  SafeGT911 touch_;
  lv_disp_draw_buf_t drawBuffer_{};
  lv_disp_drv_t displayDriver_{};
  lv_indev_drv_t inputDriver_{};
  uint32_t lastTouchProbe_ = 0;
  uint32_t lightRestoreAt_ = 0;
  uint8_t brightness_ = 255;

  static void flushDisplay(lv_disp_drv_t *driver, const lv_area_t *area,
                           lv_color_t *pixels);
  static void readTouch(lv_indev_drv_t *driver, lv_indev_data_t *data);
};
