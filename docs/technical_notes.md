# Technische Notizen – „Racetrack“

## 1. Systemarchitektur

Das System besteht aus drei Hauptkomponenten:

- physische Streckenmodule
- Mikrocontroller-basierte Steuerung
- visuelle und akustische Ausgabe (Pi Badge + LEDs)

---

## 2. Modulerkennungssystem

### Geplanter Ansatz: Widerstandsbasierte Identifikation

Jedes Modul besitzt:
- einen definierten Widerstandswert
- der beim Einlesen erkannt wird

### Ziel:
- eindeutige Klassifikation jedes Streckenteils
- einfache Hardware-Implementierung ohne komplexe Kommunikation

### Risiko:
- Messungen können ungenau sein
- Kontaktprobleme möglich

---

## 3. Alternative Erkennungsansätze

### Option 1: Mikrocontroller pro Modul
- jedes Modul sendet ID an Hauptsystem
- sehr zuverlässig
- jedoch höhere Kosten und Komplexität

---

### Option 2: QR-Code-Erkennung
- visuelle Identifikation durch Kamera
- einfache Implementierung auf Modulebene

### Nachteil:
- erfordert initiales Scanning aller Module
- weniger geeignet für dynamische Änderungen während des Spiels

---

## 4. LED-System (NeoPixel)

### Einsatz:
- visuelle Darstellung der Fahrzeugposition
- Statusanzeige (z. B. Fehlerzustände)

### Technisches Risiko:
NeoPixel-LEDs blockieren während der Datenübertragung den Mikrocontroller.

#### Auswirkungen:
- keine parallele Verarbeitung möglich
- mögliche Verzögerungen im System
- Einschränkung von Echtzeit-Interaktion

---

## 5. Performance-Überlegungen

### Problem:
- viele LED-Elemente erhöhen Rechenlast
- Echtzeit-Synchronisation von Musik und Licht kritisch

### Empfehlung:
- LED-Updates minimieren oder gruppieren
- klare Trennung zwischen Logik und visueller Ausgabe

---

## 6. Physische Skalierung

### Risiko:
Große Streckensysteme (Carrera-Größe):
- hoher Platzbedarf
- mechanische Instabilität möglich
- erhöhte Komplexität bei Verkabelung

### Empfehlung:
- kleiner Prototyp für erste Entwicklungsphase

---

## 7. Eingabesysteme

### Gaspedal:
- Potentiometer-basierte Eingabe
- optional mechanisches 3D-gedrucktes Pedal

### Gangsystem:
- digitale Buttons im Prototyp
- mögliche spätere mechanische Umsetzung

---

## 8. Audio-System

### Grundprinzip:
- jedes Streckenelement triggert Soundevents
- Sequenzierung basiert auf Fahrzeugbewegung

### Erweiterungsideen:
- Akkordbasierte Audioausgabe
- harmonische Anpassung über Streckenkombinationen
- Loop-basierte Musikstruktur

---

## 9. Systemidee: Sequencer-Logik

Das Fahrzeug kann als „Playhead“ eines Sequencers interpretiert werden.

### Prinzip:
- Bewegung = Trigger für Sound
- Strecke = musikalische Sequenzstruktur

### Vorteil:
- klare Synchronisation von Bewegung und Musik
- einfache Erweiterbarkeit für komplexe Musiksysteme

---

## 10. Risiken und offene Fragen

- Zuverlässigkeit der Modulerkennung
- Synchronisation von Audio und Licht
- Skalierbarkeit des physischen Systems
- Balance zwischen Spiel, Musik und Puzzle