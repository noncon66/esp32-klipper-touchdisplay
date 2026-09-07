# Hardwaretest am GUITION-Display

## 1. Was erscheinen soll

- Dunkle Testoberfläche mit vier beschrifteten Farbfeldern: Rot, Grün, Blau, Weiß.
- GT911-Verbindungsstatus, Koordinaten und I²C-Fehlerzähler.
- Vier große Ecktasten OL / OR / UL / UR. Nach Antippen wird die jeweilige Taste grün; der Zähler steigt erst bei abgeschlossenem Klick.
- Helligkeitsregler und Taste zum Ausschalten für 1,5 Sekunden; die Beleuchtung kehrt automatisch zurück.
- Im seriellen Monitor: Flash-/PSRAM-Größe, freier Heap und regelmäßige Statuszeilen.

Es werden weder WLAN-Zugangsdaten noch Moonraker-Adresse benötigt. Die Firmware enthält keine Druckerbefehle.

## 2. Abnahme am Gerät

Bitte diese Ergebnisse zurückmelden:

| Prüfung | Erwartung / Rückmeldung |
|---|---|
| Boot | 16 MB Flash, 8 MB PSRAM erkannt; keine Neustartschleife |
| Farben | Beschriftung und Farbe stimmen überein; Weiß ist neutral |
| Bild | Vollständig, keine Streifen, kein ständiges Flackern |
| Ecktasten | Alle vier reagieren am richtigen Ort |
| Koordinaten | Bereich 0–479; keine Spiegelung oder Achsvertauschung |
| Loslassen | Nach Wegnehmen des Fingers „losgelassen“, keine hängende Betätigung |
| Backlight | Regler wirkt; Ausschalten und automatische Rückkehr funktionieren |
| Stabilität | Mindestens 10 Minuten Bedienung ohne Reset, I²C-Fehlerzähler bleibt idealerweise 0 |

Bei spiegelverkehrtem Touch zunächst ein Foto der Anzeige und beobachtete Koordinaten schicken; nicht gleichzeitig Displayrotation und Touchtransformation verändern. Der eigene Treiber verwendet direkte Rohkoordinaten als Ausgangspunkt, entsprechend der Nettoausrichtung des Herstellerbeispiels ohne dessen Rundungseffekt.

Falls GT911 nicht erkannt wird, laufen Anzeige und Diagnose weiter. Alle zwei Sekunden werden die Adressen 0x5D und 0x14 erneut auf die Produktkennung 911 geprüft. Reset- und Interruptleitungen werden nicht vom ESP32 angesteuert. Die Controllerkonfiguration wird nicht umgeschrieben.

## 3. Testergebnis vom 07.09.2026

Der vollständige Hardwaretest wurde bestanden. Bestätigt wurden 16 MB Flash, rund 8 MB PSRAM, GT911 auf Adresse 0x5D, korrekte Farben und Bilddarstellung, alle vier Ecktasten, gültige und richtig zugeordnete Touchkoordinaten, Loslassen sowie Helligkeitsregelung und automatische Backlight-Rückkehr.

Der serielle Stabilitätstest lief bis Uptime 610 Sekunden ohne Reset. Freier Heap blieb bei 267432 Bytes, freie PSRAM bei 7925019 Bytes, Touchstatus bei `OK` und der I²C-Fehlerzähler bei 0. Stabilität unter WLAN-Last ist nicht Bestandteil dieses Tests und bleibt für die spätere Moonraker-Integration offen.

