# Validierung – Touchtest 0.1.1 (PlatformIO)

Stand: 04.09.2026.

## Erfolgreich durchgeführt

- API-Signaturen für den verwendeten ST7701-/RGB-Treiber anhand der gelieferten Arduino_GFX-Quellen geprüft.
- Displaypins und Timings mit dem Herstellerdemo abgeglichen.
- Touch-Treiber als C++11 mit `-Wall -Wextra -Werror` auf dem Host kompiliert.
- Tests mit simuliertem I²C und Address-/UndefinedBehavior-Sanitizer bestanden: GT911-Erkennung, beide Adressen, falsche Produktkennung, Koordinaten, Randwerte, Loslassen, veralteter Kontakt, ungültige Punktanzahl, kurze Antworten, Busfehler und erneute Erkennung.
- LeakSanitizer war wegen der Hostumgebung deaktiviert; keine Aussage über einen LeakSanitizer-Test.
- Paketinhalt und ZIP-Integrität geprüft.

Hosttest reproduzieren (Linux mit g++ oder entsprechend eingerichteter C++-Umgebung):

```sh
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined -I tests/stubs tests/touch_test.cpp -o touch_test
ASAN_OPTIONS=detect_leaks=0 ./touch_test
```

Die Stubs werden ausschließlich für diesen Hosttest verwendet. Sie ersetzen keine ESP32-Hardwaretests und gehören nicht in den Sketchordner.

## Noch offen

- Vollständiger ESP32-Build und Linktest: Arduino CLI heruntergeladen, aber Installation des Core/der Toolchain wegen nicht erreichbarer Paketquellen aus der CLI gescheitert. Kein Firmware-Binary erzeugt.
- Windows-Build mit PlatformIO und Arduino-ESP32 2.0.11 und den mitgelieferten Bibliotheken.
- Display-, Touch-, PSRAM- und PWM-Funktion auf dem tatsächlichen Board.
- Stabilität unter WLAN-Last; WLAN ist in diesem Test noch nicht eingebunden.

Das Paket ist eine vorbereitete Testfirmware im Quellcode, keine bereits am Gerät bestätigte Firmware.

PlatformIO-Konvertierung: main.cpp statt Arduino-Sketch; Aufrufreihenfolge der Funktionen geprüft (keine automatisch erzeugten Arduino-Prototypen erforderlich). Lokale Bibliotheken und LVGL-Konfigurationspfad angepasst. Plattformmanifest 6.4.0 geprüft: verwendet Arduino-ESP32-Paket ~3.20011.0. Kein vollständiger PlatformIO-Build durchgeführt.
