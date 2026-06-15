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