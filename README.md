# ESP32-Klipper-Touchdisplay

Hardwaretest 0.1.1 · Windows, VS Code und PlatformIO · GUITION ESP32-4848S040C_I

Eigene lokale Touchoberfläche für den Ender-3 mit Klipper. Diese erste Firmware testet nur Display, Touch und Backlight. WLAN/Moonraker und Druckeraktionen folgen später.

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
| src/main.cpp | Testoberfläche und Hardwareinitialisierung |
| include/SafeGT911.h | geprüfte Touch-Lesezugriffe, Fehlerbehandlung |
| include/lv_conf.h | LVGL-Konfiguration |
| lib/lvgl und lib/Arduino_GFX | Herstellerbibliotheken mit vorhandenen Lizenzdateien |
| platformio.ini | Build-, Speicher- und Uploadparameter |
| docs/ | Projektdokumentation, Hardwaretest und Validierung |
| tests/ | Hosttests mit simuliertem I²C, separat von PlatformIO-Firmwarebuild |

## GitHub vorbereiten

Dieses Paket ist bereit, in dein noch anzulegendes Repository übernommen zu werden. Der Repository-Name könnte `ESP32-Klipper-Touchdisplay` sein. Im Repository sollen `platformio.ini`, `src`, `include`, `lib`, `docs`, `tests`, README und `.gitignore` liegen.

Am einfachsten: leeres Repository auf GitHub anlegen, in VS Code klonen, Paketinhalt in dessen Wurzel kopieren und Änderungen über die Quellcodeverwaltung committen. Zuerst prüfen, dass `.pio` nicht zum Commit gehört. Der Push erfolgt durch dich; es wurde noch kein Repository erstellt oder veröffentlicht.

Die `.gitignore` lässt Buildausgaben, lokale IDE-Dateien und vorgesehene Secrets-Dateien aus. Im aktuellen Projekt sind keine WLAN-/Moonraker-Zugangsdaten enthalten. Die mitgelieferten Drittanbieterbibliotheken behalten ihre Lizenzbedingungen; für den eigenen Projektcode ist noch keine öffentliche Lizenz ausgewählt.

Die Dokumentation bleibt ein lebendes Projektdokument. Sobald das Repository besteht, wird `docs/ESP32-Klipper-Touchdisplay-Projektdokumentation.md` dort als Projektdatei weitergeführt. Künftige Änderungen sollen den Repository-Stand berücksichtigen; aktuell ist noch keine Synchronisation eingerichtet.

## Prüfung und nächster Schritt

Der Touch-Treiber wurde auf dem Host mit simuliertem I²C getestet. Ein vollständiger PlatformIO-Build und ein Hardwaretest sind noch offen. Es gibt kein geprüftes Firmware-Binary. Details in `docs/VALIDIERUNG.md`.

Nach erfolgreichem Build und Upload die Prüfschritte in `docs/Hardwaretest.md` durchführen. Bitte Bootlog und Ergebnis der vier Ecktasten zurückmelden.
