# Validierung – Firmware 0.4.0 (PlatformIO)

Stand: 08.09.2026.

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
- Modulare Firmwarebasis 0.2.0 vollständig gebaut und über COM5 hochgeladen: 499413 Bytes Flash (14,9 %) und 86812 Bytes statischer RAM (26,5 %).
- Boot der modularen Firmware bestanden: 16 MB Flash, rund 8 MB PSRAM, GT911 auf 0x5D, Touchstatus `OK` und keine I²C-Fehler.
- Nutzer bestätigt Testseite, Systemdiagnoseseite, korrekte Livewerte, flüssige Navigation in beide Richtungen sowie unveränderte Touch- und Backlight-Funktion.
- Read-only-Statusclient 0.3.0 vollständig gebaut: 1080145 Bytes Flash von 3342336 Bytes (32,3 %) und 113684 Bytes statischer RAM von 327680 Bytes (34,7 %).
- Firmware 0.3.0 erfolgreich über COM5 übertragen; Hashprüfung aller geschriebenen Bereiche bestanden.
- WLAN-Verbindung am Gerät erfolgreich; Moonraker unter dem konfigurierten lokalen Host über `/websocket` verbunden und verfügbare Statusobjekte abonniert.
- Der anfängliche HTTP-403-Handshakefehler wurde auf den Bibliotheksstandard `Origin: file://` zurückgeführt. Nach explizitem Entfernen dieses optionalen Headers verbindet sich der Moonraker-WebSocket erfolgreich.
- Nutzer bestätigt Klipper-Status `bereit` sowie plausible Hotend- und Heizbetttemperaturen, die sich laufend aktualisieren.
- Reconnect-Test bestanden: WLAN am laufenden Panel gezielt einmal getrennt; Firmware markierte die Verbindung als getrennt, baute WLAN und WebSocket selbstständig neu auf und abonnierte die Statusobjekte erneut. Kein Panel-Neustart und kein Druckerbefehl erforderlich.
- Bereinigten Endstand ohne temporären Testauslöser erneut gebaut, hochgeladen und per Bootlog geprüft: WLAN verbunden, Moonraker verbunden, Statusobjekte abonniert, Touch `OK`, I²C-Fehlerzähler 0.
- Vorhandene und eingebundene Makros `PREHEAT_PLA`, `PREHEAT_PETG` und `COOLDOWN` aus `printer.cfg` und `bedienung_macros.cfg` über Moonraker read-only geprüft. Die Vorheizmakros verwenden die serverseitige Druck-/Pause-Sperre `_BEDIENUNG_IDLE`.
- Konfigurierte Temperaturgrenzen geprüft: Hotend 250 °C, Heizbett 130 °C. Die fest verdrahteten Profile PLA 210/60 °C und PETG 240/80 °C liegen innerhalb dieser Grenzen; freie Temperatureingaben sind nicht implementiert.
- Offizielle JSON-RPC-Methoden `printer.gcode.script` für die drei Makros und `printer.firmware_restart` für die MCU-Wiederverbindung verwendet. Aktionen werden nicht gespeichert oder nach einem Reconnect wiederholt.
- Firmware 0.4.0 vollständig gebaut und über COM5 hochgeladen: 1083649 Bytes Flash von 3342336 Bytes (32,4 %) und 113764 Bytes statischer RAM von 327680 Bytes (34,7 %); Hashprüfung bestanden.
- Klipper-Firmware-Neustart am Display erfolgreich bestätigt: Nach Einschalten der zuvor abgeschalteten Drucker-MCU wechselte Klipper ohne Mainsail von `shutdown` zu `ready`.
- Nutzer bestätigt Bestätigungsdialoge und erfolgreiche PLA-/PETG-Vorheizaktionen. Moonrakers G-Code-Verlauf enthält `PREHEAT_PLA`, `COOLDOWN`, `PREHEAT_PETG`, `COOLDOWN` in der geprüften Reihenfolge.
- Cooldown nach beiden Heiztests bestätigt; abschließende Sollwerte für Hotend und Heizbett jeweils 0 °C, Klipper `ready`, Druckzustand `standby`.

Hosttest reproduzieren (Linux mit g++ oder entsprechend eingerichteter C++-Umgebung):

```sh
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined -I tests/stubs tests/touch_test.cpp -o touch_test
ASAN_OPTIONS=detect_leaks=0 ./touch_test
```

Die Stubs werden ausschließlich für diesen Hosttest verwendet. Sie ersetzen keine ESP32-Hardwaretests und gehören nicht in den Sketchordner.

## Noch offen

- Mehrstündige Stabilität unter dauerhaftem WLAN-/Moonraker-Betrieb.
- Read-only-Anzeige während eines echten Druckjobs einschließlich Dateiname und Fortschritt.

Firmware 0.4.0 ist auf der Zielhardware einschließlich der ersten kontrollierten Druckeraktionen bestätigt. Erzwungene Moonraker-Fehler- und Timeoutpfade sind implementiert, aber noch nicht durch absichtlich erzeugte Serverfehler praktisch getestet.

PlatformIO-Konvertierung: main.cpp statt Arduino-Sketch; Aufrufreihenfolge der Funktionen geprüft (keine automatisch erzeugten Arduino-Prototypen erforderlich). Lokale Bibliotheken und LVGL-Konfigurationspfad angepasst. Plattformmanifest 6.4.0 geprüft: verwendet Arduino-ESP32-Paket ~3.20011.0. Vollständiger Build am 06.09.2026 erfolgreich durchgeführt.
