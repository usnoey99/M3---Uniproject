
#define I2C_ADDRESS  0x10  // Chip-Adresse: 0x10=STRAIGHT, 0x11=LEFT, 0x12=RIGHT
#define DIRECTION    0     // Richtung: 0=STRAIGHT, 1=LEFT, 2=RIGHT
// =============================================

#include <USIWire.h>  


#define PIN_KICK      3   // PB3, ADC3
#define PIN_SNARE     0     // PB5/ADC0 = Snare Ladder (nach Fuse-Aenderung!)
#define PIN_CHAIN_IN  4   // PB4
#define PIN_CHAIN_OUT 1   // PB1

// Statusvariablen 
uint8_t kick_val     = 0;     // Aktuelles Kick-Bitmuster (0-15, je nach Jumper)
uint8_t snare_val    = 0;     // Snare-Wert (0-15, je nach Jumper)
bool chain_active    = false; // dran (chain_active=1 = ja)
bool selected        = false; // vom ESP32 erkannt und gespeichert yes or no

// Liest den Analog-Pin stabil aus

uint16_t readStableADC(uint8_t pin) {
    analogRead(pin);          // Dummy-Read: erster Wert wird weggeworfen
    delayMicroseconds(200);   // Kurz warten bis Multiplexer stabil ist

    long sum = 0;
    for (uint8_t i = 0; i < 16; i++) {
        sum += analogRead(pin);       // 16 Messungen addieren
        delayMicroseconds(100);       // Kurze Pause zwischen Messungen
    }
    return sum / 16;  // Durchschnitt 
}

//   Kick
// Wandelt den ADC-Wert (0-1023) in ein Bitmuster (0-15) um.
uint8_t decodeKick(uint16_t val) {
   
    if      (val > 605) return 0;   // kein Jumper
    else if (val > 588) return 1;   // J1
    else if (val > 569) return 2;   // J2
    else if (val > 548) return 3;   // J1+J2
    else if (val > 524) return 4;   // J3
    else if (val > 499) return 5;   // J1+J3
    else if (val > 470) return 6;   // J2+J3
    else if (val > 438) return 7;   // J1+J2+J3
    else if (val > 401) return 8;   // J4
    else if (val > 361) return 9;   // J1+J4
    else if (val > 316) return 10;  // J2+J4
    else if (val > 262) return 11;  // J1+J2+J4
    else if (val > 201) return 12;  // J3+J4
    else if (val > 129) return 13;  // J1+J3+J4
    else if (val > 45) return 14;  // J2+J3+J4
    else                return 15;  // alle Jumper
}

//   Snare 
uint8_t decodeSnare(uint16_t val) {
   
    if      (val > 605) return 0;   // kein Jumper
    else if (val > 588) return 1;   // J1
    else if (val > 569) return 2;   // J2
    else if (val > 548) return 3;   // J1+J2
    else if (val > 524) return 4;   // J3
    else if (val > 499) return 5;   // J1+J3
    else if (val > 470) return 6;   // J2+J3
    else if (val > 438) return 7;   // J1+J2+J3
    else if (val > 401) return 8;   // J4
    else if (val > 361) return 9;   // J1+J4
    else if (val > 316) return 10;  // J2+J4
    else if (val > 262) return 11;  // J1+J2+J4
    else if (val > 201) return 12;  // J3+J4
    else if (val > 129) return 13;  // J1+J3+J4
    else if (val > 45) return 14;  // J2+J3+J4
    else                return 15;  // alle Jumper
}

// Chain
//   1. ESP32 setzt GPIO4 HIGH → erstes Modul bekommt Signal an Chain-IN
//   2. Dieses Modul: chain_in=HIGH → chain_active=true → meldet sich per I2C
//   3. ESP32 liest chain_active=1 → speichert dieses Modul als Position 0
//   4. ESP32 schickt NEXT (0x01) → selected=true, Chain-OUT=HIGH
//   5. Chain-OUT HIGH geht ans naechste Modul als Chain-IN
void handleChain() {
    bool chain_in = (digitalRead(PIN_CHAIN_IN) == HIGH);

    if (selected) {
        // Ich wurde schon erkannt und gespeichert
        // Chain-OUT bleibt HIGH damit das naechste Modul aktiviert bleibt
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, HIGH);
    }
    else if (chain_in) {
       
        chain_active = true;
        digitalWrite(PIN_CHAIN_OUT, LOW); // naechstes Modul noch warten lassen
    }
    else {
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, LOW);
    }
}


//Wire
//   Byte 1: DIRECTION (0=STRAIGHT, 1=LEFT, 2=RIGHT)
//   Byte 2: kick_val  (Jumper-Bitmuster 0-15)
//   Byte 3: snare_val (Jumper-Bitmuster 0-15)
//   Byte 4: chain_active (1=ich bin dran, 0=ich bin nicht dran)
void requestEvent() {
    Wire.write(DIRECTION);
    Wire.write(kick_val);
    Wire.write(snare_val);
    Wire.write(chain_active ? 1 : 0);
}


//   0x01 = NEXT:  erkannt, leitet weiter
//   0x00 = RESET
void receiveEvent(int numBytes) {
    if (numBytes < 1) return;
    uint8_t cmd = Wire.read();

    if (cmd == 0x01) {
        selected     = true;
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, HIGH);
    }
    else if (cmd == 0x00) {
        selected     = false;
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, LOW);
    }
}

//SETUP
void setup() {
    // Chain-Pins konfigurieren

    // PB5 als normaler Input nach Fuse-Aenderung
    // Fuse-Bits: lfuse=0xE2, hfuse=0x5F
    DDRB  &= ~(1 << PB5);  // PB5 als Input
    PORTB &= ~(1 << PB5);  // kein interner Pull-up

    pinMode(PIN_CHAIN_IN,  INPUT);   // Empfaengt Signal vom vorherigen Modul
    pinMode(PIN_CHAIN_OUT, OUTPUT);  // Sendet Signal ans naechste Modul
    digitalWrite(PIN_CHAIN_OUT, LOW); // Erstmal LOW (noch nicht aktiv)

    // I2C starten mit der definierten Adresse
    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(requestEvent); 
    Wire.onReceive(receiveEvent);  
}

// LOOP
//  Kick und Snare-Jumper auslesen 
//   Chain-Signal verwalten 
//  Kurze Pause 
// I2C wird automatisch im Hintergrund von der USIWire Library verwaltet
// requestEvent() und receiveEvent() werden automatisch aufgerufen wenn noetig
void loop() {
    uint16_t kick_adc = readStableADC(PIN_KICK);
    kick_val   = decodeKick(kick_adc);
    uint16_t snare_adc = readStableADC(PIN_SNARE);
    snare_val = decodeSnare(snare_adc);
    handleChain();
    delay(5);
}