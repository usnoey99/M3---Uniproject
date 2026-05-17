# RACETRACK – Prototype

## Beschreibung
Ein einfacher Musik-Step-Sequencer für den Adafruit PyBadge.

## Funktionen
- Circle-of-Fifths Musiksystem (F, C, G, D, A, E, B)
- Schrittbasierte Sequenz (Track-System)
- BPM-Steuerung mit Buttons (A = langsamer, B = schneller)
- Display zeigt Note, BPM und Track-Status
- WAV-Audio wird pro Note abgespielt
- START-Button startet den Sequencer
- SELECT-Button pausiert den Sequencer

## Status
- Sequencer funktioniert
- Display funktioniert
- BPM-Steuerung funktioniert
- WAV-Wiedergabe funktioniert jetzt
- Start/Stop-Steuerung funktioniert

## Aktuelle Einschränkungen
- PyBadge unterstützt nur einen Audio-Stream gleichzeitig
- Keine echte gleichzeitige Wiedergabe mehrerer WAV-Dateien
- Echtzeit-Audio-Mixing ist aktuell nicht möglich

## Nächste Schritte
- Mehrspur-Audio / Audio-Engine untersuchen
- BPM-abhängige Audio-Geschwindigkeit testen
- Hardware-Input mit Microswitches und Pedalen erweitern