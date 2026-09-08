#pragma once

#include <lvgl.h>

#include "BoardHardware.h"
#include "MoonrakerClient.h"
#include "PrinterState.h"

class HardwareTestUi {
 public:
  void begin(BoardHardware &board, PrinterState &printerState,
             MoonrakerClient &moonraker);
  void update(uint32_t now);

 private:
  BoardHardware *board_ = nullptr;
  PrinterState *printerState_ = nullptr;
  MoonrakerClient *moonraker_ = nullptr;
  lv_obj_t *printerScreen_ = nullptr;
  lv_obj_t *actionsScreen_ = nullptr;
  lv_obj_t *everydayScreen_ = nullptr;
  lv_obj_t *bedLevelScreen_ = nullptr;
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
  lv_obj_t *actionButtons_[3]{};
  lv_obj_t *firmwareRestartButton_ = nullptr;
  lv_obj_t *actionFeedbackLabel_ = nullptr;
  lv_obj_t *everydayActionButtons_[5]{};
  lv_obj_t *everydayStateLabel_ = nullptr;
  lv_obj_t *everydayFeedbackLabel_ = nullptr;
  lv_obj_t *bedLevelStatusLabel_ = nullptr;
  lv_obj_t *bedLevelButtons_[3]{};
  lv_obj_t *bedLevelBackButton_ = nullptr;
  lv_obj_t *confirmPanel_ = nullptr;
  lv_obj_t *confirmLabel_ = nullptr;
  PrinterAction confirmationAction_ = PrinterAction::None;
  lv_obj_t *targets_[4]{};
  uint32_t clicks_ = 0;
  uint32_t lastUpdate_ = 0;

  void createPrinterScreen();
  void createActionsScreen();
  void createEverydayScreen();
  void createBedLevelScreen();
  void createTestScreen();
  void createSystemScreen();
  void resetTargets();
  void showActionConfirmation(PrinterAction action);
  lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y);
  lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y,
                   int width, int height, lv_event_cb_t callback);
  static HardwareTestUi *fromEvent(lv_event_t *event);
  static void targetPressed(lv_event_t *event);
  static void brightnessChanged(lv_event_t *event);
  static void blinkLight(lv_event_t *event);
  static void resetPressed(lv_event_t *event);
  static void showPrinter(lv_event_t *event);
  static void showActions(lv_event_t *event);
  static void showEveryday(lv_event_t *event);
  static void showBedLevel(lv_event_t *event);
  static void showSystem(lv_event_t *event);
  static void showTest(lv_event_t *event);
  static void preheatPlaPressed(lv_event_t *event);
  static void preheatPetgPressed(lv_event_t *event);
  static void cooldownPressed(lv_event_t *event);
  static void firmwareRestartPressed(lv_event_t *event);
  static void loadPlaPressed(lv_event_t *event);
  static void loadPetgPressed(lv_event_t *event);
  static void unloadPlaPressed(lv_event_t *event);
  static void homeAllPressed(lv_event_t *event);
  static void bedLevelStartPressed(lv_event_t *event);
  static void bedLevelAdjustedPressed(lv_event_t *event);
  static void bedLevelAcceptPressed(lv_event_t *event);
  static void bedLevelAbortPressed(lv_event_t *event);
  static void confirmAction(lv_event_t *event);
  static void cancelAction(lv_event_t *event);
};
