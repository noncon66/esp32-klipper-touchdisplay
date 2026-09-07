#pragma once

#include <Arduino.h>
#include <cstring>

#if defined(__has_include)
#if __has_include("secrets.h")
#include "secrets.h"
#define ESP32_TOUCHDISPLAY_HAS_SECRETS 1
#endif
#endif

#ifndef ESP32_TOUCHDISPLAY_HAS_SECRETS
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define MOONRAKER_HOST ""
#define MOONRAKER_PORT 7125
#define MOONRAKER_API_KEY ""
#endif

namespace NetworkConfig {
static constexpr const char *wifiSsid = WIFI_SSID;
static constexpr const char *wifiPassword = WIFI_PASSWORD;
static constexpr const char *moonrakerHost = MOONRAKER_HOST;
static constexpr uint16_t moonrakerPort = MOONRAKER_PORT;
static constexpr const char *moonrakerApiKey = MOONRAKER_API_KEY;

inline bool isComplete() {
  return std::strlen(wifiSsid) > 0 && std::strlen(wifiPassword) > 0 &&
         std::strlen(moonrakerHost) > 0 && moonrakerPort > 0;
}
}  // namespace NetworkConfig
