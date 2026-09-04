#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <initializer_list>

// Read-only controller configuration; only acknowledges consumed touch reports.
// No reset/interrupt pin: board connects these independently of the ESP32.
class SafeGT911 {
 public:
  bool begin() {
    Wire.begin(19, 45);
    Wire.setClock(100000);
    Wire.setTimeOut(30);
    return probe();
  }
  bool probe() {
    online = false;
    pressed = false;
    for (uint8_t candidate : {uint8_t(0x5D), uint8_t(0x14)}) {
      address = candidate;
      uint8_t id[4] = {};
      if (readRegister(0x8140, id, sizeof(id)) &&
          id[0] == '9' && id[1] == '1' && id[2] == '1') {
        online = true;
        lastReport = millis();
        Serial.printf("GT911 erkannt: Adresse 0x%02X, ID %c%c%c\n", address, id[0], id[1], id[2]);
        return true;
      }
    }
    address = 0;
    return false;
  }
  void poll() {
    if (!online) { pressed = false; return; }
    uint8_t status = 0;
    if (!readRegister(0x814E, &status, 1)) { fail(); return; }
    if (!(status & 0x80)) {
      // No fresh report: retain last state briefly, then release a stale touch.
      if (uint32_t(millis() - lastReport) > 150) pressed = false;
      return;
    }
    const uint8_t count = status & 0x0F;
    if (count > 5) { acknowledge(); fail(); return; }
    uint8_t point[8] = {};
    if (count && !readRegister(0x814F, point, sizeof(point))) { fail(); return; }
    if (!acknowledge()) { fail(); return; }
    lastReport = millis();
    pressed = count > 0;
    if (pressed) {
      const uint16_t rawX = uint16_t(point[1]) | (uint16_t(point[2]) << 8);
      const uint16_t rawY = uint16_t(point[3]) | (uint16_t(point[4]) << 8);
      if (rawX >= 480 || rawY >= 480) { pressed = false; ++errors; return; }
      // Net orientation of manufacturer driver + its reversed mapping is direct.
      // Validate on all four targets before changing orientation.
      x = rawX; y = rawY;
    }
  }
  bool online = false;
  bool pressed = false;
  uint16_t x = 0, y = 0;
  uint8_t address = 0;
  uint32_t errors = 0;
 private:
  uint32_t lastReport = 0;
  void fail() { ++errors; pressed = false; online = false; }
  bool readRegister(uint16_t reg, uint8_t *data, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(uint8_t(reg >> 8)); Wire.write(uint8_t(reg));
    if (Wire.endTransmission() != 0) return false;
    const size_t received = Wire.requestFrom(address, length);
    if (received != length) {
      while (Wire.available()) Wire.read();
      return false;
    }
    for (uint8_t i = 0; i < length; ++i) {
      const int value = Wire.read();
      if (value < 0) return false;
      data[i] = uint8_t(value);
    }
    return true;
  }
  bool acknowledge() {
    Wire.beginTransmission(address);
    Wire.write(uint8_t(0x81)); Wire.write(uint8_t(0x4E)); Wire.write(uint8_t(0));
    return Wire.endTransmission() == 0;
  }
};
