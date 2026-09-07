#include "HardwareTestUi.h"

#include <Arduino.h>

namespace {
void styleScreen(lv_obj_t *screen) {
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x101B2B), 0);
  lv_obj_set_style_text_color(screen, lv_color_hex(0xF4F7FB), 0);
}
}  // namespace

void HardwareTestUi::begin(BoardHardware &board, PrinterState &printerState) {
  board_ = &board;
  printerState_ = &printerState;
  createPrinterScreen();
  createTestScreen();
  createSystemScreen();
  lv_scr_load(printerScreen_);
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

  lv_label_set_text_fmt(connectionLabel_,
                        "WLAN: %s\nMoonraker: %s\nKlipper: %s",
                        linkStateText(printerState_->wifi),
                        linkStateText(printerState_->moonraker),
                        klipperStateText(printerState_->klipper));
  if (printerState_->extruder.actualValid &&
      printerState_->extruder.targetValid) {
    lv_label_set_text_fmt(extruderLabel_, "HOTEND\n%d / %d C",
                          static_cast<int>(printerState_->extruder.actual + 0.5f),
                          static_cast<int>(printerState_->extruder.target + 0.5f));
  } else {
    lv_label_set_text(extruderLabel_, "HOTEND\n-- / -- C");
  }
  if (printerState_->bed.actualValid && printerState_->bed.targetValid) {
    lv_label_set_text_fmt(bedLabel_, "HEIZBETT\n%d / %d C",
                          static_cast<int>(printerState_->bed.actual + 0.5f),
                          static_cast<int>(printerState_->bed.target + 0.5f));
  } else {
    lv_label_set_text(bedLabel_, "HEIZBETT\n-- / -- C");
  }
  lv_label_set_text_fmt(printLabel_, "Druckstatus: %s",
                        printStateText(printerState_->print));
  lv_label_set_text(filenameLabel_,
                    printerState_->filenameValid && !printerState_->filename.isEmpty()
                        ? printerState_->filename.c_str()
                        : "Keine Druckdatei");
  int progress = printerState_->progressValid
                     ? static_cast<int>(printerState_->progress * 100.0f + 0.5f)
                     : 0;
  if (progress < 0) progress = 0;
  if (progress > 100) progress = 100;
  lv_bar_set_value(progressBar_, progress, LV_ANIM_OFF);
  if (printerState_->stale) {
    lv_label_set_text_fmt(freshnessLabel_,
                          "Daten nicht aktuell | Protokollfehler: %lu",
                          static_cast<unsigned long>(printerState_->protocolErrors));
  } else {
    lv_label_set_text_fmt(freshnessLabel_,
                          "Fortschritt: %d %% | Aktualisiert vor %lu s",
                          progress,
                          static_cast<unsigned long>(
                              (now - printerState_->lastUpdateMs) / 1000));
  }
}

void HardwareTestUi::createPrinterScreen() {
  printerScreen_ = lv_obj_create(nullptr);
  styleScreen(printerScreen_);
  label(printerScreen_, "KLIPPER STATUS", 168, 24);
  connectionLabel_ = label(printerScreen_, "Verbindung wird geprueft ...", 24, 62);
  extruderLabel_ = label(printerScreen_, "HOTEND\n-- / -- C", 24, 146);
  bedLabel_ = label(printerScreen_, "HEIZBETT\n-- / -- C", 260, 146);
  printLabel_ = label(printerScreen_, "Druckstatus: unbekannt", 24, 226);
  filenameLabel_ = label(printerScreen_, "Keine Druckdatei", 24, 260);
  lv_obj_set_width(filenameLabel_, 432);
  lv_label_set_long_mode(filenameLabel_, LV_LABEL_LONG_DOT);
  progressBar_ = lv_bar_create(printerScreen_);
  lv_obj_set_pos(progressBar_, 24, 304);
  lv_obj_set_size(progressBar_, 432, 24);
  lv_bar_set_range(progressBar_, 0, 100);
  freshnessLabel_ = label(printerScreen_, "Daten nicht aktuell", 24, 344);
  button(printerScreen_, "HARDWARETEST", 24, 400, 208, 48, showTest);
  button(printerScreen_, "SYSTEM", 248, 400, 208, 48, showSystem);
}

void HardwareTestUi::createTestScreen() {
  testScreen_ = lv_obj_create(nullptr);
  styleScreen(testScreen_);
  label(testScreen_, "DISPLAY + TOUCHTEST", 118, 26);
  button(testScreen_, "STATUS", 135, 61, 100, 38, showPrinter);
  button(testScreen_, "SYSTEM", 245, 61, 100, 38, showSystem);
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
  label(systemScreen_, "Firmware 0.3.0", 178, 66);
  systemTouchLabel_ = label(systemScreen_, "Touch wird gelesen ...", 38, 120);
  systemMemoryLabel_ = label(systemScreen_, "Speicher wird gelesen ...", 230, 120);
  systemUptimeLabel_ = label(systemScreen_, "Laufzeit wird gelesen ...", 38, 260);
  button(systemScreen_, "STATUS", 24, 376, 208, 58, showPrinter);
  button(systemScreen_, "HARDWARETEST", 248, 376, 208, 58, showTest);
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

void HardwareTestUi::showPrinter(lv_event_t *event) {
  auto *ui = fromEvent(event);
  lv_scr_load_anim(ui->printerScreen_, LV_SCR_LOAD_ANIM_FADE_ON,
                   180, 0, false);
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
