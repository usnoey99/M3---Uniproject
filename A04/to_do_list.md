# RACETRACK – To-Do Liste | Yeonsu

- [x] **1. Entwicklungsumgebung**
  - [x] ESP32 konfigurieren & Blink-Test erfolgreich

- [x] **2. UART-Kommunikation**
  - [x] ESP32 <-> PyBadge verbinden
  - [x] UART testen
  - [x] Befehle senden/empfangen (BPM_UP / BPM_DOWN / START / STOP)

- [x] **3. PyBadge (UI)**
  - [x] UART-Empfang implementieren
  - [x] Buttons -> UART-Befehle senden
  - [x] Display für NOTE / BPM / STATUS
  - [x] Nur UI-Logik (keine Berechnung)

- [x] **4. ESP32 Datenstruktur**
  - [x] BlockType (LEFT / RIGHT / STRAIGHT)
  - [x] Track-Array + playhead

- [x] **5. Eingabesystem (ESP32)**
  - [x] Serielle Eingabe (L / R / S / X)
  - [x] Track-Erstellung testen


Ich habe das technische Fundament erfolgreich fertiggestellt und die UART-Verbindung zwischen ESP32 und PyBadge stabil eingerichtet. Auf dem PyBadge läuft die komplette UI-Logik (CircuitPython) zur Anzeige von BPM, Note und Status sowie zum Senden von Button-Befehlen. Auf dem ESP32 wurde die Track-Datenstruktur implementiert und auf 18 Blöcke optimiert. Das Eingabesystem über den PC-Serienmonitor liest Streckenbefehle (L/R/S/X) fehlerfrei in das Array ein.

---

- [ ] **6. Musik-Engine (ESP32)**
  - [ ] Circle of Fifths implementieren
  - [ ] Notenberechnung basierend auf Track-Richtung

- [ ] **7. Playback-Engine (ESP32)**
  - [ ] Playhead-Loop implementieren
  - [ ] BPM-System im ESP32
  - [ ] Timing mit millis()

- [ ] **8. Integrationstest**
  - [ ] UART-Kommunikation testen (ESP32 <-> PyBadge)
  - [ ] BPM beeinflusst Playback
  - [ ] Track -> Note -> Loop funktioniert
  - [ ] Display zeigt Live-Daten korrekt

- [ ] **Optional (Future)**
  - [ ] Pedal-Eingang für BPM (ESP32 übernimmt Steuerung)
  - [ ] Buzzer/Audio-Ausgabe