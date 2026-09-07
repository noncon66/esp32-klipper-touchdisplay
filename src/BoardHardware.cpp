#include "BoardHardware.h"

BoardHardware *BoardHardware::activeInstance_ = nullptr;

BoardHardware::BoardHardware()
    : bus_(39, 48, 47, 18, 17, 16, 21,
           11, 12, 13, 14, 0, 8, 20, 3, 46, 9, 10, 4, 5, 6, 7, 15),
      gfx_(&bus_, GFX_NOT_DEFINED, 0, true, width, height,
           st7701_type1_init_operations,
           sizeof(st7701_type1_init_operations), true,
           10, 8, 50, 10, 8, 20) {}

bool BoardHardware::begin() {
  activeInstance_ = this;
  pinMode(backlightPin, OUTPUT);
  digitalWrite(backlightPin, HIGH);

  if (!psramFound()) {
    Serial.println("STOP: PSRAM fehlt. OPI PSRAM waehlen.");
    return false;
  }

  gfx_.begin(16000000);
  if (!gfx_.getFramebuffer()) {
    Serial.println("STOP: RGB-Framebuffer fehlt.");
    return false;
  }
  gfx_.fillScreen(BLACK);

  ledcSetup(0, 1000, 8);
  ledcAttachPin(backlightPin, 0);
  ledcWrite(0, brightness_);
  touch_.begin();

  lv_init();
  auto *pixels = static_cast<lv_color_t *>(heap_caps_malloc(
      width * bufferLines * sizeof(lv_color_t),
      MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  if (!pixels) {
    Serial.println("STOP: LVGL-Zeichenpuffer fehlt.");
    return false;
  }

  lv_disp_draw_buf_init(&drawBuffer_, pixels, nullptr, width * bufferLines);
  lv_disp_drv_init(&displayDriver_);
  displayDriver_.hor_res = width;
  displayDriver_.ver_res = height;
  displayDriver_.flush_cb = flushDisplay;
  displayDriver_.draw_buf = &drawBuffer_;
  if (!lv_disp_drv_register(&displayDriver_)) {
    Serial.println("STOP: LVGL-Displayregistrierung fehlgeschlagen.");
    return false;
  }

  lv_indev_drv_init(&inputDriver_);
  inputDriver_.type = LV_INDEV_TYPE_POINTER;
  inputDriver_.read_cb = readTouch;
  if (!lv_indev_drv_register(&inputDriver_)) {
    Serial.println("STOP: LVGL-Touchregistrierung fehlgeschlagen.");
    return false;
  }
  return true;
}

void BoardHardware::service(uint32_t now) {
  if (lightRestoreAt_ && int32_t(now - lightRestoreAt_) >= 0) {
    lightRestoreAt_ = 0;
    ledcWrite(0, brightness_);
  }
  if (!touch_.online && uint32_t(now - lastTouchProbe_) >= 2000) {
    lastTouchProbe_ = now;
    touch_.probe();
  }
}

void BoardHardware::setBrightness(uint8_t value) {
  brightness_ = value;
  if (!lightRestoreAt_) ledcWrite(0, brightness_);
}

void BoardHardware::blackoutFor(uint32_t durationMs) {
  ledcWrite(0, 0);
  lightRestoreAt_ = millis() + durationMs;
}

void BoardHardware::flushDisplay(lv_disp_drv_t *driver,
                                 const lv_area_t *area,
                                 lv_color_t *pixels) {
  if (!activeInstance_) {
    lv_disp_flush_ready(driver);
    return;
  }
  activeInstance_->gfx_.draw16bitRGBBitmap(
      area->x1, area->y1, reinterpret_cast<uint16_t *>(pixels),
      area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
  lv_disp_flush_ready(driver);
}

void BoardHardware::readTouch(lv_indev_drv_t *, lv_indev_data_t *data) {
  if (!activeInstance_) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  activeInstance_->touch_.poll();
  data->state = activeInstance_->touch_.pressed
                    ? LV_INDEV_STATE_PR
                    : LV_INDEV_STATE_REL;
  data->point.x = activeInstance_->touch_.x;
  data->point.y = activeInstance_->touch_.y;
}
