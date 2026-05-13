# 📄 Feedback zur Projektidee „Racetrack“

## 1. Projektüberblick

Das Projekt „Racetrack“ kombiniert ein physisches Bau- und Spielsystem mit Musikgenerierung.

Spieler:innen bauen eine Rennstrecke aus modularen Elementen. Diese Strecke bestimmt gleichzeitig eine musikalische Komposition. Dadurch entsteht eine Verbindung zwischen:
- physischem Puzzle-System
- Rennspiel-Mechanik
- musikalischer Interaktion

---

## 2. Feedback zur musikalischen Struktur

### Positive Rückmeldung
Die Grundidee, Musik über physische Interaktion zu erzeugen, wurde als sehr interessant und kreativ bewertet.

---

### Zentrale Kritik: Zufällige musikalische Zuordnung

Die aktuelle Idee, Streckenteile direkt mit Melodien oder Akkorden zu verknüpfen, kann problematisch sein.

#### Problem:
- Akkorde/Melodien werden möglicherweise zu zufällig kombiniert
- musikalische Logik kann verloren gehen
- Ergebnis wirkt eventuell nicht „komponiert“, sondern konstruiert

---

### Vorschlag: Orientierung am Quintenzirkel

Statt feste Akkorde einzelnen Modulen zuzuweisen, sollte das System stärker auf musikalische Bewegung setzen.

#### Idee:
- Rechtskurve = Bewegung im Quintenzirkel in eine Richtung
- Linkskurve = Bewegung in die Gegenrichtung

#### Vorteil:
- harmonisch nachvollziehbare Progressionen
- musikalisch konsistentere Ergebnisse
- höhere Qualität der entstehenden Musik

---

### Tonale Stabilität

Es wurde empfohlen, die musikalische Struktur stärker zu stabilisieren:

- Start und Ende eines musikalischen Durchlaufs sollten tonal zusammenpassen
- Melodien sollten auf einem gemeinsamen Grundton stabilisiert werden

→ sorgt für ein „abgeschlossenes“ musikalisches Gefühl

---

## 3. Strukturelles Kernproblem: Drei gleichzeitige Ziele

Das Projekt verfolgt gleichzeitig drei starke Konzepte:

- Rennspiel
- physisches Puzzle
- Musikinstrument / Musikgenerator

### Problem:
Diese Ziele stehen teilweise in Konkurrenz zueinander.

#### Beispiel:
Spieler:innen optimieren nicht musikalisch, sondern:
- um die Strecke wieder schließen zu können

→ dadurch wird musikalische Kreativität eingeschränkt

---

## 4. Feedback zur musikalischen Richtung

### Eindruck
Die Musik sollte stärker an ein Rennspiel angepasst sein.

### Kritik:
Die bisherige musikalische Idee kann zu ruhig oder abstrakt wirken und passt eventuell nicht ausreichend zum Racing-Kontext.

---

### Vorschläge:
Inspiration aus klassischen Racing-Soundtracks:
- Fast & Furious
- Asphalt-Reihe

### Anforderungen an die Musik:
- hoher Energielevel
- schneller Rhythmus
- klare Dynamik
- „Driving Feeling“

---

## 5. Scope-Problem des Projekts

### Problem:
Das Projekt ist sehr breit angelegt und versucht mehrere komplexe Systeme gleichzeitig umzusetzen:
- Rennspiel
- Puzzle-System
- Musiksystem

### Risiko:
Keines der Systeme wird ausreichend tief oder überzeugend umgesetzt.

---

### Empfehlung:
Ein klarer Fokus sollte definiert werden.

Mögliche Optionen:
- Fokus auf Musiksystem
- Fokus auf Rennspiel
- Fokus auf Puzzle-Mechanik

Die anderen Elemente sollten dann unterstützend wirken.

---

### Alternative Designidee:
- feste Rennstrecke als Grundlage
- Umgebungselemente erzeugen Sound (z. B. Häuser, Bäume, Objekte)

→ reduziert Komplexität und stärkt musikalischen Fokus

---

## 6. Kritik am Loop-System (geschlossene Strecke)

### Problem:
Das aktuelle Konzept basiert auf einem geschlossenen Rundkurs.

#### Konsequenzen:
- starke Einschränkung beim Bauen
- Zwang zur Rückkehr zum Startpunkt
- reduzierte kreative Freiheit

---

### Vorschlag: Sprint-Strecke

Statt eines Rundkurses:
- lineare Strecke (Start → Ziel)

#### Vorteile:
- mehr Freiheit beim Design
- weniger strukturelle Einschränkungen
- einfachere musikalische Logik

---

## 7. Technische Hinweise (NeoPixel)

### Problem:
NeoPixel-LEDs können den Mikrocontroller während des Schreibens blockieren.

#### Risiko:
- Verzögerungen im System
- Einschränkung paralleler Prozesse
- mögliche Probleme bei Echtzeit-Interaktion

---

## 8. Erweiterungsideen für Interaktion

### Track-Switching / Verzweigungen
- alternative Routen im Streckenverlauf
- dynamische Musikwechsel
- A/B-Strukturen möglich

---

### Parallele Tracks
- mehrere gleichzeitig laufende Pfade
- mehrere musikalische Layer

#### Einschränkung:
- Synchronisation bei unterschiedlich langen Pfaden ist schwierig

---

## 9. Physische Skalierung

### Problem:
Große physische Systeme (Carrera-ähnlich) können problematisch sein:
- hoher Platzbedarf
- schwierige Umsetzung
- mögliche Hardwareprobleme (z. B. Wärme, Stabilität)

### Empfehlung:
- kleiner Prototyp für erste Iteration

---

## 10. Erkennung der Module

### Aktueller Ansatz:
- Widerstandswerte zur Identifikation von Modulen

### Herausforderungen:
- technische Zuverlässigkeit unklar
- Fehleranfälligkeit möglich

---

### Alternative Ansätze:
- Mikrocontroller pro Modul (eindeutige Identifikation)
- QR-Code-basierte Erkennung

#### Hinweis:
QR-Code-Systeme erfordern ggf. komplettes Scanning vor Spielbeginn und können die Nutzererfahrung verlangsamen.

---

## 11. Sequencer-Idee (starkes Konzept)

### Grundidee:
Das Fahrzeug fungiert als „Playhead“ eines musikalischen Sequencers.

#### Funktionsprinzip:
- Bewegung auf der Strecke triggert Sounds
- jedes Streckenelement ist ein musikalisches Event

### Vorteil:
- klare Verbindung zwischen Bewegung und Musik
- gut verständliche musikalische Logik
- hohe Erweiterbarkeit

---

## 12. Alternative Systemidee: Stadt- oder Board-System

### Konzept:
Ein festes Spielfeld (z. B. 2×2 Meter) mit vorgegebenem Straßennetz.

### Spieler:innen platzieren:
- Gebäude
- Objekte
- Elemente

### Diese erzeugen:
- Beats
- Sounds
- musikalische Strukturen

### Vorteile:
- stabile Grundstruktur
- bessere Skalierbarkeit
- weniger technische Komplexität

---

## 13. Zusammenfassung der wichtigsten Erkenntnisse

### 1. Fokus des Projekts muss klar definiert werden
Aktuell besteht eine Überladung aus:
- Racing
- Puzzle
- Musik

→ eine klare Priorisierung ist notwendig

---

### 2. Das Loop-Konzept ist kritisch
Die Notwendigkeit, eine geschlossene Strecke zu bauen:
- reduziert kreative Freiheit
- beeinflusst musikalische Entscheidungen negativ

---

### 3. Musiksystem sollte stärker strukturiert sein
Empfehlungen:
- Quintenzirkel
- harmonische Progressionen
- Tonalität und musikalische Stabilität

---

### 4. Racing-Charakter der Musik stärken
- hohe Energie
- schneller Rhythmus
- klare Dynamik
- „Driving Feeling“

---

### 5. Technische Risiken berücksichtigen
- NeoPixel-Blocking
- Skalierungsprobleme
- Track-Erkennung
- physische Komplexität

---

### 6. Sequencer-Ansatz hat hohes Potenzial
Die Idee, das System als musikalischen Sequencer zu interpretieren, wurde als besonders vielversprechend bewertet.