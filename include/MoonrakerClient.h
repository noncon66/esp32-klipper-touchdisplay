#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>

#include "PrinterState.h"

class MoonrakerClient {
 public:
  void begin(PrinterState &state);
  void loop(uint32_t now);
  bool runAction(PrinterAction action);
  bool canRunAction(PrinterAction action) const;

 private:
  static constexpr uint32_t wifiRetryMs = 10000;
  static constexpr uint32_t serverInfoRequestId = 1;
  static constexpr uint32_t objectListRequestId = 2;
  static constexpr uint32_t subscribeRequestId = 3;
  static constexpr uint32_t actionTimeoutMs = 15000;
  static MoonrakerClient *activeInstance_;

  WebSocketsClient webSocket_;
  PrinterState *state_ = nullptr;
  String extraHeaders_;
  bool websocketStarted_ = false;
  uint32_t lastWifiAttempt_ = 0;
  uint32_t nextActionRequestId_ = 100;
  uint32_t pendingActionRequestId_ = 0;
  uint32_t actionSentMs_ = 0;

  void startWifi(uint32_t now);
  void startWebsocket();
  void handleWebsocketEvent(WStype_t type, uint8_t *payload, size_t length);
  void handleMessage(uint8_t *payload, size_t length);
  void requestServerInfo();
  void requestObjectList();
  void subscribeToAvailableObjects(JsonArrayConst objects);
  void applyStatus(JsonObjectConst status);
  void applyKlipperState(const char *state);
  void failPendingAction(const char *message);
  static void websocketEvent(WStype_t type, uint8_t *payload, size_t length);
};
