# Validierung – Touchtest 0.1.1 (PlatformIO)

Stand: 07.09.2026.

## Erfolgreich durchgeführt

- API-Signaturen für den verwendeten ST7701-/RGB-Treiber anhand der gelieferten Arduino_GFX-Quellen geprüft.
- Displaypins und Timings mit dem Herstellerdemo abgeglichen.
- Touch-Treiber als C++11 mit `-Wall -Wextra -Werror` auf dem Host kompiliert.
- Tests mit simuliertem I²C und Address-/UndefinedBehavior-Sanitizer bestanden: GT911-Erkennung, beide Adressen, falsche Produktkennung, Koordinaten, Randwerte, Loslassen, veralteter Kontakt, ungültige Punktanzahl, kurze Antworten, Busfehler und erneute Erkennung.
- LeakSanitizer war wegen der Hostumgebung deaktiviert; keine Aussage über einen LeakSanitizer-Test.
- Paketinhalt und ZIP-Integrität geprüft.
- Vollständiger PlatformIO-Build der Umgebung `guition_touchtest` unter Windows bestanden: PlatformIO Core 6.2.0, `espressif32` 6.4.0, Arduino-ESP32 2.0.11, Arduino_GFX 1.2.9 und LVGL 8.3.9.
- Firmware erfolgreich kompiliert und gelinkt; `firmware.bin` erzeugt. Speicherbelegung: 497125 Bytes Flash von 3342336 Bytes (14,9 %) und 86780 Bytes RAM von 327680 Bytes (26,5 %).
- Zwei Warnungen zum verworfenen `const`-Qualifier stammen aus unveränderten LVGL-8.3.9-Quellen; der Build wurde erfolgreich beendet.
- Firmware am 07.09.2026 erfolgreich über COM5 auf einen ESP32-S3 Revision v0.2 übertragen; Hashprüfung aller geschriebenen Bereiche bestanden.
- Bootlog bestätigt 16 MB Flash, rund 8 MB PSRAM, GT911 auf Adresse 0x5D und erfolgreichen Start der LVGL-Testoberfläche.
- Nach zehn Sekunden: 267432 Bytes freier Heap, 7925019 Bytes freie PSRAM, Touchstatus `OK` und keine I²C-Fehler.
- Nutzer bestätigt korrekte Darstellung der dunklen Testoberfläche und der vier Farbfelder Rot, Grün, Blau und Weiß.
- Nutzer bestätigt alle vier Ecktasten an der richtigen Position, Klickzähler, gültigen Koordinatenbereich und korrektes Loslassen.
- Nutzer bestätigt funktionierenden Helligkeitsregler sowie Ausschalten und automatische Rückkehr der Beleuchtung nach etwa 1,5 Sekunden.
- Zehn-Minuten-Stabilitätstest bis Uptime 610 Sekunden bestanden: kein Reset, Heap konstant bei 267432 Bytes, freie PSRAM konstant bei 7925019 Bytes, Touch durchgehend `OK` und I²C-Fehlerzähler 0.

Hosttest reproduzieren (Linux mit g++ oder entsprechend eingerichteter C++-Umgebung):

```sh
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined -I tests/stubs tests/touch_test.cpp -o touch_test
ASAN_OPTIONS=detect_leaks=0 ./touch_test
```

Die Stubs werden ausschließlich für diesen Hosttest verwendet. Sie ersetzen keine ESP32-Hardwaretests und gehören nicht in den Sketchordner.

## Noch offen

- Stabilität unter WLAN-Last; WLAN ist in diesem Test noch nicht eingebunden.

Das Projekt erzeugt ein Firmware-Binary, ist aber noch keine am Gerät bestätigte Firmware.

PlatformIO-Konvertierung: main.cpp statt Arduino-Sketch; Aufrufreihenfolge der Funktionen geprüft (keine automatisch erzeugten Arduino-Prototypen erforderlich). Lokale Bibliotheken und LVGL-Konfigurationspfad angepasst. Plattformmanifest 6.4.0 geprüft: verwendet Arduino-ESP32-Paket ~3.20011.0. Vollständiger Build am 06.09.2026 erfolgreich durchgeführt.
