# RACETRACK – To-Do Liste | Yeonsu

- [x] **1. Entwicklungsumgebung**
  - [x] ESP32 konfigurieren & Blink-Test erfolgreich

- [ ] **2. UART-Kommunikation**
  - [ ] ESP32 ↔ PyBadge verbinden
  - [ ] UART testen
  - [ ] Befehle senden/empfangen (BPM_UP / BPM_DOWN / START / STOP)

- [ ] **3. PyBadge (UI)**
  - [ ] UART-Empfang implementieren
  - [ ] Buttons → UART-Befehle senden
  - [ ] Display für NOTE / BPM / STATUS
  - [ ] Nur UI-Logik (keine Berechnung)

- [ ] **4. ESP32 Datenstruktur**
  - [ ] BlockType (LEFT / RIGHT / STRAIGHT)
  - [ ] Track-Array + playhead

- [ ] **5. Eingabesystem (ESP32)**
  - [ ] Serielle Eingabe (L / R / S / X)
  - [ ] Track-Erstellung testen

---

- [ ] **6. Musik-Engine (ESP32)**
  - [ ] Circle of Fifths implementieren
  - [ ] Notenberechnung basierend auf Track-Richtung

- [ ] **7. Playback-Engine (ESP32)**
  - [ ] Playhead-Loop implementieren
  - [ ] BPM-System im ESP32
  - [ ] Timing mit millis()

- [ ] **8. Integrationstest**
  - [ ] UART-Kommunikation testen (ESP32 ↔ PyBadge)
  - [ ] BPM beeinflusst Playback
  - [ ] Track → Note → Loop funktioniert
  - [ ] Display zeigt Live-Daten korrekt

- [ ] **Optional (Future)**
  - [ ] Pedal-Eingang für BPM (ESP32 übernimmt Steuerung)
  - [ ] Buzzer/Audio-Ausgabe