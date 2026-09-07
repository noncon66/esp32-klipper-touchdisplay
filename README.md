# ESP32-Klipper-Touchdisplay

Firmwarebasis 0.2.0 · Windows, VS Code und PlatformIO · GUITION ESP32-4848S040C_I

Eigene lokale Touchoberfläche für den Ender-3 mit Klipper. Die aktuelle modulare Firmwarebasis enthält eine Hardwaretestseite und eine Systemdiagnoseseite. WLAN/Moonraker und Druckeraktionen folgen später.

## Start unter Windows

1. VS Code und die Erweiterung **PlatformIO IDE** installieren: [offizielle Anleitung](https://docs.platformio.org/en/latest/integration/ide/vscode.html).
2. ZIP vollständig in einen Projektordner entpacken, z. B. `C:\Projekte\ESP32-Klipper-Touchdisplay`.
3. In VS Code **Datei → Ordner öffnen** und den Ordner wählen, der `platformio.ini` direkt enthält. Kein neues Boardprojekt erzeugen.
4. PlatformIO initialisieren lassen; beim ersten Build werden Plattform und Toolchain heruntergeladen. Dafür ist Internetzugang nötig.
5. Im PlatformIO-Bereich unter **Project Tasks → guition_touchtest → General → Build** zuerst kompilieren.
6. Display über USB-Datenkabel anschließen, **Upload** und anschließend **Monitor** wählen. Der Monitor läuft mit 115200 Baud.

Alternativ im PlatformIO-Terminal:

```powershell
pio run -e guition_touchtest
pio run -e guition_touchtest -t upload
pio device monitor -b 115200
```

Bei mehreren COM-Ports die kommentierten `upload_port`-/`monitor_port`-Zeilen in `platformio.ini` auf den tatsächlichen Port anpassen. Das Board verwendet laut Schaltplan CH340C, nicht die native USB-Verbindung auf GPIO19/20. Bei fehlendem COM-Port Datenkabel und Windows-Gerätemanager prüfen.

Falls der automatische Flashmodus scheitert: am Board BOOT halten, RESET kurz drücken, BOOT loslassen und erneut hochladen. Danach bei Bedarf RESET drücken. Der Upload ersetzt die vorhandene Demo-Firmware. Für den Test allein über USB versorgen.

## Festgelegte Buildbasis

| Bestandteil | Konfiguration |
|---|---|
| PlatformIO-Plattform | espressif32 6.4.0 |
| Framework | Arduino-ESP32 2.0.11, zusätzlich im Code geprüft |
| Basisboard | esp32-s3-devkitc-1 mit projektspezifischen Speicher-Overrides |
| Flash / PSRAM | 16 MB QIO / OPI-PSRAM, anhand N16R8-Unterlagen |
| Speicherprofil | qio_opi; 80 MHz Flash |
| Partition | default_8MB.csv; nutzt zunächst einen Teil des 16-MB-Flashs |
| LVGL | 8.3.9 aus Herstellerpaket |
| Arduino_GFX | 1.2.9 aus Herstellerpaket |
| Touch | eigener SafeGT911-Treiber |

PlatformIO ist die Buildumgebung; Arduino bleibt hier das Firmwareframework. Keine Arduino IDE und keine globale Sketchbook-Umschaltung erforderlich. Die tatsächlich am Board erkannte Speichergröße muss im Bootlog bestätigt werden.

Die gelieferten Bibliotheken liegen lokal unter `lib/`, damit deren Quellstand erhalten bleibt. Nicht zusätzlich aus dem Registry mit möglicherweise abweichenden Versionen installieren. `include/lv_conf.h` wird über `LV_CONF_INCLUDE_SIMPLE` eingebunden. Es sind keine Änderungen an globalen `boards.txt` oder Herstellerpartitionstabellen erforderlich.

Quellen für die Konfiguration: [PlatformIO-Plattformmanifest v6.4.0](https://github.com/platformio/platform-espressif32/blob/v6.4.0/platform.json), [Basisboard v6.4.0](https://github.com/platformio/platform-espressif32/blob/v6.4.0/boards/esp32-s3-devkitc-1.json), [PlatformIO-Projektkonfiguration](https://docs.platformio.org/en/latest/projectconf/index.html). Manifest und Boarddefinition wurden am 04.09.2026 eingesehen.

## Projektstruktur

| Pfad | Inhalt |
|---|---|
| src/main.cpp | Anwendungsstart, zyklische Dienste und Statuslogging |
| include/BoardHardware.h und src/BoardHardware.cpp | Display, Touch, Backlight und LVGL-Treiberanbindung |
| include/HardwareTestUi.h und src/HardwareTestUi.cpp | Hardwaretest- und Systemdiagnoseseite mit Navigation |
| include/SafeGT911.h | geprüfte Touch-Lesezugriffe, Fehlerbehandlung |
| include/lv_conf.h | LVGL-Konfiguration |
| lib/lvgl und lib/Arduino_GFX | Herstellerbibliotheken mit vorhandenen Lizenzdateien |
| platformio.ini | Build-, Speicher- und Uploadparameter |
| docs/ | Projektdokumentation, Hardwaretest und Validierung |
| tests/ | Hosttests mit simuliertem I²C, separat von PlatformIO-Firmwarebuild |

## Repository

Das Projekt ist mit [noncon66/esp32-klipper-touchdisplay](https://github.com/noncon66/esp32-klipper-touchdisplay) verbunden. Buildausgaben unter `.pio` bleiben vom Commit ausgeschlossen.

Die `.gitignore` lässt Buildausgaben, lokale IDE-Dateien und vorgesehene Secrets-Dateien aus. Im aktuellen Projekt sind keine WLAN-/Moonraker-Zugangsdaten enthalten. Die mitgelieferten Drittanbieterbibliotheken behalten ihre Lizenzbedingungen; für den eigenen Projektcode ist noch keine öffentliche Lizenz ausgewählt.

Die Dokumentation bleibt ein lebendes Projektdokument und wird unter `docs/ESP32-Klipper-Touchdisplay-Projektdokumentation.md` weitergeführt.

## Prüfung und nächster Schritt

Der Touch-Treiber wurde auf dem Host mit simuliertem I²C getestet. Vollständiger PlatformIO-Build, erster Hardwaretest und die modulare Firmwarebasis 0.2.0 wurden erfolgreich abgeschlossen. Am Gerät bestätigt sind 16 MB Flash, rund 8 MB PSRAM, GT911 auf Adresse 0x5D, Bild, Farben, Touch, Backlight sowie beide LVGL-Seiten und deren Navigation. Details in `docs/VALIDIERUNG.md`.

Nach erfolgreichem Build und Upload die Prüfschritte in `docs/Hardwaretest.md` durchführen. Bitte Bootlog und Ergebnis der vier Ecktasten zurückmelden.
