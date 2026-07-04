
#define I2C_ADDRESS  0x13   //change
#define DIRECTION    0     // Richtung: 0=STRAIGHT, 1=LEFT, 2=RIGHT
// =============================================

#include <USIWire.h>  


#define PIN_KICK      3   // PB3, ADC3
#define PIN_CHAIN_IN  4   // PB4
#define PIN_CHAIN_OUT 1   // PB1

// Statusvariablen 
uint8_t kick_val     = 0;     
uint8_t snare_val    = 0;     
bool chain_active    = false; 
bool selected        = false; 

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

//   decodeKick() 
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

// Chain

void handleChain() {
    bool chain_in = (digitalRead(PIN_CHAIN_IN) == HIGH);

    if (selected) {
        // Chain-OUT bleibt HIGH damit das naechste Modul aktiviert bleibt
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, HIGH);
    }
    else if (chain_in) {
       
        chain_active = true;
        digitalWrite(PIN_CHAIN_OUT, LOW); //  Modul noch warten lassen
    }
    else {
        chain_active = false;
        digitalWrite(PIN_CHAIN_OUT, LOW);
    }
}


//(Wire.requestFrom)

void requestEvent() {
    Wire.write(DIRECTION);
    Wire.write(kick_val);
    Wire.write(snare_val);
    Wire.write(chain_active ? 1 : 0);
}


//   0x01 = NEXT: 
//   0x00 = RESET: 
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
    pinMode(PIN_CHAIN_IN,  INPUT);   //Signal vom vorherigen Modul
    pinMode(PIN_CHAIN_OUT, OUTPUT);  // Sendet Signal 
    digitalWrite(PIN_CHAIN_OUT, LOW); // Erstmal LOW (noch nicht aktiv)

    // I2C starten mit der definierten Adresse
    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(requestEvent); 
    Wire.onReceive(receiveEvent);  
}

// LOOP
// I2C wird automatisch im Hintergrund von der USIWire Library verwaltet
// requestEvent() und receiveEvent() werden automatisch aufgerufen 
void loop() {
    uint16_t kick_adc = readStableADC(PIN_KICK);
    kick_val          = decodeKick(kick_adc);
    snare_val         = 0;
    handleChain();
    delay(5);
}