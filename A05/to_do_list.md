# RACETRACK – To-Do Liste A05 | Yeonsu
- [x] **1. PyBadge Racing-Simulator (Prototyp)**
  - [x] Pseudo-3D Rennstrecke mit Basis-Formen (Himmel, Gras, Straße, Auto) implementiert
  - [x] Dynamische Scrolling-Animation der Fahrbahnmarkierungen (BPM fungiert als temporäres Gaspedal)
  - [x] UI-Anzeige für "GEAR" (Oktave) als statischer Platzhalter vorbereitet
  - [x] Temporäre Tastensteuerung (Button A/B für BPM) für die Prototyp-Phase ohne Hardware-Pedal eingerichtet

  Der grafische Prototyp für das PyBadge-Display wurde heute erfolgreich fertiggestellt. Die bisherige textbasierte Anzeige wurde durch eine Pseudo-3D-Rennstrecke aus Basis-Formen (TileGrids) ersetzt. Neu hinzugekommen ist eine flüssige Scrolling-Animation der Fahrbahnmarkierungen. Da das Hardware-Pedal noch nicht angeschlossen ist, fungieren die Buttons A und B temporär als Gaspedal, um die BPM und damit die Animationsgeschwindigkeit zu steuern. Die Anzeige für den "GEAR" (spätere Oktave) ist bereits als statischer Platzhalter in der UI integriert. Die UART-Kommunikation läuft fehlerfrei parallel zur neuen Rendering-Schleife. Der nächste Schritt ist die Erstellung und Integration der finalen Pixel-Art-Assets, um die Platzhalter-Formen zu ersetzen.

- [ ] **10. Grafik-Assets (Pixel Art) erstellen**
  - [ ] Zeichen-Tool auswählen (z.B. Piskel (piskelapp.com) oder Pixilart (pixilart.com))
  - [ ] Sprites entwerfen (Auto, Hintergrund, UI-Elemente)
  - [ ] Assets formatgerecht exportieren (max. 160x128 Pixel, .bmp Format / 256 Farben)

- [ ] **11. PyBadge Grafik-Integration (CircuitPython)**
  - [ ] BMP-Dateien in den CIRCUITPY-Speicher übertragen
  - [ ] adafruit_imageload Bibliothek einbinden
  - [ ] Bisherige Platzhalter-Formen (TileGrids) durch echte Pixel-Art-Sprites ersetzen