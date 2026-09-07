#pragma once

#include <lvgl.h>

#include "BoardHardware.h"
#include "PrinterState.h"

class HardwareTestUi {
 public:
  void begin(BoardHardware &board, PrinterState &printerState);
  void update(uint32_t now);

 private:
  BoardHardware *board_ = nullptr;
  PrinterState *printerState_ = nullptr;
  lv_obj_t *printerScreen_ = nullptr;
  lv_obj_t *testScreen_ = nullptr;
  lv_obj_t *systemScreen_ = nullptr;
  lv_obj_t *statusLabel_ = nullptr;
  lv_obj_t *coordinateLabel_ = nullptr;
  lv_obj_t *counterLabel_ = nullptr;
  lv_obj_t *systemTouchLabel_ = nullptr;
  lv_obj_t *systemMemoryLabel_ = nullptr;
  lv_obj_t *systemUptimeLabel_ = nullptr;
  lv_obj_t *connectionLabel_ = nullptr;
  lv_obj_t *extruderLabel_ = nullptr;
  lv_obj_t *bedLabel_ = nullptr;
  lv_obj_t *printLabel_ = nullptr;
  lv_obj_t *filenameLabel_ = nullptr;
  lv_obj_t *freshnessLabel_ = nullptr;
  lv_obj_t *progressBar_ = nullptr;
  lv_obj_t *targets_[4]{};
  uint32_t clicks_ = 0;
  uint32_t lastUpdate_ = 0;

  void createPrinterScreen();
  void createTestScreen();
  void createSystemScreen();
  void resetTargets();
  lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y);
  lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y,
                   int width, int height, lv_event_cb_t callback);
  static HardwareTestUi *fromEvent(lv_event_t *event);
  static void targetPressed(lv_event_t *event);
  static void brightnessChanged(lv_event_t *event);
  static void blinkLight(lv_event_t *event);
  static void resetPressed(lv_event_t *event);
  static void showPrinter(lv_event_t *event);
  static void showSystem(lv_event_t *event);
  static void showTest(lv_event_t *event);
};
