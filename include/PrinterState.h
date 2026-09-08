#pragma once

#include <Arduino.h>

enum class LinkState : uint8_t {
  NotConfigured,
  Disconnected,
  Connecting,
  Connected,
};

enum class KlipperState : uint8_t {
  Unknown,
  Startup,
  Ready,
  Shutdown,
  Error,
};

enum class PrintState : uint8_t {
  Unknown,
  Standby,
  Printing,
  Paused,
  Complete,
  Cancelled,
  Error,
};

enum class PrinterAction : uint8_t {
  None,
  PreheatPla,
  PreheatPetg,
  Cooldown,
  FirmwareRestart,
};

enum class ActionState : uint8_t {
  Idle,
  Pending,
  Succeeded,
  Failed,
};

struct TemperatureState {
  float actual = 0.0f;
  float target = 0.0f;
  bool actualValid = false;
  bool targetValid = false;
};

struct PrinterState {
  LinkState wifi = LinkState::NotConfigured;
  LinkState moonraker = LinkState::NotConfigured;
  KlipperState klipper = KlipperState::Unknown;
  PrintState print = PrintState::Unknown;
  TemperatureState extruder;
  TemperatureState bed;
  String filename;
  float progress = 0.0f;
  bool filenameValid = false;
  bool progressValid = false;
  bool stale = true;
  uint32_t lastUpdateMs = 0;
  uint32_t protocolErrors = 0;
  PrinterAction action = PrinterAction::None;
  ActionState actionState = ActionState::Idle;
  String actionMessage = "Noch keine Aktion";
  uint32_t actionUpdatedMs = 0;

  void invalidateLiveData();
};

const char *linkStateText(LinkState state);
const char *klipperStateText(KlipperState state);
const char *printStateText(PrintState state);
const char *printerActionText(PrinterAction action);
