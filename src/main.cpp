#include <Arduino.h>
#include <esp_arduino_version.h>

#if ESP_ARDUINO_VERSION_MAJOR != 2 || ESP_ARDUINO_VERSION_MINOR != 0 || \
    ESP_ARDUINO_VERSION_PATCH != 11
#error "Dieses Projekt verwendet Arduino-ESP32 2.0.11. Bitte die festgelegte PlatformIO-Konfiguration verwenden."
#endif

#include "BoardHardware.h"
#include "HardwareTestUi.h"
#include "MoonrakerClient.h"
#include "PrinterState.h"

BoardHardware board;
HardwareTestUi ui;
PrinterState printerState;
MoonrakerClient moonraker;
bool ready = false;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nESP32-Klipper Bedienclient 0.4.0 / Core 2.0.11");
  Serial.printf("Flash: %u Bytes, PSRAM: %u Bytes, freier Heap: %u Bytes\n",
                ESP.getFlashChipSize(), ESP.getPsramSize(), ESP.getFreeHeap());

  if (!board.begin()) return;
  ui.begin(board, printerState, moonraker);
  moonraker.begin(printerState);
  ready = true;
  Serial.printf("Bereit. Zeichenpuffer: %u Bytes, freier Heap: %u\n",
                unsigned(BoardHardware::width * BoardHardware::bufferLines *
                         sizeof(lv_color_t)),
                ESP.getFreeHeap());
}

void loop() {
  static uint32_t lastLog = 0;
  if (!ready) {
    delay(50);
    return;
  }

  const uint32_t now = millis();
  board.service(now);
  moonraker.loop(now);
  lv_timer_handler();
  ui.update(now);

  if (uint32_t(now - lastLog) >= 5000) {
    lastLog = now;
    Serial.printf("Uptime %lus | Heap %u | PSRAM frei %u | Touch %s | I2C %lu | WLAN %s | Moonraker %s\n",
                  static_cast<unsigned long>(now / 1000), ESP.getFreeHeap(),
                  ESP.getFreePsram(), board.touchOnline() ? "OK" : "OFFLINE",
                  static_cast<unsigned long>(board.touchErrors()),
                  linkStateText(printerState.wifi),
                  linkStateText(printerState.moonraker));
  }
  delay(5);
}
