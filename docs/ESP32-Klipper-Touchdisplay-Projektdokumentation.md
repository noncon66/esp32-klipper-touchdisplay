# ESP32-Klipper-Touchdisplay – Projektdokumentation

Stand: 07.09.2026 · Version 1.0 · Status: Read-only-Moonraker-Anbindung mit Firmware 0.3.0 bestätigt

## 1. Ziel und Geltungsbereich

Ein eigenes Touchpanel für den Ender-3 Classic zeigt den Druckerzustand und ermöglicht häufige Bedienaufgaben direkt am Drucker. Das vorhandene GUITION ESP32-4848S040C_I mit 480 × 480 Pixeln kommuniziert per WLAN mit Moonraker. Klipper auf dem Raspberry Pi 4 bleibt für alle Druckerabläufe zuständig; Mainsail bleibt parallel nutzbar. Es entsteht keine vollständige Kopie von Mainsail oder KlipperScreen.

Diese Datei führt den technischen Arbeitsstand, Entscheidungen, offene Punkte und Prüfergebnisse zusammen. Die ursprüngliche Projektübersicht bleibt die Anforderungsquelle. „Belegt“ bedeutet hier durch Unterlagen belegt, nicht am vorhandenen Gerät getestet. Vorschläge sind ausdrücklich noch keine abgeschlossenen Implementierungen.

## 2. Ausgangslage

Laut Projektübersicht: Ender-3 Classic, BTT SKR Mini E3 V3.0, Raspberry Pi 4, Klipper/Moonraker/Mainsail, Bowden-Extruder, 0,4-mm-Düse und PEI-Oberfläche. CR Touch und BTT S2DW sind geplant/bestellt; ihr Einbau ist in diesem Projekt noch nicht bestätigt.

Inzwischen liegen zusätzlich Operating instructions.zip, Libraries.zip und das Herstellerdemo 1_2_4.0_LvglWidgets.zip vor. Demoquellen und LVGL-Konfiguration sind geprüft. Firmware 0.3.0 wurde vollständig mit PlatformIO gebaut, auf die Zielhardware geflasht und read-only mit Moonraker verbunden. Aktuelle Klipper-Konfigurationsdateien und die darin tatsächlich vorhandenen Makros stehen für den nächsten Schritt noch aus.

## 3. Sichtung der Quelldateien

| Quelle | Ergebnis und Nutzen |
|---|---|
| ESP32-Klipper-Touchdisplay – Projektübersicht.md | Vollständig gelesen; Grundlage für Anforderungen, Architektur und Funktionsumfang. |
| ESP32-4848S040 Specifications-EN.pdf | 6 Seiten; technische Daten und Abbildungen geprüft. Bestätigt 480 × 480, ST7701, 16 MB Flash, 8 MB PSRAM und 5 V. Modell- und Maßangaben widersprüchlich. |
| 4.0 inches IO pin distribution.xlsx | Sheet1, 36 GPIO-Zeilen vollständig ausgewertet und mit Schaltplänen abgeglichen. |
| 2.png | Schaltplan: ESP32-Modul, Display, Touch, Audio, Batteriepfad. Hauptquelle für Signalzuordnung. |
| 1.png | Schaltplan: Versorgung, CH340C, USB-C, Backlight und microSD. |
| ESP32-S3-WROOM-1 Pin definition.png | Allgemeines Entwicklungsboard-Pinout, kein Anschlussplan des GUITION-Displays. Nicht zur Verdrahtung des Panels verwenden. |
| esp32-s3_datasheet_en.pdf | SoC-Datenblatt v1.3, 69 Seiten; Überblick und GPIO-/Schnittstellenbezug geprüft. Kein Board-Schaltplan. |
| esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf | Modul-Datenblatt v1.1, 34 Seiten; Modulvarianten und Einschränkung von GPIO35–37 geprüft. |
| Nsiway-NS4168.pdf | 10-seitiges Bild-PDF; Titelseite visuell geprüft. I²S-Audioverstärker, kein Touch-Controller. Für den ersten Prototyp nicht erforderlich. |

Die drei PNG-Dateien sind lesbar. Die anfänglich gemeldeten fehlenden Bildpfade verhindern die Auswertung nicht mehr.

### Widersprüche und Grenzen

- Deckblatt der Board-Spezifikation: **ESP32-4848S043C_I_Y_1/Y_3**; Folgeseiten: **ESP32-4848S040C_I**. Die genaue Variante am Gerät muss bestätigt werden.
- Parameterseite: **86,5 × 86,5 × 37,8 mm**; Maßzeichnung: **86,5 × 86,5 × 13,6 mm**. Kein Gehäuse allein nach diesen Angaben konstruieren.
- Die genannten **ca. 260 mA** sind keine belegte maximale Stromaufnahme. Stromversorgung erst nach Prüfung unter Display- und WLAN-Last endgültig dimensionieren.
- Das nachgereichte Demo konfiguriert ausdrücklich GT911. Damit ist der vorgesehene Touch-Typ im Herstellerpaket belegt; konkrete Boardrevision und tatsächliche Hardware bleiben am Gerät zu bestätigen.
- Ein Schaltplan enthält optionale Audio-/Relais- und Batteriepfade. Daraus folgt nicht, dass alle Bauteile auf dem vorhandenen Board bestückt sind.

## 4. Hardware-Arbeitsstand

| Merkmal | Stand | Beleg |
|---|---|---|
| MCU/Modul | ESP32-S3; Schaltplansymbol nennt N16R8-Variante | 2.png und Board-PDF |
| Flash / PSRAM | 16 MB / 8 MB am Gerät bestätigt | Bootlog vom 07.09.2026 |
| Panel | 480 × 480, ST7701, RGB-Datenbus mit serieller Initialisierung | PDF und 2.png |
| Touch | Kapazitiv über I²C; GT911 am Gerät auf Adresse 0x5D erkannt | Bootlog vom 07.09.2026 |
| Backlight | GPIO38 | Excel, 1.png |
| USB-C | USB-UART über CH340C an GPIO43/44 laut Schaltplan | 1.png |
| Versorgung | 5 V am vorgesehenen Eingang | PDF, 1.png |

Die ESPHome-Gerätedokumentation nennt für die 4848S040-Familie GT911 und einen ST7701S-Treiber. Das ist ein brauchbarer Vergleich für den Hardwaretest, ersetzt aber keine Identifikation unserer Revision. Quelle: [ESPHome-Geräteseite](https://devices.esphome.io/devices/guition-esp32-s3-4848s040/), geprüft am 04.09.2026.

### Konsolidierte Pinbelegung

GPIO-Nummern sind keine physischen Steckerkontaktnummern. RGB-Gruppen sind in aufsteigender Datenbitreihenfolge aufgeführt; die LCD-Bezeichnungen DBx sind aus den Quellen übernommen.

| Funktion | GPIO | LCD-/Signalbezeichnung |
|---|---|---|
| Blau, 5 Bit | 4, 5, 6, 7, 15 | DB1, DB2, DB3, DB4, DB5 |
| Grün, 6 Bit | 8, 20, 3, 46, 9, 10 | DB6, DB7, DB8, DB9, DB10, DB11 |
| Rot, 5 Bit | 11, 12, 13, 14, 0 | DB13, DB14, DB15, DB16, DB17 |
| HSYNC | 16 | HS |
| VSYNC | 17 | VS |
| Data Enable | 18 | DE |
| Pixel Clock | 21 | PCLK |
| LCD Chip Select | 39 | LCD_CS |
| LCD serielle Daten | 47 | SDA; zugleich SD-MOSI |
| LCD serieller Takt | 48 | SCK; zugleich SD-SCK |
| Backlight | 38 | BL_C / BL_CTR |
| Touch SDA | 19 | TP_SDA |
| Touch SCL | 45 | TP_SCL |
| UART TX / RX | 43 / 44 | U0TXD / U0RXD zum CH340C |
| microSD CS / MISO | 42 / 41 | TF_CS / MCU_MISO |
| Audio BCLK / LRCLK / DATA | 1 / 2 / 40 | alternativ Relaisfunktionen je nach Bestückung |

Besonderheiten:

- Touch-INT ist im Schaltplan nicht zu einem ESP32-GPIO geführt. Für den Start Touch zyklisch abfragen. LCD/Touch-Reset liegt auf einem gemeinsamen RC-Resetnetz ohne erkennbaren steuerbaren MCU-Pin.
- GPIO19/20 sind bereits Touch/RGB zugeordnet. Den USB-C-Port nach diesem Schaltplan über CH340C verwenden; keine native USB-Konfiguration für GPIO19/20 voraussetzen.
- GPIO35–37 sind trotz leerer Excel-Felder bei der R8-Variante durch Octal-PSRAM belegt und nicht frei. Beleg: Modul-Datenblatt, Pin-Definitionen, Fußnote b.
- GPIO0, 3, 45 und 46 haben Boot-Strapping-Funktionen. Vorhandene Boardbeschaltung berücksichtigen; keine zusätzlichen Lasten allein anhand der Tabelle anschließen.
- LCD-Initialisierung und SD teilen GPIO47/48. microSD bleibt im ersten Prototyp deaktiviert; spätere Nutzung braucht koordinierte Buszugriffe.
- RGB-Timing, Initialisierungssequenz und Backlight-Ansteuerung sind inzwischen im Demo konkret angegeben (Abschnitt 12). Touch-Adresse und Funktion aller Einstellungen sind am Gerät zu prüfen.

## 5. Architektur und Softwareentscheidung

Festgelegt durch die Projektübersicht: **Touchpanel → WLAN → Moonraker → Klipper → Drucker**. Mainsail greift parallel auf Moonraker zu. Das Panel funktioniert ohne geöffnetes Mainsail, benötigt aber WLAN, Raspberry Pi und Moonraker für Druckeraktionen.

Vorschlag: eigene C++-Firmware mit LVGL. Für die erste Inbetriebnahme das inzwischen vorliegende Arduino-Herstellerdemo mit den gelieferten Bibliotheken bereinigen und reproduzieren. Framework- und LVGL-Version dieses funktionierenden Beispiels festhalten; nicht gleichzeitig Treiber und LVGL auf neue Hauptversionen migrieren. Für den ersten Hardwaretest wird jetzt Arduino als Arbeitsvorschlag verwendet, weil das Herstellerdemo dafür vorliegt. Eine spätere ESP-IDF-Umstellung bleibt offen; sie ist keine Voraussetzung für den Prototyp. Windows ist vom Nutzer als Entwicklungsplattform festgelegt; VS Code mit PlatformIO ist inzwischen vom Nutzer festgelegt.

Vorgesehene Softwaremodule:

| Modul | Verantwortung |
|---|---|
| board | Als `BoardHardware` umgesetzt: Pins, Displayinitialisierung, Touch und Backlight |
| ui | Als `HardwareTestUi` umgesetzt: Klipper-Status, Hardwaretest, Systemdiagnose und Navigation |
| printer_state | Als `PrinterState` umgesetzt: zusammengeführter Druckerzustand und Datengültigkeit |
| moonraker_client | Als `MoonrakerClient` umgesetzt: WLAN, optionale Authentifizierung, Anfragen, Statusupdates und Reconnect |
| actions | Erlaubte Aktionen, Bestätigung und Rückmeldung |
| settings | WLAN, Host/Port, Zugangsdaten und Anzeigeoptionen |

Netzwerkoperationen dürfen Touch und Darstellung nicht blockieren. LVGL-Zugriffe sollen über einen klaren UI-Ausführungskontext erfolgen. WLAN-Schlüssel und Moonraker-Zugangsdaten gehören nicht in veröffentlichte Quellen oder Logs.

## 6. Erster nutzbarer Funktionsumfang

Der erste Prototyp umfasst Statusanzeige, Temperaturen, PLA/PETG-Vorheizen und Cooldown. Bewegungen und Kalibrierung folgen erst nach einem stabilen Statusmodell.

| Ausbaustufe | Funktionen |
|---|---|
| MVP | Verbindungsstatus, Hotend/Bett Ist/Soll, Druckzustand, Dateiname, Fortschritt; Preheat PLA/PETG, Cooldown |
| Alltag | Filament laden/entladen, Homing, begrenzte Bewegungen; Pause/Resume/Cancel |
| Kalibrierung | CR Touch, Schraubenabgleich, Bed Mesh, geführter Z-Offset |
| Komfort | Helligkeit, Konfiguration am Gerät/Setupportal, OTA, Thumbnail, weitere Profile |

Layoutvorschlag: Statusleiste oben, Temperaturkarten und Druckstatus in der Mitte, wenige große Navigationstasten unten. Startseite, Temperatur, Filament, Bewegung und später Leveling. Während des Drucks stehen Druckstatus und Pause/Fortsetzen im Vordergrund. Deutsche Beschriftung ist der Arbeitsvorschlag.

## 7. Moonraker-Verhalten

WebSocket über `/websocket` mit JSON-RPC ist für den laufenden Read-only-Status umgesetzt; HTTP wurde ergänzend zur Diagnose verwendet. Der lokale Zielhost ist als `192.168.178.128:7125` bestätigt. WLAN-Daten und ein optionaler Moonraker-API-Key stehen ausschließlich in der von Git ignorierten Datei `include/secrets.h`; `include/secrets.example.h` dient als Vorlage.

Nach „Klipper ready“ ermittelt der Client die verfügbaren Objekte und abonniert daraus `extruder`, `heater_bed`, `print_stats`, `virtual_sdcard` und `webhooks`. Der Start-Snapshot wird übernommen; partielle `notify_status_update`-Meldungen werden in den lokalen Zustand eingearbeitet. Bei einer Trennung werden Livewerte als ungültig markiert. Nach Wiederverbindung ermittelt und abonniert der Client die Objekte erneut. Quellen: [API-Einführung](https://moonraker.readthedocs.io/en/latest/external_api/introduction/), [Printer Objects](https://moonraker.readthedocs.io/en/latest/printer_objects/), [Printer Administration](https://moonraker.readthedocs.io/en/latest/external_api/printer/), [Notifications](https://moonraker.readthedocs.io/en/latest/external_api/jsonrpc_notifications/).

Beim ersten Gerätetest scheiterte der WebSocket-Handshake trotz erreichbarem Moonraker mit HTTP 403. Ursache war der von arduinoWebSockets voreingestellte Header `Origin: file://`, den Moonraker ablehnte. Der Client entfernt diesen optionalen Default-Header nun explizit; ein konfigurierter `X-Api-Key` bleibt möglich. Danach wurden Verbindung, Abonnement und Liveupdates erfolgreich bestätigt.

| Objektkandidat | Anzeige/Verwendung |
|---|---|
| extruder, heater_bed | Temperatur und Zieltemperatur |
| print_stats | Druckzustand, Dateiname, Druckdauer |
| virtual_sdcard | Fortschritt, sofern verfügbar |
| toolhead | Homingstatus und Position |
| gcode_move | G-Code-Koordinaten und Faktoren bei Bedarf |
| pause_resume | Pausenzustand, sofern konfiguriert |
| fan | Lüfterleistung später |

Die UI muss klar definieren, welche Koordinaten sie anzeigt. Nicht referenzierte Achsen nicht als zuverlässig positioniert darstellen. Restzeit bleibt zunächst ausgeblendet; eine spätere Schätzung wird ausdrücklich als Schätzung gekennzeichnet.

Zustände unterscheiden: WLAN getrennt, Moonraker getrennt, Klipper startet, bereit, Shutdown sowie Druck läuft/pausiert/beendet/fehlerhaft. Bei Trennung Werte als veraltet kennzeichnen und Aktionen sperren. Unbestätigte Bewegungs-, Heiz- oder Extrusionsbefehle nicht nach Reconnect automatisch wiederholen. Eine erfolgreiche API-Antwort allein bedeutet nicht, dass ein mechanischer Vorgang abgeschlossen ist.

## 8. Makros und Bedienregeln

Die Übersicht benennt gewünschte Funktionen, belegt aber nicht deren Installation. Aktuelle `printer.cfg` und eingebundene Makrodateien sind vor der Anbindung erforderlich.

| Vorgeschlagener Makroname | Zweck | Stand |
|---|---|---|
| PREHEAT_PLA | Hotend 210 °C, Bett 60 °C | Gewünschtes Profil, Existenz offen |
| PREHEAT_PETG | Hotend 240 °C, Bett 80 °C | Gewünschtes Profil, Existenz offen |
| COOLDOWN | Heizungen aus | Existenz offen |
| LOAD_PLA / UNLOAD_PLA | Aufheizen und Filament bewegen | Länge/Geschwindigkeit und Existenz offen |
| LOAD_PETG / UNLOAD_PETG | Aufheizen und Filament bewegen | Länge/Geschwindigkeit und Existenz offen |

Makronamen sind ein Vorschlag, keine bestätigte Schnittstelle. Temperaturprofile sind Projektvorgaben, keine neu geprüfte Freigabe der Hotendhardware. ASA bleibt bis zur Festlegung passender Temperaturen und Hardwarebedingungen ausgenommen.

Bewegungen benötigen Referenzierung und Achsgrenzen; Extrusion benötigt geeignete Temperatur. Sperren gehören auch in die Klipper-Abläufe, weil Mainsail parallel bedienen kann. Filamentwechsel, Leveling und manuelles Verfahren während eines laufenden Drucks bleiben in der ersten Fassung gesperrt. Cooldown während eines Drucks nicht als unauffällige Schnellaktion anbieten.

Abbruch und Neustart benötigen eindeutige Bestätigung. `SAVE_CONFIG` ist keine gewöhnliche Speichertaste: Der Ablauf muss den anschließenden Verbindungswechsel/Neustart berücksichtigen. Z-Offset wird später als geführter Dialog mit Fortschritt, Anpassung, Übernahme und Abbruch umgesetzt; ein einzelner Startknopf genügt nicht. Ein WLAN-Bedienpanel ist kein physischer Not-Aus.

## 9. Umsetzung in überprüfbaren Schritten

| Schritt | Arbeit | Abschlusskriterium |
|---|---|---|
| 1 – Gerät identifizieren | Platinenfotos, Aufdruck und Anschlüsse mit dem bereits geprüften Herstellerdemo abgleichen | Variante, Flashweg und Quellbeispiel bekannt |
| 2 – Hardwaretest | Flash/PSRAM auslesen; Farbflächen, Text, Touchpunkte und Backlight testen | Korrekte Farben, alle Ecken bedienbar, keine Resets; auch unter WLAN-Last stabil |
| 3 – Firmwarebasis | Versionen fixieren, Module anlegen, zwei LVGL-Seiten | Reproduzierbarer Build und flüssige Navigation |
| 4 – Status ohne Steuerung | WLAN/Moonraker, Temperaturen und Zustand; Ausfall und Reconnect | Automatische Rückkehr zu aktuellen Daten ohne Neustart des Panels |
| 5 – Erste Aktionen | Geprüfte Vorheiz-/Cooldown-Makros, Fehlerrückmeldung | Aktion sichtbar bestätigt; Fehler und Verbindungsausfall korrekt dargestellt |
| 6 – Alltag | Filament, Bewegung, Pause/Resume/Cancel | Zustandsabhängige Sperren und Bestätigungen am Drucker geprüft |
| 7 – CR Touch | Erst nach bestätigtem Einbau und funktionierender Klipper-Konfiguration | Kalibrierablauf einschließlich Abbruch getestet |
| 8 – Montage | Ausrichtung, reale Maße, Stromversorgung und Halterung | Stabil montiert, zugänglicher USB-Port, keine Versorgungsausfälle |

Die Statusintegration liegt bewusst vor den Steueraktionen: Erst mit verlässlichem Druckerzustand kann das Panel passende Bedienfunktionen freigeben.

Stand 07.09.2026: Schritte 1 bis 4 sind abgeschlossen. Beim Reconnect-Test wurde die WLAN-Verbindung des laufenden Panels einmalig getrennt. WLAN und WebSocket wurden ohne Panel-Neustart automatisch wiederhergestellt, die Statusobjekte erneut abonniert und aktuelle Werte wieder angezeigt. Es wurden keine Druckerbefehle gesendet.

## 10. Noch benötigte Informationen

Für den unmittelbar nächsten Schritt werden die relevante Moonraker-/Klipper-Konfiguration ohne Geheimnisse und `printer.cfg` einschließlich eingebundener Makrodateien benötigt. Vorheizen und Cooldown werden nur an tatsächlich vorhandene, geprüfte Makros angebunden. Der aktuelle CR-Touch-Installationsstand bleibt erst für den späteren Kalibrierschritt relevant. WLAN-Passwörter müssen nicht im Chat geteilt werden.

## 11. Entscheidungs- und Testprotokoll

| ID | Entscheidung/Status | Begründung |
|---|---|---|
| D01 | Übernommen: WLAN/Moonraker | Bestehende Projektarchitektur |
| D02 | Übernommen: Druckerabläufe in Klipper | Einheitliches Verhalten bei Panel und Mainsail |
| D03 | Vorgeschlagen: C++/LVGL; Arduino für ersten Hardwaretest | Herstellerdemo mit gelieferten Versionen reproduzieren; endgültiges Framework offen |
| D04 | Vorgeschlagen: MVP ohne SD, Audio, Relais | Für tägliche Druckerbedienung zunächst unnötig |
| D05 | Vorgeschlagen: Status vor Aktionen | Zustandsabhängige Bedienung zuverlässig umsetzen |
| D06 | Umgesetzt: lokale Secrets-Datei mit versionierbarer Vorlage | Zugangsdaten bleiben aus Repository und seriellen Logs heraus |
| D07 | Umgesetzt: dynamisches Read-only-Abonnement | Nur auf dem Drucker vorhandene Statusobjekte werden verwendet |

Teststatus 07.09.2026: Dokumente, Bibliotheksmetadaten und relevante Demoquellen geprüft. Hosttests, vollständige PlatformIO-Builds und Hardwaretests bestanden. Firmware-Upload, Bootdiagnose, Bild, Farben, Touch, Ecktasten, Loslassen und Backlight funktionieren. 16 MB Flash, rund 8 MB PSRAM und GT911 auf Adresse 0x5D sind am Gerät bestätigt. Firmware 0.3.0 verbindet sich mit WLAN und Moonraker, zeigt Klipper `bereit` sowie laufend aktualisierte Temperaturen und stellt die Verbindung nach einem WLAN-Abbruch ohne Panel-Neustart wieder her. Druckersteuernde Aktionen wurden noch nicht implementiert oder getestet.

Änderungsprotokoll: Version 0.1 – neun Quelldateien gesichtet, GPIO-Tabelle konsolidiert, Dokumentwidersprüche erfasst, MVP und Meilensteine vorgeschlagen. Künftige Änderungen erhalten Datum, Beleg und Teststatus; bestätigte Ergebnisse ersetzen offene Annahmen.


## 12. Nachgereichte Herstellerpakete – Auswertung

### Operating instructions.zip

Enthält `Operating instructions.txt`, `boards.txt` und `huge_app.csv`. Die Anleitung fordert Arduino-ESP32 mindestens 2.0.6; Beispielpfade beziehen sich auf 2.0.11. Dies belegt einen historischen Herstellerstand, keine getestete Kompatibilität mit allen neueren Versionen.

Die Anleitung bestätigt die gemeinsame Nutzung von GPIO1/2/40 durch Audio und Relais. Für Audio sollen R25/R26/R27 auf R21/R22/R23 umgesetzt werden. Das ist für unser erstes Panel nicht erforderlich; Bestückung am Gerät prüfen, bevor Änderungen erwogen werden.

Partitionstabelle: NVS ab 0x9000, OTA-Metadaten ab 0xE000, eine Factory-App ab 0x10000 mit Größe 0x7E0000 (7,875 MiB) und Coredump ab 0x7F0000. Sie belegt den Bereich bis 8 MiB; ein zweiter App-Slot für reguläre OTA-Updates fehlt. Vorhandene OTA-Metadaten allein ermöglichen kein solches Update.

Widerspruch: In `boards.txt` ist für ESP32S3 Dev Module der Eintrag Huge APP weiterhin mit 3 MB Uploadlimit (3145728 Bytes) definiert. Die Anleitung spricht von „max app“. Globale Dateien daher nicht ungeprüft ersetzen; für das Projekt eine konsistente Partitionierung und Größenbegrenzung festlegen.

### Libraries.zip

| Bibliothek | Gelieferter Stand | Einordnung |
|---|---|---|
| LVGL | 8.3.9, Metadaten und lvgl.h stimmen überein | Referenz für die GUI |
| Arduino_GFX | 1.2.9 laut library.properties | ST7701-RGB- und ESP32-RGB-Treiber enthalten; gelieferten Quellstand bewahren |
| Touch_GT911 | Keine Versionsmetadaten vorhanden | Eigener GT911-Treiber mit zwei Quelldateien |
| ArduinoJson | 6.17.2 | Historischer Lieferstand; die Statusfirmware verwendet fest 6.21.5 aus der PlatformIO Registry |
| HTTPClient | 1.2 | Mitgelieferte Kopie; Konflikte mit Core-Bibliothek vor Installation prüfen |
| Time | 1.6.1 | Für den ersten Hardwaretest nicht erforderlich |
| NtpClientLib | 3.0.2-beta | Für den ersten Hardwaretest nicht erforderlich |
| ArduinoZlib | 0.0.1 | Für den ersten Hardwaretest nicht erforderlich |

Der enthaltene Touch-Treiber prüft I²C-Lesefehler nicht und begrenzt die ausgelesene Touchanzahl nicht auf das Array mit fünf Punkten. Die Prüfsummenvariable in `calculateChecksum()` ist nicht initialisiert. Dieser Originaltreiber wurde nicht verändert. Die Testfirmware nutzt stattdessen einen eigenen Treiber SafeGT911 mit geprüften Leseoperationen und ohne Schreiben der Controllerkonfiguration.

### 1_2_4.0_LvglWidgets.zip

Das ausgewählte Demo enthält Sketch, `touch.h`, Widgets-Quellen und Bildressourcen, eine separate `lv_conf.h` sowie eine zusätzliche Panel-Initialisierungsreferenz als Textdatei. Die relevanten Quellen wurden gelesen. Der erneut als `1_2_4.0_LvglWidgets(1).zip` bereitgestellte Dateiname bezeichnet den nachgereichten Upload; daraus wird keine neue Softwareversion abgeleitet.

| Einstellung | Tatsächlicher Demo-Code |
|---|---|
| Auflösung / Rotation | 480 × 480 / 0 |
| Treiber | Arduino_ST7701_RGBPanel, IPS=true, BGR=true |
| Initialisierungssequenz | st7701_type1_init_operations aus der gelieferten Arduino_GFX-Bibliothek |
| Pixeltakt | 16 MHz über gfx->begin(16000000) |
| Horizontal Front Porch / Puls / Back Porch | 10 / 8 / 50 |
| Vertikal Front Porch / Puls / Back Porch | 10 / 8 / 20 |
| RGB- und Steuerpins | Stimmen mit der konsolidierten Tabelle in Abschnitt 4 überein |
| Backlight | GPIO38, HIGH zum Einschalten |
| Touch | GT911; SDA=19, SCL=45, INT=-1, RST=-1 |
| Touch-Ausrichtung | ROTATION_NORMAL; X- und Y-Mapping jeweils 480 → 0 |
| LVGL-Farben | LV_COLOR_DEPTH=16; LV_COLOR_16_SWAP=0 |
| LVGL-Zeitbasis | LV_TICK_CUSTOM=1; millis() |
| LVGL-Heap | 128 KiB, LV_MEM_CUSTOM=0 |
| LVGL-Zeichenpuffer | 480 × 200 × 2 = 192000 Bytes im internen RAM |
| RGB-Framebuffer | Treiber fordert Framebuffer in PSRAM an |

Die `lv_conf.h` trägt im Kommentar v8.3.3, während LVGL selbst 8.3.9 ist. Das ist ein Versionshinweis, noch kein nachgewiesener Buildfehler. Die zusätzliche Panel-Textdatei weicht in Registerwerten von der im Sketch ausgewählten Bibliothekssequenz ab; für den ersten Test die tatsächlich verwendete Kombination als Referenz erhalten und nicht beide Sequenzen vermischen.

**Buildblocker im unveränderten Herstellerdemo:** `touch.h` inkludiert `TAMC_GT911.h` und instanziiert `TAMC_GT911`. Im gelieferten Bibliothekspaket ist dieser Treiber nicht enthalten; dort existieren `Touch_GT911.h` und `Touch_GT911`. Das Projekt ist mit diesen Dateien allein unverändert nicht vollständig. Für die Testfirmware den vorhandenen Treiber gezielt einbinden und prüfen oder die exakte fehlende Bibliotheksversion beschaffen. Eine bloße Namensänderung ist kein ausreichender Funktionstest.

**Speicherplanung:** Die 192000 Bytes Zeichenpuffer plus 128 KiB LVGL-Heap beanspruchen zusammen 323072 Bytes, zusätzlich zu sonstigem internem Speicherbedarf. Vorschlag: im bereinigten Test mit 20–40 Pufferzeilen beginnen (19200–38400 Bytes) und Anzeigeverhalten sowie freien Heap prüfen. Im Testpaket umgesetzt: 30 Zeilen/28800 Bytes und 64 KiB LVGL-Heap. Hardwaremessung noch offen.

## 13. Windows-Entwicklung und unmittelbarer Arbeitsplan

**Vom Nutzer bestätigt:** Entwicklung und Flashen unter Windows. VS Code mit PlatformIO ist als Entwicklungsumgebung festgelegt. Windows-Version und Installationsstand bleiben offen. Der folgende Arduino-IDE-Ablauf beschreibt den bisherigen Paketstand; maßgeblich ist jetzt Abschnitt 16.

1. Arbeitsvorschlag für den ersten Test: Arduino-Umgebung mit bewusst fixiertem ESP32-Core aus der Herstellerreferenz; 2.0.11 ist ein Startkandidat aus den gelieferten Pfaden, noch kein getesteter Projektstandard. Kein pauschales Update auf neueste Bibliotheksversionen während der Inbetriebnahme.
2. Boardprofil ESP32S3 Dev Module, 16 MB Flash und OPI-PSRAM anhand des Geräts bestätigen. Flash- und PSRAM-Größe beim Boot ausgeben lassen.
3. USB-Datenkabel verwenden und CH340C als COM-Port in Windows identifizieren. Treiberinstallation erst bei Bedarf behandeln. USB-CDC über native ESP32-Pins nicht voraussetzen.
4. Bereinigte Testfirmware erstellen: Touch-Abhängigkeit vereinheitlichen, Fehlerbehandlung ergänzen, Speicherbedarf reduzieren und Demo-Konfiguration nachvollziehbar übernehmen.
5. Build zuerst prüfen, dann am Gerät Farben, Text, Touch an allen Ecken, Loslassen und Backlight testen. Ergebnisse und tatsächlich verwendete Versionen hier protokollieren.
6. Danach WLAN/Moonraker zunächst nur lesend anbinden; erst mit verlässlichem Status Steueraktionen ergänzen.

Testfirmware 0.1.0 im Quellcode erstellt und eigener Touch-Treiber auf dem Host getestet. Vollständiger ESP32-Build sowie Flash- und Hardwaretest stehen aus. Kein Druckerzugriff durchgeführt.

## 14. Pflege der Projektdokumentation

Auf ausdrücklichen Wunsch des Nutzers wird diese bestehende Datei bei der weiteren Projektarbeit laufend erweitert und aktualisiert. Windows, neue Erkenntnisse, Entscheidungen, Konfigurationsänderungen, Probleme und Testergebnisse werden direkt im jeweiligen Abschnitt nachgeführt. Überholte Aussagen werden ersetzt, wichtige Änderungen zusätzlich im Änderungsprotokoll festgehalten. Quellenbefund, Arbeitsvorschlag und am Gerät bestätigtes Ergebnis bleiben getrennt. Die Pflege erfolgt im Zuge unserer Projektarbeit, nicht als unbeaufsichtigter Hintergrundprozess.

| Version | Datum | Änderung |
|---|---|---|
| 0.1 | 04.09.2026 | Grunddokumentation aus neun Dateien, Pinout und Meilensteine |
| 0.2 | 04.09.2026 | Windows bestätigt; drei Herstellerarchive ausgewertet; LVGL-/GFX-Versionen, GT911-Konfiguration, Display-Timing, fehlende Touch-Abhängigkeit, Speicher- und Partitionsprobleme ergänzt; offene Punkte und nächster Schritt aktualisiert |


## 15. Testpaket 0.1.0 – erstellt

Datei: `ESP32-Klipper-Touchtest-v0.1.0.zip`. Enthält einen vollständigen Arduino-Sketch mit projektspezifischer LVGL-Konfiguration, benötigten Herstellerbibliotheken, Windows-Anleitung, simulierten Touch-Tests, Validierungsprotokoll und SHA-256-Dateiliste. Bibliotheken sind als eigenes Sketchbook gebündelt; keine Hersteller-`boards.txt` oder globale Partitionstabelle muss überschrieben werden.

Implementiert: Farbvergleich Rot/Grün/Blau/Weiß; vier quittierbare Ecktasten; Touchkoordinaten und Fehleranzeige; Helligkeitsregler; Backlight-Aus für 1,5 Sekunden mit automatischer Rückkehr; Speicherdiagnose und regelmäßige serielle Statusausgabe. Keine WLAN- oder Druckerbefehle.

Die fehlende TAMC_GT911-Abhängigkeit ist durch `SafeGT911.h` ersetzt. Der Treiber prüft an 0x5D/0x14 die Produktkennung 911, prüft Leselängen, lehnt Touchanzahlen über fünf ab und lässt bei Fehlern los. Nach Verbindungsfehlern erfolgt ein neuer Erkennungsversuch. Koordinaten werden auf gültige Werte geprüft, ohne Controllerkonfiguration oder Resetpins zu ändern. Hardwareausrichtung bleibt am Gerät zu verifizieren.

Aktuelle Speicherbudgets: Zeichenpuffer 28800 Bytes, LVGL-Heap 65536 Bytes, zusammen 94336 Bytes statt 323072 Bytes im Herstellerdemo, jeweils zuzüglich anderer Speicherbelegungen. RGB-Framebuffer weiterhin in PSRAM. Helligkeits-PWM 1 kHz als noch zu prüfende Testeinstellung.

Zielkonfiguration für den Windows-Test: Arduino-ESP32 2.0.11, ESP32S3 Dev Module, 16 MB Flash, QIO 80 MHz, OPI PSRAM, Standardpartition „8M with spiffs (3MB APP/1.5MB SPIFFS)“, USB CDC deaktiviert, UART0/Hardware CDC, serieller Monitor 115200 Baud. Die kleinere Standardpartition nutzt nicht den gesamten erwarteten Flash und dient nur dem Hardwaretest. Die Core-Version wird im Sketch geprüft. Diese Konfiguration ist vorbereitet, noch nicht durch einen vollständigen ESP32-Build bestätigt.

Validierung: C++11-Hosttests für den eigenen Touch-Treiber mit strengen Compilerwarnungen sowie Address-/UndefinedBehavior-Sanitizer bestanden. Diese Tests simulieren I²C, nicht das Display oder den ESP32. Die vollständige Toolchaininstallation scheiterte an für Arduino CLI unerreichbaren Paketquellen; deshalb kein ESP32-Binary, kein erfolgreicher Linktest und kein Hardwaretest. Windows-Anleitung und `VALIDIERUNG.md` nennen diese Grenze ausdrücklich.

Nächster konkreter Schritt: Paket unter Windows entpacken, eigenes Sketchbook auswählen, Core 2.0.11 einrichten und Sketch zuerst kompilieren. Danach Flashen und Abnahme anhand der Tabelle in README-Windows.md. Firmware-Upload ersetzt die vorhandene Gerätedemo.

Änderungsprotokoll 0.3 / 04.09.2026: Testpaket erstellt, fehlende Touch-Abhängigkeit ersetzt, Speicherbudget reduziert, Hosttests bestanden; Windows-Anleitung ergänzt; vollständiger Build als blockiert und Hardwaretests als offen dokumentiert.


## 16. Umstellung auf VS Code, PlatformIO und geplantes GitHub-Repository

**Vom Nutzer festgelegt:** Zuhause wird auf Windows mit VS Code und PlatformIO entwickelt. Der Nutzer hat das Repository erstellt und dessen URL mitgeteilt: [noncon66/esp32-klipper-touchdisplay](https://github.com/noncon66/esp32-klipper-touchdisplay). Sichtbarkeit und aktueller Inhalt konnten hier nicht verifiziert werden; der Webabruf war technisch blockiert. Es besteht noch keine bestätigte Synchronisation mit dem vorbereiteten Projektpaket.

Neues Arbeitsartefakt: `ESP32-Klipper-PlatformIO-v0.1.1.zip`. Dieses Paket ersetzt die Arduino-IDE-Anleitung als aktuellen Einstieg. Das bisherige Paket 0.1.0 bleibt ein historischer Stand.

Umgesetzt: Sketch in `src/main.cpp` überführt; Touch-Treiber und LVGL-Konfiguration nach `include/`; mitgelieferte Bibliotheken nach `lib/`; `platformio.ini`, Git-Ausschlüsse, VS-Code-Erweiterungsempfehlung und neue README ergänzt. Hardwarelogik und Hosttestfälle bleiben erhalten. Dokumentation und Abnahmeanleitung liegen unter `docs/`. Für PlatformIO ist keine Arduino-IDE-Installation und kein globaler Sketchbook-Wechsel nötig.

Buildbasis: PlatformIO espressif32 **6.4.0**, laut eingesehenem Plattformmanifest Arduino-Paket **~3.20011.0** (Core 2.0.11). Der Code prüft den Core exakt auf 2.0.11. Basisboard `esp32-s3-devkitc-1`, mit Overrides auf 16 MB Flash, `qio_opi`, PSRAM und bestehende 8-MB-Partitionierung. Das Basisboard selbst bezeichnet eine N8-Variante ohne PSRAM; die Overrides sind daher wesentlich. Standard-UART-Ausgabe über den CH340C bleibt aktiv, USB CDC on boot ist deaktiviert.

Bibliotheken werden für die erste Inbetriebnahme als lokale Quellstände mitgeführt. `.pio`, lokale IDE-Daten und vorbereitete Secrets-Dateinamen sind per `.gitignore` ausgeschlossen. Für eigenen Code ist noch keine öffentliche Lizenz festgelegt. Drittanbieterhinweise bleiben erhalten. Noch keine GitHub-Automatisierung oder Remote-Synchronisation eingerichtet.

Validierung: Plattformmanifest und Boarddefinition aus dem getaggten PlatformIO-Quellstand geprüft, Projektpfade/INI geprüft und Hosttests nach Pfadumstellung erneut ausgeführt. Der vollständige PlatformIO-Build wurde am 06.09.2026 unter Windows erfolgreich ausgeführt. Windows-Upload und Hardwaretest stehen aus.

Nächster Schritt für den Nutzer: neuen Paketordner in VS Code öffnen, PlatformIO-Erweiterung installieren bzw. initialisieren lassen, **Build** starten. Danach Upload/Monitor und Hardwareabnahme. Repository-Link liegt vor. Als Nächstes das Repository unter Windows klonen und den vorbereiteten PlatformIO-Projektinhalt in dessen Wurzel übernehmen; vorhandene Dateien vorher vergleichen.

Änderungsprotokoll 0.4 / 04.09.2026: Windows + VS Code + PlatformIO als verbindliche Entwicklungsumgebung festgehalten; Projektpaket auf PlatformIO konvertiert; GitHub-Vorbereitung und noch fehlende Repository-Anbindung dokumentiert. Firmware-Paketversion 0.1.1.


## 17. Repository angelegt

Stand 04.09.2026, vom Nutzer bestätigt: **https://github.com/noncon66/esp32-klipper-touchdisplay**.

Repository-Name: `esp32-klipper-touchdisplay`, Eigentümer: `noncon66`. Der lokale Arbeitsstand ist mit `origin/main` unter dieser Adresse verbunden. Der Standardbranch ist `main`; vor Beginn des Builds war der Arbeitsbaum sauber und mit `origin/main` synchronisiert. Die Sichtbarkeit des Repositorys wurde nicht geprüft.

Der bereitgestellte PlatformIO-Quellstand wurde in das lokale Repository übernommen und liegt auf `main`. Die Datei `platformio.ini` befindet sich direkt im geöffneten Repositoryordner. Der Build wurde inzwischen erfolgreich ausgeführt; Upload und Hardwaretest bleiben offen.

Änderungsprotokoll 0.5 / 04.09.2026: Vom Nutzer angelegtes GitHub-Repository vermerkt; Linkanforderung erledigt; tatsächliche Synchronisation und Repositoryprüfung bleiben offen. Keine Firmwareänderung.


## 18. Erster vollständiger PlatformIO-Build

Am 06.09.2026 wurde die Umgebung `guition_touchtest` unter Windows mit PlatformIO Core 6.2.0 vollständig kompiliert und gelinkt. Verwendet wurden wie festgelegt Platform `espressif32` 6.4.0, Arduino-ESP32 2.0.11, Arduino_GFX 1.2.9 und LVGL 8.3.9. `firmware.elf`, `firmware.bin`, `bootloader.bin` und `partitions.bin` wurden erfolgreich erzeugt.

Buildgröße: 497125 Bytes Flash von 3342336 Bytes (14,9 %) und 86780 Bytes statischer RAM von 327680 Bytes (26,5 %). Der Build meldete zwei `const`-Qualifier-Warnungen innerhalb der unveränderten LVGL-8.3.9-Quellen; im eigenen Projektcode wurden keine Compilerfehler gemeldet.

Die lokale PlatformIO-Installation war zunächst unvollständig: In der virtuellen Umgebung fehlte PlatformIO Core und die portable Python-3.11.7-Standardbibliothek war entfernt. Core und offizielle portable Runtime wurden wiederhergestellt. Diese Reparatur betrifft die lokale Entwicklungsumgebung und keine Repository-Datei.

Der erfolgreiche Build bestätigt Compiler-, Bibliotheks- und Linkkompatibilität, aber nicht die elektrische oder funktionale Eignung am Gerät. Nächster Schritt: Display per USB-Datenkabel anschließen, COM-Port prüfen, Firmware hochladen, seriellen Bootlog aufzeichnen und die Abnahme in `docs/Hardwaretest.md` durchführen.

Änderungsprotokoll 0.6 / 06.09.2026: Ersten vollständigen PlatformIO-Build bestanden; Speicherbelegung und lokale Toolchain-Reparatur dokumentiert; Repository-Verbindung bestätigt. Hardwaretest bleibt offen.


## 19. Erster Upload und Bootdiagnose

Am 07.09.2026 wurde die Hardwaretest-Firmware über den vom System erkannten Port COM5 auf das angeschlossene Display geschrieben. `esptool.py` identifizierte einen ESP32-S3 Revision v0.2. Bootloader, Partitionstabelle und Firmware wurden mit 460800 Baud übertragen; die Hashprüfung aller geschriebenen Daten war erfolgreich. Anschließend erfolgte ein automatischer Hardware-Reset.

Der serielle Bootlog bei 115200 Baud bestätigt Arduino-ESP32 2.0.11, 16777216 Bytes Flash und 8386279 Bytes nutzbare PSRAM-Größe. Der GT911 wurde auf I²C-Adresse 0x5D mit Produktkennung 911 erkannt. Die Firmware erreichte den Zustand `Bereit`; der interne LVGL-Zeichenpuffer umfasst 28800 Bytes. Nach zehn Sekunden blieben freier Heap mit 267432 Bytes und freie PSRAM mit 7925019 Bytes stabil, der Touchstatus war `OK` und der I²C-Fehlerzähler stand auf 0.

Damit sind Flashweg, Speichererkennung, Firmwarestart und grundlegende Touch-Kommunikation bestätigt. Der Nutzer bestätigte anschließend die korrekte Darstellung der dunklen Testoberfläche und der vier Farbfelder Rot, Grün, Blau und Weiß. Auch alle vier Ecktasten, gültige Koordinaten, korrekte räumliche Zuordnung, Klickzähler und Loslassen wurden bestätigt. Helligkeitsregler, kurzzeitiges Ausschalten und automatische Rückkehr funktionieren ebenfalls.

Der abschließende serielle Stabilitätstest lief nach einem definierten Neustart bis Uptime 610 Sekunden. Während des gesamten Laufs blieben der freie Heap bei 267432 Bytes und die freie PSRAM bei 7925019 Bytes konstant. Der Touchstatus blieb `OK`, der I²C-Fehlerzähler blieb 0 und es trat kein weiterer Reset auf. Der erste vollständige Hardwaretest ist damit bestanden.

Änderungsprotokoll 0.7 / 07.09.2026: Ersten Firmware-Upload und seriellen Boottest bestanden; ESP32-S3 Revision, Flash, PSRAM und GT911 am Gerät bestätigt. Visuelle und interaktive Hardwareabnahme gestartet.

Änderungsprotokoll 0.8 / 07.09.2026: Bild-, Farb-, Touch-, Ecktasten-, Loslass- und Backlight-Prüfung bestanden; Zehn-Minuten-Stabilitätstest ohne Reset, Speicherverlust oder I²C-Fehler abgeschlossen. Erster Hardwaretest vollständig bestanden.


## 20. Modulare Firmwarebasis 0.2.0

Am 07.09.2026 wurde die bisherige monolithische Testfirmware in eine modulare Basis überführt. `BoardHardware` kapselt Displaybus, ST7701-Initialisierung, LVGL-Treiberregistrierung, GT911, Wiedererkennung und Backlight. `HardwareTestUi` enthält die LVGL-Objekte, Ereignisbehandlung und Aktualisierung der Anzeige. `main.cpp` beschränkt sich auf Initialisierung, zyklische Dienste, LVGL-Ausführung und serielles Statuslogging.

Zusätzlich zur bestehenden Display- und Touchtestseite wurde eine Systemdiagnoseseite ergänzt. Sie zeigt Firmwarestand, GT911-Verbindung und Adresse, I²C-Fehler, Flash, PSRAM, freien Heap, freie PSRAM, Laufzeit und Helligkeit. Animierte Schaltflächen wechseln zwischen beiden Seiten.

Der vollständige PlatformIO-Build war erfolgreich. Firmware 0.2.0 benötigt 499413 Bytes Flash von 3342336 Bytes (14,9 %) und 86812 Bytes statischen RAM von 327680 Bytes (26,5 %). Der Upload über COM5 und der Boot auf dem ESP32-S3 waren erfolgreich; GT911 wurde auf 0x5D erkannt und der I²C-Fehlerzähler blieb 0.

Der Nutzer bestätigte auf der Hardware beide Seiten, die korrekte Live-Diagnose, flüssige Navigation in beide Richtungen sowie die unveränderte Funktion von Touchtest, Ecktasten und Helligkeitssteuerung. Damit ist Schritt 3 der Umsetzungsplanung abgeschlossen. Als Nächstes folgt Schritt 4: WLAN und Moonraker zunächst nur lesend anbinden, einen lokalen Druckerzustand aufbauen und Verbindungsabbruch sowie Reconnect testen.

Änderungsprotokoll 0.9 / 07.09.2026: Hardware- und UI-Code modularisiert; zweite LVGL-Systemdiagnoseseite ergänzt; Build, Upload, Boot und Navigation auf Hardware bestanden. Firmwarebasis 0.2.0 und Projektschritt 3 abgeschlossen.


## 21. Read-only-Moonraker-Status 0.3.0

Am 07.09.2026 wurde Projektschritt 4 umgesetzt und auf der Zielhardware bestätigt. `PrinterState` hält WLAN-, Moonraker-, Klipper- und Druckzustand sowie Temperaturen, Dateiname, Fortschritt, Aktualisierungszeit und Gültigkeitskennzeichen. `MoonrakerClient` betreibt WLAN und WebSocket nicht blockierend im Hauptzyklus. Die Statusseite ist nun die Startseite; Hardwaretest und Systemdiagnose bleiben über große Navigationstasten erreichbar.

Die Firmware fragt nach dem WebSocket-Aufbau zunächst `server.info` ab. Sobald Klipper bereit ist, folgt `printer.objects.list`. Anschließend werden nur vorhandene Objekte mit den benötigten Feldern abonniert. Unterstützt sind derzeit Hotend- und Heizbetttemperatur samt Zielwert, Druckzustand, Dateiname, Fortschritt und Klipper-Zustand. Startdaten und partielle Benachrichtigungen werden in denselben lokalen Zustand eingearbeitet. Dieser Stand enthält bewusst keine Methode zum Auslösen von G-Code, Makros oder anderen Druckeraktionen.

Die lokale Konfiguration ist aufgeteilt: `include/secrets.example.h` ist eine versionierbare Vorlage; die echte Datei `include/secrets.h` wird durch `.gitignore` ausgeschlossen. Der Quellcode und die seriellen Meldungen geben weder WLAN-Namen noch Passwort oder API-Key aus. Als Ziel wurde `192.168.178.128:7125` bestätigt. ArduinoJson 6.21.5 und WebSockets 2.4.1 sind in `platformio.ini` festgelegt. Das zusätzliche Buildskript `scripts/framework_library_paths.py` stellt der älteren WebSockets-Bibliothek die benötigten WiFi-Includepfade aus dem festgelegten Arduino-ESP32-Core portabel bereit.

Beim ersten Liveversuch erhielt der ESP32 eine WLAN-Adresse, der WebSocket wurde jedoch sofort getrennt. Ein unabhängiger HTTP-Test bestätigte Moonraker mit Status 200; ein unabhängiger WebSocket-Test funktionierte ebenfalls. Der gezielte Vergleich zeigte, dass Moonraker den Bibliotheksstandard `Origin: file://` mit HTTP 403 ablehnt. Seit der Client diesen optionalen Header entfernt und kein unnötiges Subprotokoll anfordert, werden WebSocket und Statusabonnement erfolgreich aufgebaut.

Endgültiger Build: 1080145 Bytes Flash von 3342336 Bytes (32,3 %) und 113684 Bytes statischer RAM von 327680 Bytes (34,7 %). Upload über COM5 und Hashprüfung waren erfolgreich. Der Bootlog bestätigt WLAN, Moonraker und Statusabonnement; Touch blieb `OK`, der I²C-Fehlerzähler blieb 0. Der Nutzer bestätigte auf dem Display Klipper-Zustand `bereit` und sauber aktualisierte Hotend-/Heizbetttemperaturen.

Für die Reconnect-Abnahme wurde ausschließlich in einer temporären Testfirmware die WLAN-Verbindung des Panels einmalig getrennt. Das Panel kennzeichnete die Verbindung als getrennt, verband WLAN und WebSocket erneut und abonnierte die Statusobjekte wieder. Bereits bei Uptime 15 Sekunden waren WLAN und Moonraker wieder verbunden; der Heap blieb stabil. Danach wurde der Testauslöser entfernt und der bereinigte Endstand erneut gebaut, hochgeladen und im Bootlog geprüft. Damit ist das Abschlusskriterium von Schritt 4 erfüllt.

Nächster Schritt ist Projektschritt 5: tatsächlich vorhandene Vorheiz- und Cooldown-Makros aus der Klipper-Konfiguration prüfen, eine eng begrenzte Aktionsschnittstelle mit Rückmeldung entwerfen und erst danach steuernde UI-Elemente hinzufügen.

Änderungsprotokoll 1.0 / 07.09.2026: Firmware 0.3.0 mit lokalem Druckerzustand, sicherer Secrets-Vorlage, nicht blockierender WLAN-/Moonraker-Verbindung und dritter LVGL-Statusseite umgesetzt. Origin-403 diagnostiziert und behoben. Livewerte, Verbindungsabbruch, automatische Wiederverbindung, erneutes Abonnement, finaler Build und Upload auf Hardware bestanden. Projektschritt 4 abgeschlossen.
