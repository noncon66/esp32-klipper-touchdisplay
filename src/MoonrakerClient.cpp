#include "MoonrakerClient.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
#include <initializer_list>

#include "NetworkConfig.h"

MoonrakerClient *MoonrakerClient::activeInstance_ = nullptr;

namespace {
bool hasObject(JsonArrayConst objects, const char *name) {
  for (JsonVariantConst object : objects) {
    if (object.is<const char *>() &&
        std::strcmp(object.as<const char *>(), name) == 0) {
      return true;
    }
  }
  return false;
}

void addFields(JsonObject objects, const char *name,
               std::initializer_list<const char *> fields) {
  JsonArray values = objects.createNestedArray(name);
  for (const char *field : fields) values.add(field);
}

PrintState parsePrintState(const char *value) {
  if (!value) return PrintState::Unknown;
  if (std::strcmp(value, "standby") == 0) return PrintState::Standby;
  if (std::strcmp(value, "printing") == 0) return PrintState::Printing;
  if (std::strcmp(value, "paused") == 0) return PrintState::Paused;
  if (std::strcmp(value, "complete") == 0) return PrintState::Complete;
  if (std::strcmp(value, "cancelled") == 0) return PrintState::Cancelled;
  if (std::strcmp(value, "error") == 0) return PrintState::Error;
  return PrintState::Unknown;
}
}  // namespace

void MoonrakerClient::begin(PrinterState &state) {
  activeInstance_ = this;
  state_ = &state;
  if (!NetworkConfig::isComplete()) {
    state_->wifi = LinkState::NotConfigured;
    state_->moonraker = LinkState::NotConfigured;
    state_->invalidateLiveData();
    Serial.println("Netzwerk: include/secrets.h ist noch nicht vollstaendig.");
    return;
  }
  state_->wifi = LinkState::Disconnected;
  state_->moonraker = LinkState::Disconnected;
  startWifi(millis());
}

void MoonrakerClient::loop(uint32_t now) {
  if (!NetworkConfig::isComplete()) return;

  if (state_->actionState == ActionState::Pending &&
      uint32_t(now - actionSentMs_) >= actionTimeoutMs) {
    failPendingAction("Zeitueberschreitung ohne Moonraker-Antwort");
  }

  if (WiFi.status() != WL_CONNECTED) {
    if (state_->wifi == LinkState::Connected || websocketStarted_) {
      webSocket_.disconnect();
      websocketStarted_ = false;
      state_->moonraker = LinkState::Disconnected;
      state_->klipper = KlipperState::Unknown;
      state_->invalidateLiveData();
    }
    if (state_->wifi != LinkState::Connecting ||
        uint32_t(now - lastWifiAttempt_) >= wifiRetryMs) {
      startWifi(now);
    }
    return;
  }

  if (state_->wifi != LinkState::Connected) {
    state_->wifi = LinkState::Connected;
    Serial.printf("WLAN verbunden, IP %s\n",
                  WiFi.localIP().toString().c_str());
  }
  if (!websocketStarted_) startWebsocket();
  webSocket_.loop();
}

void MoonrakerClient::startWifi(uint32_t now) {
  lastWifiAttempt_ = now;
  state_->wifi = LinkState::Connecting;
  state_->moonraker = LinkState::Disconnected;
  state_->invalidateLiveData();
  WiFi.mode(WIFI_STA);
  WiFi.begin(NetworkConfig::wifiSsid, NetworkConfig::wifiPassword);
  Serial.println("WLAN-Verbindung wird aufgebaut.");
}

void MoonrakerClient::startWebsocket() {
  websocketStarted_ = true;
  state_->moonraker = LinkState::Connecting;
  webSocket_.onEvent(websocketEvent);
  webSocket_.setReconnectInterval(5000);
  if (std::strlen(NetworkConfig::moonrakerApiKey) > 0) {
    extraHeaders_ = "X-Api-Key: ";
    extraHeaders_ += NetworkConfig::moonrakerApiKey;
    extraHeaders_ += "\r\n";
    webSocket_.setExtraHeaders(extraHeaders_.c_str());
  } else {
    // arduinoWebSockets defaults to "Origin: file://". Moonraker rejects
    // that origin with HTTP 403, so omit the optional header entirely.
    webSocket_.setExtraHeaders(nullptr);
  }
  webSocket_.begin(NetworkConfig::moonrakerHost,
                   NetworkConfig::moonrakerPort, "/websocket", "");
  Serial.printf("Moonraker WebSocket: ws://%s:%u/websocket\n",
                NetworkConfig::moonrakerHost,
                NetworkConfig::moonrakerPort);
}

void MoonrakerClient::websocketEvent(WStype_t type, uint8_t *payload,
                                     size_t length) {
  if (activeInstance_) {
    activeInstance_->handleWebsocketEvent(type, payload, length);
  }
}

void MoonrakerClient::handleWebsocketEvent(WStype_t type, uint8_t *payload,
                                           size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      state_->moonraker = LinkState::Connected;
      state_->stale = true;
      Serial.println("Moonraker verbunden.");
      requestServerInfo();
      break;
    case WStype_DISCONNECTED:
      if (state_->actionState == ActionState::Pending) {
        failPendingAction("Verbindung waehrend der Aktion getrennt");
      }
      state_->moonraker = LinkState::Disconnected;
      state_->klipper = KlipperState::Unknown;
      state_->invalidateLiveData();
      Serial.println("Moonraker getrennt.");
      break;
    case WStype_TEXT:
      handleMessage(payload, length);
      break;
    case WStype_ERROR:
      ++state_->protocolErrors;
      break;
    default:
      break;
  }
}

void MoonrakerClient::handleMessage(uint8_t *payload, size_t length) {
  const size_t capacity = length + length / 2 + 2048;
  if (capacity > 49152) {
    ++state_->protocolErrors;
    Serial.printf("Moonraker-Nachricht zu gross: %u Bytes\n",
                  static_cast<unsigned>(length));
    return;
  }

  DynamicJsonDocument document(capacity);
  const DeserializationError error = deserializeJson(document, payload, length);
  if (error) {
    ++state_->protocolErrors;
    Serial.printf("Moonraker-JSON ungueltig: %s\n", error.c_str());
    return;
  }

  const uint32_t id = document["id"] | 0;
  if (id == pendingActionRequestId_ &&
      state_->actionState == ActionState::Pending) {
    if (document.containsKey("error")) {
      const char *message = document["error"]["message"] | "Moonraker-Fehler";
      failPendingAction(message);
    } else {
      state_->actionState = ActionState::Succeeded;
      state_->actionMessage = printerActionText(state_->action);
      state_->actionMessage += " angenommen";
      state_->actionUpdatedMs = millis();
      pendingActionRequestId_ = 0;
      Serial.printf("Aktion bestaetigt: %s\n",
                    printerActionText(state_->action));
    }
    return;
  }

  if (document.containsKey("error")) {
    ++state_->protocolErrors;
    const char *message = document["error"]["message"] | "unbekannt";
    Serial.printf("Moonraker-Fehler fuer Anfrage %lu: %s\n",
                  static_cast<unsigned long>(id), message);
    return;
  }
  if (id == serverInfoRequestId) {
    const char *klippyState = document["result"]["klippy_state"] | "unknown";
    applyKlipperState(klippyState);
    if (state_->klipper == KlipperState::Ready) requestObjectList();
    return;
  }
  if (id == objectListRequestId) {
    JsonArrayConst objects = document["result"]["objects"].as<JsonArrayConst>();
    if (!objects.isNull()) subscribeToAvailableObjects(objects);
    return;
  }
  if (id == subscribeRequestId) {
    JsonObjectConst status = document["result"]["status"].as<JsonObjectConst>();
    if (!status.isNull()) applyStatus(status);
    return;
  }

  const char *method = document["method"] | "";
  if (std::strcmp(method, "notify_status_update") == 0) {
    JsonObjectConst status = document["params"][0].as<JsonObjectConst>();
    if (!status.isNull()) applyStatus(status);
  } else if (std::strcmp(method, "notify_klippy_ready") == 0) {
    state_->klipper = KlipperState::Ready;
    requestObjectList();
  } else if (std::strcmp(method, "notify_klippy_shutdown") == 0) {
    state_->klipper = KlipperState::Shutdown;
    state_->invalidateLiveData();
  } else if (std::strcmp(method, "notify_klippy_disconnected") == 0) {
    state_->klipper = KlipperState::Unknown;
    state_->invalidateLiveData();
  }
}

void MoonrakerClient::requestServerInfo() {
  webSocket_.sendTXT(
      "{\"jsonrpc\":\"2.0\",\"method\":\"server.info\",\"id\":1}");
}

void MoonrakerClient::requestObjectList() {
  webSocket_.sendTXT(
      "{\"jsonrpc\":\"2.0\",\"method\":\"printer.objects.list\",\"id\":2}");
}

void MoonrakerClient::subscribeToAvailableObjects(JsonArrayConst available) {
  StaticJsonDocument<1024> document;
  document["jsonrpc"] = "2.0";
  document["method"] = "printer.objects.subscribe";
  document["id"] = subscribeRequestId;
  JsonObject objects = document.createNestedObject("params")
                                   .createNestedObject("objects");

  if (hasObject(available, "extruder")) {
    addFields(objects, "extruder", {"temperature", "target"});
  }
  if (hasObject(available, "heater_bed")) {
    addFields(objects, "heater_bed", {"temperature", "target"});
  }
  if (hasObject(available, "print_stats")) {
    addFields(objects, "print_stats", {"state", "filename"});
  }
  if (hasObject(available, "virtual_sdcard")) {
    addFields(objects, "virtual_sdcard", {"progress"});
  }
  if (hasObject(available, "webhooks")) {
    addFields(objects, "webhooks", {"state"});
  }

  String message;
  serializeJson(document, message);
  webSocket_.sendTXT(message);
  Serial.println("Moonraker-Statusobjekte abonniert.");
}

void MoonrakerClient::applyStatus(JsonObjectConst status) {
  JsonObjectConst extruder = status["extruder"].as<JsonObjectConst>();
  if (!extruder.isNull()) {
    if (extruder.containsKey("temperature")) {
      state_->extruder.actual = extruder["temperature"].as<float>();
      state_->extruder.actualValid = true;
    }
    if (extruder.containsKey("target")) {
      state_->extruder.target = extruder["target"].as<float>();
      state_->extruder.targetValid = true;
    }
  }

  JsonObjectConst bed = status["heater_bed"].as<JsonObjectConst>();
  if (!bed.isNull()) {
    if (bed.containsKey("temperature")) {
      state_->bed.actual = bed["temperature"].as<float>();
      state_->bed.actualValid = true;
    }
    if (bed.containsKey("target")) {
      state_->bed.target = bed["target"].as<float>();
      state_->bed.targetValid = true;
    }
  }

  JsonObjectConst printStats = status["print_stats"].as<JsonObjectConst>();
  if (!printStats.isNull()) {
    if (printStats.containsKey("state")) {
      state_->print = parsePrintState(printStats["state"].as<const char *>());
    }
    if (printStats.containsKey("filename")) {
      state_->filename = printStats["filename"].as<const char *>();
      state_->filenameValid = true;
    }
  }

  JsonObjectConst virtualSd = status["virtual_sdcard"].as<JsonObjectConst>();
  if (!virtualSd.isNull() && virtualSd.containsKey("progress")) {
    state_->progress = virtualSd["progress"].as<float>();
    state_->progressValid = true;
  }

  JsonObjectConst webhooks = status["webhooks"].as<JsonObjectConst>();
  if (!webhooks.isNull() && webhooks.containsKey("state")) {
    applyKlipperState(webhooks["state"].as<const char *>());
  }

  state_->stale = false;
  state_->lastUpdateMs = millis();
}

void MoonrakerClient::applyKlipperState(const char *value) {
  if (!value) {
    state_->klipper = KlipperState::Unknown;
  } else if (std::strcmp(value, "ready") == 0) {
    state_->klipper = KlipperState::Ready;
  } else if (std::strcmp(value, "startup") == 0) {
    state_->klipper = KlipperState::Startup;
  } else if (std::strcmp(value, "shutdown") == 0) {
    state_->klipper = KlipperState::Shutdown;
    state_->invalidateLiveData();
  } else if (std::strcmp(value, "error") == 0) {
    state_->klipper = KlipperState::Error;
    state_->invalidateLiveData();
  } else {
    state_->klipper = KlipperState::Unknown;
  }
}

bool MoonrakerClient::runAction(PrinterAction action) {
  if (!canRunAction(action)) {
    state_->action = action;
    state_->actionState = ActionState::Failed;
    state_->actionMessage = "Aktion im aktuellen Zustand gesperrt";
    state_->actionUpdatedMs = millis();
    return false;
  }

  const char *script = nullptr;
  switch (action) {
    case PrinterAction::PreheatPla: script = "PREHEAT_PLA"; break;
    case PrinterAction::PreheatPetg: script = "PREHEAT_PETG"; break;
    case PrinterAction::Cooldown: script = "COOLDOWN"; break;
    case PrinterAction::FirmwareRestart: break;
    case PrinterAction::None: return false;
  }

  StaticJsonDocument<256> document;
  document["jsonrpc"] = "2.0";
  pendingActionRequestId_ = nextActionRequestId_++;
  document["id"] = pendingActionRequestId_;
  if (action == PrinterAction::FirmwareRestart) {
    document["method"] = "printer.firmware_restart";
  } else {
    document["method"] = "printer.gcode.script";
    document.createNestedObject("params")["script"] = script;
  }

  String message;
  serializeJson(document, message);
  if (!webSocket_.sendTXT(message)) {
    pendingActionRequestId_ = 0;
    state_->action = action;
    state_->actionState = ActionState::Failed;
    state_->actionMessage = "Aktion konnte nicht gesendet werden";
    state_->actionUpdatedMs = millis();
    return false;
  }

  state_->action = action;
  state_->actionState = ActionState::Pending;
  state_->actionMessage = printerActionText(action);
  state_->actionMessage += " wird ausgefuehrt";
  state_->actionUpdatedMs = millis();
  actionSentMs_ = state_->actionUpdatedMs;
  Serial.printf("Aktion gesendet: %s\n",
                action == PrinterAction::FirmwareRestart
                    ? "printer.firmware_restart"
                    : script);
  return true;
}

bool MoonrakerClient::canRunAction(PrinterAction action) const {
  if (!state_ || action == PrinterAction::None ||
      state_->moonraker != LinkState::Connected ||
      state_->actionState == ActionState::Pending) {
    return false;
  }

  if (action == PrinterAction::FirmwareRestart) {
    return state_->klipper == KlipperState::Shutdown ||
           state_->klipper == KlipperState::Error;
  }

  if (state_->klipper != KlipperState::Ready || state_->stale) return false;
  return state_->print != PrintState::Unknown &&
         state_->print != PrintState::Printing &&
         state_->print != PrintState::Paused;
}

void MoonrakerClient::failPendingAction(const char *message) {
  state_->actionState = ActionState::Failed;
  state_->actionMessage = message;
  state_->actionUpdatedMs = millis();
  pendingActionRequestId_ = 0;
  Serial.printf("Aktion fehlgeschlagen: %s\n", message);
}
