#include "HardwareTestUi.h"

#include <Arduino.h>

namespace {
void styleScreen(lv_obj_t *screen) {
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x101B2B), 0);
  lv_obj_set_style_text_color(screen, lv_color_hex(0xF4F7FB), 0);
}
}  // namespace

void HardwareTestUi::begin(BoardHardware &board) {
  board_ = &board;
  createTestScreen();
  createSystemScreen();
  lv_scr_load(testScreen_);
}

void HardwareTestUi::update(uint32_t now) {
  if (uint32_t(now - lastUpdate_) < 100) return;
  lastUpdate_ = now;

  lv_label_set_text_fmt(statusLabel_, "GT911: %s  |  I2C-Fehler: %lu",
                        board_->touchOnline() ? "verbunden" : "nicht erkannt",
                        static_cast<unsigned long>(board_->touchErrors()));
  lv_label_set_text_fmt(coordinateLabel_, "X: %u   Y: %u   %s",
                        board_->touchX(), board_->touchY(),
                        board_->touchPressed() ? "beruehrt" : "losgelassen");
  lv_label_set_text_fmt(systemTouchLabel_,
                        "GT911: %s\nAdresse: 0x%02X\nI2C-Fehler: %lu",
                        board_->touchOnline() ? "verbunden" : "nicht erkannt",
                        board_->touchAddress(),
                        static_cast<unsigned long>(board_->touchErrors()));
  lv_label_set_text_fmt(systemMemoryLabel_,
                        "Flash: %u Bytes\nPSRAM: %u Bytes\nHeap frei: %u Bytes\nPSRAM frei: %u Bytes",
                        ESP.getFlashChipSize(), ESP.getPsramSize(),
                        ESP.getFreeHeap(), ESP.getFreePsram());
  lv_label_set_text_fmt(systemUptimeLabel_,
                        "Uptime: %lu s\nHelligkeit: %u / 255",
                        static_cast<unsigned long>(now / 1000),
                        board_->brightness());
}

void HardwareTestUi::createTestScreen() {
  testScreen_ = lv_obj_create(nullptr);
  styleScreen(testScreen_);
  label(testScreen_, "DISPLAY + TOUCHTEST", 118, 26);
  button(testScreen_, "SYSTEM", 190, 61, 100, 38, showSystem);
  statusLabel_ = label(testScreen_, "Touch wird geprueft ...", 24, 108);
  coordinateLabel_ = label(testScreen_, "X: ---   Y: ---   losgelassen", 24, 138);
  counterLabel_ = label(testScreen_, "Tastendruecke: 0", 24, 166);

  const uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};
  const char *names[] = {"ROT", "GRUEN", "BLAU", "WEISS"};
  for (int i = 0; i < 4; ++i) {
    lv_obj_t *tile = lv_obj_create(testScreen_);
    lv_obj_set_pos(tile, 24 + i * 110, 205);
    lv_obj_set_size(tile, 100, 52);
    lv_obj_set_style_bg_color(tile, lv_color_hex(colors[i]), 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    label(testScreen_, names[i], 30 + i * 110, 265);
  }

  label(testScreen_, "Helligkeit", 24, 303);
  lv_obj_t *slider = lv_slider_create(testScreen_);
  lv_obj_set_pos(slider, 142, 306);
  lv_obj_set_size(slider, 290, 20);
  lv_slider_set_range(slider, 25, 255);
  lv_slider_set_value(slider, 255, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider, brightnessChanged, LV_EVENT_VALUE_CHANGED, this);
  button(testScreen_, "Licht aus: 1,5 s", 24, 349, 208, 44, blinkLight);
  button(testScreen_, "Zaehler zurueck", 248, 349, 208, 44, resetPressed);
  targets_[0] = button(testScreen_, "OL", 8, 8, 78, 72, targetPressed);
  targets_[1] = button(testScreen_, "OR", 394, 8, 78, 72, targetPressed);
  targets_[2] = button(testScreen_, "UL", 8, 400, 78, 72, targetPressed);
  targets_[3] = button(testScreen_, "UR", 394, 400, 78, 72, targetPressed);
  label(testScreen_, "Vier Ecktasten testen", 130, 433);
}

void HardwareTestUi::createSystemScreen() {
  systemScreen_ = lv_obj_create(nullptr);
  styleScreen(systemScreen_);
  label(systemScreen_, "SYSTEMDIAGNOSE", 155, 30);
  label(systemScreen_, "Firmware 0.2.0", 178, 66);
  systemTouchLabel_ = label(systemScreen_, "Touch wird gelesen ...", 38, 120);
  systemMemoryLabel_ = label(systemScreen_, "Speicher wird gelesen ...", 230, 120);
  systemUptimeLabel_ = label(systemScreen_, "Laufzeit wird gelesen ...", 38, 260);
  button(systemScreen_, "< ZURUECK ZUM TEST", 110, 376, 260, 58, showTest);
}

void HardwareTestUi::resetTargets() {
  clicks_ = 0;
  lv_label_set_text(counterLabel_, "Tastendruecke: 0");
  for (auto *target : targets_) {
    lv_obj_set_style_bg_color(target, lv_color_hex(0x285AB4), 0);
  }
}

lv_obj_t *HardwareTestUi::label(lv_obj_t *parent, const char *text,
                                int x, int y) {
  lv_obj_t *object = lv_label_create(parent);
  lv_label_set_text(object, text);
  lv_obj_set_pos(object, x, y);
  return object;
}

lv_obj_t *HardwareTestUi::button(lv_obj_t *parent, const char *text,
                                 int x, int y, int width, int height,
                                 lv_event_cb_t callback) {
  lv_obj_t *object = lv_btn_create(parent);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, width, height);
  lv_obj_add_event_cb(object, callback, LV_EVENT_CLICKED, this);
  lv_obj_t *caption = lv_label_create(object);
  lv_label_set_text(caption, text);
  lv_obj_center(caption);
  return object;
}

HardwareTestUi *HardwareTestUi::fromEvent(lv_event_t *event) {
  return static_cast<HardwareTestUi *>(lv_event_get_user_data(event));
}

void HardwareTestUi::targetPressed(lv_event_t *event) {
  auto *ui = fromEvent(event);
  lv_obj_set_style_bg_color(lv_event_get_target(event),
                            lv_color_hex(0x087F5B), 0);
  ++ui->clicks_;
  lv_label_set_text_fmt(ui->counterLabel_, "Tastendruecke: %lu",
                        static_cast<unsigned long>(ui->clicks_));
}

void HardwareTestUi::brightnessChanged(lv_event_t *event) {
  auto *ui = fromEvent(event);
  ui->board_->setBrightness(
      static_cast<uint8_t>(lv_slider_get_value(lv_event_get_target(event))));
}

void HardwareTestUi::blinkLight(lv_event_t *event) {
  fromEvent(event)->board_->blackoutFor(1500);
}

void HardwareTestUi::resetPressed(lv_event_t *event) {
  fromEvent(event)->resetTargets();
}

void HardwareTestUi::showSystem(lv_event_t *event) {
  auto *ui = fromEvent(event);
  lv_scr_load_anim(ui->systemScreen_, LV_SCR_LOAD_ANIM_MOVE_LEFT,
                   220, 0, false);
}

void HardwareTestUi::showTest(lv_event_t *event) {
  auto *ui = fromEvent(event);
  lv_scr_load_anim(ui->testScreen_, LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                   220, 0, false);
}
