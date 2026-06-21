
// Kalibrierungs-Sketch – Resistor Ladder
// Ladder an A0 anschliessen
// VCC der Ladder an Uno 5V, GND an Uno GND

#define PIN_LADDER A0

void setup() {
    Serial.begin(115200);
    
}

void loop() {
    // 16x mal messen für stabile werte (durchschnitt)
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(PIN_LADDER);
        delay(2);
    }
    int val = sum / 16;

    Serial.println(val); //eigentliche Messung ausgabe
    delay(300);
}
