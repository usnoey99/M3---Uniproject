# Konzeptdokument – „Racetrack“

## 1. Grundidee des Projekts

„Racetrack“ ist ein interaktives System, bei dem physische Rennstrecken-Module zusammengesetzt werden.  
Diese Module bilden gleichzeitig eine musikalische Komposition.

Der Name „Racetrack“ spielt bewusst mit zwei Bedeutungen:
- Rennstrecke (physisch, spielerisch)
- Musik-Track (kompositorisch)

---

## 2. Inspirationsquellen

Das Konzept orientiert sich an klassischen und physischen Spielsystemen wie:
- Carrera-Rennbahnen
- Modelleisenbahnen
- LEGO-Bausystemen

Der Fokus liegt auf physischem Bauen mit direkter Rückkopplung in Musik und Interaktion.

---

## 3. Streckenmodule

Es gibt vier grundlegende Modultypen:

- Linkskurve
- Rechtskurve
- S-Kurve
- Gerade

### Musikalische Bedeutung der Module

Jedes Modul enthält eine musikalische Struktur:
- eigene Melodie oder Soundsequenz
- ca. 8 Noten pro Modul
- konsistenter Klangcharakter pro Modultyp

Durch die Kombination der Module entstehen:
- unterschiedliche Streckenlayouts
- unterschiedliche musikalische Abläufe

---

## 4. Modulerkennung

Jedes Streckenelement wird technisch eindeutig identifiziert.

### Geplanter Ansatz:
- Widerstandswerte pro Modultyp
- gleiche Modultypen teilen sich denselben Widerstand
- Mikrocontroller liest den Widerstand aus und identifiziert das Modul

---

## 5. Interaktion und Spielerlebnis

### Fahrzeugdarstellung
- visuelle Darstellung eines Autos auf einem Display (Pi Badge)
- Animation der Bewegung entlang der Strecke
- visuelle Effekte wie „Wind“ oder Geschwindigkeitseffekte

---

### Beschleunigungssystem
- Nutzung eines Potentiometers als Gaspedal
- optional 3D-gedrucktes Pedal
- Funktion:
  - Druck erhöht Geschwindigkeit
  - Loslassen reduziert Geschwindigkeit

---

### Gangschaltung / Tonhöhen
- physische Buttons oder Hebel als Gangsystem
- mögliche Funktionen:
  - Änderung der Tonhöhe
  - Änderung der Oktave
  - Veränderung musikalischer Parameter

---

## 6. Erweiterte musikalische Ideen

### Loop-System
- Musik kann beim Erreichen des Startpunkts gespeichert und wiederholt werden
- erzeugt musikalische Layer und Harmonien

---

### Akkord-System
- einzelne Module können auch Akkorde repräsentieren
- mögliche 4-Akkord-Struktur basierend auf 4 Modultypen
- Erweiterung der Melodieebene zu harmonischer Struktur

---

## 7. Multiplayer-Konzept

- nur ein aktives Steuergerät (Pi Badge)
- mehrere Personen können gleichzeitig die Strecke bauen
- kollaboratives physisches Gameplay wie bei LEGO-Systemen

---

## 8. Visuelle Effekte (LED-System)

- Einsatz von NeoPixel-LED-Streifen entlang der Strecke
- Darstellung von:
  - Fahrzeugposition
  - Geschwindigkeit
  - Zustandsänderungen

### Fehlerzustand („Red Flag“)
- bei fehlerhafter Strecke:
  - rote LED-Signale
  - visuelle Warnung auf Display

---

## 9. Erweiterungsvision

- realistischeres Fahrgefühl durch alternative Eingabegeräte (z. B. Schaltmechanik)
- dynamische Musikentwicklung während der Fahrt
- stärkere Verbindung zwischen physischem Aufbau und musikalischem Ergebnis