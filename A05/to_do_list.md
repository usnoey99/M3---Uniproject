# RACETRACK – To-Do Liste A05 | Yeonsu
- [x] **1. PyBadge Racing-Simulator (Prototyp)**
  - [x] Pseudo-3D Rennstrecke mit Basis-Formen (Himmel, Gras, Straße, Auto) implementiert
  - [x] Dynamische Scrolling-Animation der Fahrbahnmarkierungen (BPM fungiert als temporäres Gaspedal)
  - [x] UI-Anzeige für "GEAR" (Oktave) als statischer Platzhalter vorbereitet
  - [x] Temporäre Tastensteuerung (Button A/B für BPM) für die Prototyp-Phase ohne Hardware-Pedal eingerichtet

Der grafische PyBadge-Prototyp ist fertiggestellt: Die einfache Text-UI wurde durch eine Pseudo-3D-Rennstrecke mit flüssiger Scrolling-Animation ersetzt. Da das Hardware-Pedal noch fehlt, steuern die Buttons A/B temporär die BPM (Animationsgeschwindigkeit). Die "GEAR"-Anzeige (Oktave) dient vorerst als statischer Platzhalter, während die UART-Kommunikation fehlerfrei parallel zum neuen Rendering läuft.

- [x] **2. Grafik-Assets (Pixel Art) erstellen**
  - [x] Zeichen-Tool auswählen (z.B. Piskel (piskelapp.com) oder Pixilart (pixilart.com))
  - [x] Sprites entwerfen (Auto, Hintergrund, UI-Elemente)
  - [x] Assets formatgerecht exportieren (max. 160x128 Pixel, .bmp Format / 256 Farben)

- [x] **3. PyBadge Grafik-Integration (CircuitPython)**
  - [x] BMP-Dateien in den CIRCUITPY-Speicher übertragen
  - [x] adafruit_imageload Bibliothek einbinden
  - [x] Bisherige Platzhalter-Formen (TileGrids) durch echte Pixel-Art-Sprites ersetzen

Zudem wurden die bisherigen grafischen Platzhalter durch finale Pixel-Art-Assets (Retro-Hintergrund und Auto) ersetzt. Die erstellten 24-Bit-BMP-Dateien wurden mithilfe der adafruit_imageload-Bibliothek erfolgreich in den CircuitPython-Code eingebunden.

- [x] **4. ESP32 Audio-Synthesizer (Prototyp)**
  - [x] BluetoothA2DPSource Bibliothek integrieren
  - [x] Echtzeit-Klangsynthese (Sinuswelle) im ESP32 programmieren
  - [x] Notenfrequenzen (Circle of Fifths) mit der Audio-Ausgabe verknüpfen
  - [x] Bluetooth-Streaming an externen Lautsprecher einrichten

- [x] **5.BPM & Playback Synchronisation**
  - [x] Notenlänge dynamisch an die aktuelle BPM anpassen (80% des Beat-Intervalls)
  - [x] Audio-Trigger fehlerfrei in die bestehende millis()-Playback-Loop integrieren

Ein Echtzeit-Audiosynthesizer (Sinuswelle) wurde auf dem ESP32 implementiert, der die Frequenzen des Quintenzirkels berechnet und kabellos via Bluetooth A2DP streamt. Tonlänge und Timing passen sich dabei dynamisch der aktuellen BPM an. Damit ist das System nun visuell (PyBadge) und akustisch vollständig synchronisiert. Ein Praxistest mit einem echten Bluetooth-Lautsprecher steht aktuell noch aus und muss zeitnah durchgeführt werden.