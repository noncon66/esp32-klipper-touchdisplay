#include <Arduino.h>
#include <initializer_list>
#include <esp_arduino_version.h>
#if ESP_ARDUINO_VERSION_MAJOR != 2 || ESP_ARDUINO_VERSION_MINOR != 0 || ESP_ARDUINO_VERSION_PATCH != 11
#error "Dieses Referenzpaket verwendet Arduino-ESP32 2.0.11. Bitte die festgelegte PlatformIO-Konfiguration verwenden."
#endif
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include "SafeGT911.h"

constexpr uint8_t BACKLIGHT = 38;
constexpr uint16_t WIDTH = 480, HEIGHT = 480, BUFFER_LINES = 30;
Arduino_ESP32RGBPanel bus(
  39, 48, 47, 18, 17, 16, 21,
  11, 12, 13, 14, 0, 8, 20, 3, 46, 9, 10, 4, 5, 6, 7, 15);
Arduino_ST7701_RGBPanel gfx(
  &bus, GFX_NOT_DEFINED, 0, true, WIDTH, HEIGHT,
  st7701_type1_init_operations, sizeof(st7701_type1_init_operations), true,
  10, 8, 50, 10, 8, 20);
SafeGT911 touch;
lv_disp_draw_buf_t drawBuffer;
lv_disp_drv_t displayDriver;
lv_indev_drv_t inputDriver;
lv_obj_t *statusLabel, *coordinateLabel, *counterLabel;
lv_obj_t *targets[4];
uint32_t clicks = 0;
bool ready = false;
uint32_t lightRestoreAt = 0;
uint8_t brightness = 255;

void fatal(const char *message) {
  Serial.printf("STOP: %s\n", message);
  // loop() remains alive for logs; no LVGL access after a failed setup.
  ready = false;
}
void flushDisplay(lv_disp_drv_t *driver, const lv_area_t *area, lv_color_t *pixels) {
  gfx.draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(pixels),
                        area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
  lv_disp_flush_ready(driver);
}
void readTouch(lv_indev_drv_t *, lv_indev_data_t *data) {
  touch.poll();
  data->state = touch.pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  data->point.x = touch.x;
  data->point.y = touch.y;
}
lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y) {
  lv_obj_t *obj = lv_label_create(parent);
  lv_label_set_text(obj, text);
  lv_obj_set_pos(obj, x, y);
  return obj;
}
void targetPressed(lv_event_t *event) {
  lv_obj_set_style_bg_color(lv_event_get_target(event), lv_color_hex(0x087F5B), 0);
  ++clicks;
  lv_label_set_text_fmt(counterLabel, "Tastendruecke: %lu", (unsigned long)clicks);
}
void brightnessChanged(lv_event_t *event) {
  brightness = uint8_t(lv_slider_get_value(lv_event_get_target(event)));
  if (!lightRestoreAt) ledcWrite(0, brightness);
}
void blinkLight(lv_event_t *) {
  ledcWrite(0, 0);
  lightRestoreAt = millis() + 1500;
}
void resetTargets(lv_event_t *) {
  clicks = 0;
  lv_label_set_text(counterLabel, "Tastendruecke: 0");
  for (auto *obj : targets) lv_obj_set_style_bg_color(obj, lv_color_hex(0x285AB4), 0);
}
lv_obj_t *button(const char *text, int x, int y, int w, int h, lv_event_cb_t callback) {
  lv_obj_t *obj = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(obj, x, y); lv_obj_set_size(obj, w, h);
  lv_obj_add_event_cb(obj, callback, LV_EVENT_CLICKED, nullptr);
  lv_obj_t *caption = lv_label_create(obj);
  lv_label_set_text(caption, text); lv_obj_center(caption);
  return obj;
}
void createUI() {
  lv_obj_t *screen = lv_scr_act();
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x101B2B), 0);
  lv_obj_set_style_text_color(screen, lv_color_hex(0xF4F7FB), 0);
  label(screen, "DISPLAY + TOUCHTEST", 118, 26);
  statusLabel = label(screen, "Touch wird geprueft ...", 24, 102);
  coordinateLabel = label(screen, "X: ---   Y: ---   losgelassen", 24, 132);
  counterLabel = label(screen, "Tastendruecke: 0", 24, 160);
  const uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};
  const char *names[] = {"ROT", "GRUEN", "BLAU", "WEISS"};
  for (int i = 0; i < 4; ++i) {
    lv_obj_t *tile = lv_obj_create(screen);
    lv_obj_set_pos(tile, 24 + i * 110, 205); lv_obj_set_size(tile, 100, 52);
    lv_obj_set_style_bg_color(tile, lv_color_hex(colors[i]), 0);
    lv_obj_set_style_border_width(tile, 0, 0);
    lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    label(screen, names[i], 30 + i * 110, 265);
  }
  label(screen, "Helligkeit", 24, 303);
  lv_obj_t *slider = lv_slider_create(screen);
  lv_obj_set_pos(slider, 142, 306); lv_obj_set_size(slider, 290, 20);
  lv_slider_set_range(slider, 25, 255); lv_slider_set_value(slider, 255, LV_ANIM_OFF);
  lv_obj_add_event_cb(slider, brightnessChanged, LV_EVENT_VALUE_CHANGED, nullptr);
  button("Licht aus: 1,5 s", 24, 349, 208, 44, blinkLight);
  button("Zaehler zurueck", 248, 349, 208, 44, resetTargets);
  targets[0] = button("OL", 8, 8, 78, 72, targetPressed);
  targets[1] = button("OR", 394, 8, 78, 72, targetPressed);
  targets[2] = button("UL", 8, 400, 78, 72, targetPressed);
  targets[3] = button("UR", 394, 400, 78, 72, targetPressed);
  label(screen, "Vier Ecktasten testen", 130, 433);
}
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nESP32-Klipper Touchtest 0.1.1 / Core 2.0.11");
  Serial.printf("Flash: %u Bytes, PSRAM: %u Bytes, freier Heap: %u Bytes\n",
                ESP.getFlashChipSize(), ESP.getPsramSize(), ESP.getFreeHeap());
  pinMode(BACKLIGHT, OUTPUT); digitalWrite(BACKLIGHT, HIGH);
  if (!psramFound()) { fatal("PSRAM fehlt. OPI PSRAM waehlen."); return; }
  // Manufacturer timing/initialization; test without printer/network commands.
  gfx.begin(16000000);
  if (!gfx.getFramebuffer()) { fatal("RGB-Framebuffer fehlt."); return; }
  gfx.fillScreen(BLACK);
  ledcSetup(0, 1000, 8); ledcAttachPin(BACKLIGHT, 0); ledcWrite(0, brightness);
  touch.begin();
  lv_init();
  auto *pixels = static_cast<lv_color_t *>(heap_caps_malloc(
      WIDTH * BUFFER_LINES * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  if (!pixels) { fatal("LVGL-Zeichenpuffer fehlt."); return; }
  lv_disp_draw_buf_init(&drawBuffer, pixels, nullptr, WIDTH * BUFFER_LINES);
  lv_disp_drv_init(&displayDriver);
  displayDriver.hor_res = WIDTH; displayDriver.ver_res = HEIGHT;
  displayDriver.flush_cb = flushDisplay; displayDriver.draw_buf = &drawBuffer;
  if (!lv_disp_drv_register(&displayDriver)) { fatal("LVGL-Displayregistrierung fehlgeschlagen."); return; }
  lv_indev_drv_init(&inputDriver);
  inputDriver.type = LV_INDEV_TYPE_POINTER; inputDriver.read_cb = readTouch;
  if (!lv_indev_drv_register(&inputDriver)) { fatal("LVGL-Touchregistrierung fehlgeschlagen."); return; }
  createUI(); ready = true;
  Serial.printf("Bereit. Zeichenpuffer: %u Bytes, freier Heap: %u\n",
                unsigned(WIDTH * BUFFER_LINES * sizeof(lv_color_t)), ESP.getFreeHeap());
}
void loop() {
  static uint32_t lastUI = 0, lastProbe = 0, lastLog = 0;
  if (!ready) { delay(50); return; }
  const uint32_t now = millis();
  if (lightRestoreAt && int32_t(now - lightRestoreAt) >= 0) {
    lightRestoreAt = 0; ledcWrite(0, brightness);
  }
  if (!touch.online && uint32_t(now - lastProbe) >= 2000) {
    lastProbe = now; touch.probe();
  }
  lv_timer_handler();
  if (uint32_t(now - lastUI) >= 100) {
    lastUI = now;
    lv_label_set_text_fmt(statusLabel, "GT911: %s  |  I2C-Fehler: %lu",
                         touch.online ? "verbunden" : "nicht erkannt", (unsigned long)touch.errors);
    lv_label_set_text_fmt(coordinateLabel, "X: %u   Y: %u   %s", touch.x, touch.y,
                         touch.pressed ? "beruehrt" : "losgelassen");
  }
  if (uint32_t(now - lastLog) >= 5000) {
    lastLog = now;
    Serial.printf("Uptime %lus | Heap %u | PSRAM frei %u | Touch %s | Fehler %lu\n",
                  (unsigned long)(now / 1000), ESP.getFreeHeap(), ESP.getFreePsram(),
                  touch.online ? "OK" : "OFFLINE", (unsigned long)touch.errors);
  }
  delay(5);
}
