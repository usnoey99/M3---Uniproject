RACETRACK – To-Do Liste | Yeonsu

- [x] **1. Entwicklungsumgebung**
  - [x] ESP32 konfigurieren & Blink-Test erfolgreich
- [ ] **2. UART-Kommunikation**
  - [ ] ESP32 und PyBadge physisch verbinden (TX2 -> RX, GND -> GND)
  - [ ] Testdaten fehlerfrei senden und empfangen (115200 Baud)
- [ ] **3. Display-System (PyBadge)**
  - [ ] UART-Empfänger codieren
  - [ ] Live-Anzeige für NOTE, BPM und STATUS einrichten
- [ ] **4. & 5. Datenstruktur & Eingabe (ESP32)**
  - [ ] Block-Typen (LEFT, RIGHT, STRAIGHT) und Track-Array definieren
  - [ ] Serielle PC-Eingabe (L / R / S / X) für Track-Erstellung aktivieren

---

- [ ] **6. Musik-Engine (ESP32)**
  - [ ] Quintenzirkel (Circle of Fifths) implementieren
  - [ ] Notenberechnung basierend auf Track-Richtung codieren
- [ ] **7. Playback-Engine (ESP32)**
  - [ ] Playhead-Steuerung und automatischen Track-Loop (Schleife) bauen
  - [ ] BPM- und Gang-System (Gear) verknüpfen
- [ ] **8. Hardware-Verkabelung & Audio**
  - [ ] Physische Track-Sensoren an ESP32 anschließen und testen
  - [ ] Lautsprecher/Buzzer über Breadboard verbinden und Ton ausgeben
- [ ] **9. Integrationstest**
  - [ ] Gesamtes System testen: Sensor -> ESP32 -> Audio -> UART -> PyBadge-Display