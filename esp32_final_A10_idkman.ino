
// Bluetooth + ATtiny I2C + Chain + NeoPixel


#include <Arduino.h>
#include "BluetoothA2DPSource.h"  
#include <Wire.h>                  
#include <math.h>                  
#include <Adafruit_NeoPixel.h>  
#include "functionGenerator.h"   

// Definition der Pins fuer UART2
#define RXD2 16   
#define TXD2 17   

#define POT_PIN 34  // Potentiometer an GPIO34

// Chain-Kabel 
#define PIN_CHAIN_START 4

// BPM-Grenzen fuer Systemsicherheit
#define MAX_BPM 240  // Maximale BPM-Grenze, um UI-Lag oder Abstuerze zu verhindern
#define MIN_BPM 70   // Minimale BPM-Grenze
#define STD_BPM 120  // Standard BPM


// NeoPixel 
// NEO_COUNT = maximale Anzahl LEDs 
#define NEO_PIN   5
#define NEO_COUNT 36
Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Farben NeoPIXEL
#define COLOR_STRAIGHT strip.Color( 80,   0,  0)  // Rot
#define COLOR_LEFT     strip.Color( 80,   0,  0)  // Rot
#define COLOR_RIGHT    strip.Color( 80,   0,  0)  // Rot
#define COLOR_PLAYHEAD strip.Color(  0, 255,  0)  // Grün = Auto
#define COLOR_OFF      strip.Color(  0,   0,  0)  // Aus

// Datenstruktur (Track-Array und Playhead)
enum BlockType { STRAIGHT, LEFT, RIGHT, EMPTY };
const int MAX_TRACK = 16; // Maximale Track-Laenge
BlockType track[MAX_TRACK]; // Array zum Speichern der Track-Bloecke
uint8_t   kick_pattern[MAX_TRACK]; 
uint8_t   snare_pattern[MAX_TRACK]; 
uint8_t   module_addresses[MAX_TRACK];
int track_length = 0;   // Aktuelle Laenge des erstellten Tracks
int playhead     = 0;   // Aktuelle Wiedergabeposition (Playhead)
int active_playhead = 0;  // Modul, dessen Takt gerade läuft

// Fail-Safe
int module_miss_count = 0;
#define MAX_MODULE_MISS 10   // Nach 2 Fehlern → Auto-Stop

//  Musik-Engine: Quintenzirkel (Circle of Fifths)
// Lookup-Table fuer die 12 Noten. Einfacher und schneller als mathematische Berechnungen.
const char* circleOfFifths[12] = {
    "C","G","D","A","E","B","F#","C#","G#","D#","A#","F"
};
int start_note_index   = 0;   // 
int current_note_index = start_note_index;

// Frequenzen (Hz) fuer die 12 Noten des Quintenzirkels (Basis: C4 bis B4)
const float note_frequencies[12] = {
    261.63, // C
    392.00, // G
    293.66, // D
    440.00, // A
    329.63, // E
    493.88, // B
    369.99, // F#
    277.18, // C#
    415.30, // G#
    311.13, // D#
    466.16, // A#
    349.23  // F
};

//  Kick und SNare-Frequenz 
#define KICK_FREQUENCY   180.0f
#define SNARE_FREQUENCY  320.0f

#define SAMPLE_RATE      44100.0f
#define MELODY_LEVEL     2300.0f

#define KICK_LEVEL       6500.0f
#define SNARE_LEVEL      5200.0f

#define MASTER_LIMIT     30000

// Systemzustand 
struct SystemState {
    int bpm;            
    const char* note;   
    const char* status; 
};
SystemState state;

unsigned long last_send_time = 0;  

// Nicht-blockierender UART-Empfang vom PyBadge
String pybadge_command_buffer;

// Playback-Engine: Timer-Variable fuer Beats
unsigned long last_beat_time = 0;  

// Bluetooth und Audio-Synthesizer Variablen
BluetoothA2DPSource a2dp_source;

// FunctionGenerator Instanzen (eine pro Ton)
// Amplitude wird spaeter per setAmplitude() gesetzt
funcgen melody_osc;
funcgen kick_osc;
funcgen snare_osc;  // Snare

//Meldodien-tom
volatile float current_frequency = 0; // 0 bedeutet Stille
volatile unsigned long sound_duration = 0;
volatile unsigned long sound_start_time = 0;
 

// Kick-Ton
volatile float kick_current_frequency = 0; 
volatile uint32_t kick_start_time = 0; 
volatile uint32_t kick_duration = 0; 
volatile uint32_t kick_trigger_id = 0; 

// Snarea-Ton
volatile float snare_current_frequency = 0;
volatile unsigned long snare_start_time = 0; 
volatile unsigned long snare_duration = 0; 
volatile uint32_t snare_trigger_id = 0;

// Sub-Beat System: 4 Viertel pro Beat
// Bit 0 = Viertel 1, Bit 1 = Viertel 2, Bit 2 = Viertel 3, Bit 3 = Viertel 4
volatile uint32_t subbeat_interval = 0;   
volatile uint32_t last_subbeat_ms  = 0;  
volatile uint8_t  kick_bits_active = 0;   
volatile uint8_t  snare_bits_active = 0; 
volatile uint8_t  current_subbeat  = 0;  

// Callback-Funktion fuer das Bluetooth-Audio-Streaming

static float envelope(float elapsed_ms, float duration_ms, float attack_ms, float release_ms) {
    if (duration_ms <= 0.0f || elapsed_ms < 0.0f || elapsed_ms >= duration_ms) return 0.0f;
    float attack = (attack_ms > 0.0f && elapsed_ms < attack_ms)
        ? elapsed_ms / attack_ms : 1.0f;
    float remaining = duration_ms - elapsed_ms;
    float release = (release_ms > 0.0f && remaining < release_ms)
        ? remaining / release_ms : 1.0f;
    return constrain(attack * release, 0.0f, 1.0f);
}

static int16_t clampSample(float value) {
    if (value > MASTER_LIMIT)  return  MASTER_LIMIT;
    if (value < -MASTER_LIMIT) return -MASTER_LIMIT;
    return (int16_t)value;
}
//  Library gibt Werte zwischen -amplitude und +amplitude zurueck
// Wir benutzen t = Zeit in Sekunden seit Ton-Start
// Attack: erste 5ms natuerlich einblenden (kein harter Start)
// Decay fuer Note: letzte 20% des Takts ausblenden → klingt harmonischer
// Decay fuer Kick/Snare: linear von 1 auf 0 → perkussiv

int32_t get_audio_data(Frame *channels, int32_t channel_len) {
    // Kontinuierliche Phasen - niemals zurücksetzen!
    static uint32_t melody_samples = 0;
    static uint32_t kick_samples   = 0;
    static uint32_t snare_samples  = 0;

    static float last_melody_frequency = 0.0f;
    static float last_kick_frequency   = 0.0f;
    static float last_snare_frequency  = 0.0f;

    static uint32_t seen_kick_trigger_id = 0;
     static uint32_t seen_snare_trigger_id = 0;

    const uint32_t now_ms = millis();
//Callback sollte die Werte zu Beginn lokal kopieren.

    const uint32_t local_kick_trigger_id = kick_trigger_id;
const uint32_t local_snare_trigger_id = snare_trigger_id;

const uint32_t local_kick_start_time = kick_start_time;
const uint32_t local_snare_start_time = snare_start_time;

const uint32_t local_kick_duration = kick_duration;
const uint32_t local_snare_duration = snare_duration;

const float local_kick_frequency = kick_current_frequency;
const float local_snare_frequency = snare_current_frequency;

    for (int i = 0; i < channel_len; i++) {
        float sample = 0.0f;

        // Melodie
        if (current_frequency > 0 && now_ms - sound_start_time < sound_duration) {
            if (current_frequency != last_melody_frequency) {
                melody_osc.setFrequency(current_frequency);
                melody_samples = 0;
                last_melody_frequency = current_frequency;
            }
            const float elapsed_ms = (now_ms - sound_start_time) + (i * 1000.0f / SAMPLE_RATE);
            const float env = envelope(elapsed_ms, sound_duration, 18.0f, min(140.0f, sound_duration * 0.22f));
            const float t = melody_samples++ / SAMPLE_RATE;
            sample += melody_osc.sawtooth(t) * MELODY_LEVEL * env;
        } 
        else {
            melody_samples = 0;
            last_melody_frequency = 0.0f;
}

        // Kick
        if (
    local_kick_frequency > 0.0f &&
    now_ms - local_kick_start_time < local_kick_duration
) {
    if (
        local_kick_trigger_id != seen_kick_trigger_id ||
        local_kick_frequency != last_kick_frequency
    ) {
        seen_kick_trigger_id = local_kick_trigger_id;

        kick_osc.setFrequency(local_kick_frequency);

        kick_samples = 0;
        last_kick_frequency = local_kick_frequency;
    }

    const float elapsed_ms =
        (now_ms - local_kick_start_time) +
        (i * 1000.0f / SAMPLE_RATE);

    float env = envelope(
        elapsed_ms,
        local_kick_duration,
        3.0f,
        local_kick_duration * 0.92f
    );

    env *= env;

    const float t =
        kick_samples++ / SAMPLE_RATE;

    sample +=
        kick_osc.sawtooth(t) *
        KICK_LEVEL *
        env;
}
else {
    kick_samples = 0;
    last_kick_frequency = 0.0f;
}

        // Snare
      if (
    local_snare_frequency > 0.0f &&
    now_ms - local_snare_start_time < local_snare_duration
) {
    if (
        local_snare_trigger_id != seen_snare_trigger_id ||
        local_snare_frequency != last_snare_frequency
    ) {
        seen_snare_trigger_id = local_snare_trigger_id;

        snare_osc.setFrequency(local_snare_frequency);

        snare_samples = 0;
        last_snare_frequency = local_snare_frequency;
    }

    const float elapsed_ms =
        (now_ms - local_snare_start_time) +
        (i * 1000.0f / SAMPLE_RATE);

    float env = envelope(
        elapsed_ms,
        local_snare_duration,
        2.5f,
        local_snare_duration * 0.88f
    );

    env *= env;

    const float t =
        snare_samples++ / SAMPLE_RATE;

    sample +=
        snare_osc.sawtooth(t) *
        SNARE_LEVEL *
        env;
}
else {
    snare_samples = 0;
    last_snare_frequency = 0.0f;
}

        const int16_t output = clampSample(sample);
        channels[i].channel1 = output;
        channels[i].channel2 = output;
    }
    return channel_len;
}

//updateNeoPixelTrack
void updateNeoPixelTrack() {
    for (int i = 0; i < NEO_COUNT; i++) {
        int module = i / 2;  // Welches Modul gehört diese LED?
        
        if (module >= track_length) {
            strip.setPixelColor(i, COLOR_OFF);
            continue;
        }
        if (module == playhead) {
            strip.setPixelColor(i, COLOR_PLAYHEAD);  
        } else {
            switch (track[module]) {
                case LEFT:     strip.setPixelColor(i, COLOR_LEFT);     break;
                case RIGHT:    strip.setPixelColor(i, COLOR_RIGHT);    break;
                case STRAIGHT: strip.setPixelColor(i, COLOR_STRAIGHT); break;
                default:       strip.setPixelColor(i, COLOR_OFF);      break;
            }
        }
    }
    strip.show();
}
// Kick und Snare  lesen
// Gibt true zurueck wenn Modul erreichbar, false wenn nicht (Watchdog!)
bool readKickSnareLive(int position) {
    if (position >= track_length) return true;
    uint8_t addr = module_addresses[position];
    if (addr == 0) return true;

    // 4 Bytes vom ATtiny lesen: direction, kick, snare, chain_active
 Wire.requestFrom(addr, (uint8_t)4);
    
    if (Wire.available() >= 4) {
        Wire.read();                           // direction
        kick_pattern[position] = Wire.read();  // kick
        snare_pattern[position] = Wire.read(); // snare
        Wire.read();                           // chain_active
        return true;
    }
    return false;
}

// Schickt RESET-Befehl (0x00) an alle moeglichen Adressen
// Danach sind alle Module bereit fuer einen neuen Scan
void resetAllModules() {
    for (uint8_t addr = 0x10; addr <= 0x1F; addr++) {
        Wire.beginTransmission(addr);
        Wire.write(0x00);  // 0x00 = RESET
        Wire.endTransmission();
    }
    delay(20);  // Kurz warten bis alle Module zurueckgesetzt sind
}


// [NEOPIXEL] Lila Blinken (Verbindungs-Check)

void blinkPurple() {
    Serial.println("[SYSTEM] PyBadge verbunden! LED Bestaetigung (Lila blinken)...");
    
    // 3 Mal lila blinken
    for (int b = 0; b < 3; b++) { 
        for (int i = 0; i < NEO_COUNT; i++) {
            int module = i / 2;
            if (module < track_length) {
                // Lila (Purple): RGB 값 (128, 0, 128)
                strip.setPixelColor(i, strip.Color(128, 0, 128)); 
            } else {
                strip.setPixelColor(i, COLOR_OFF);
            }
        }
        strip.show();
        delay(250); // 250ms an

        for (int i = 0; i < NEO_COUNT; i++) {
            strip.setPixelColor(i, COLOR_OFF);
        }
        strip.show();
        delay(250); // 250ms aus
    }
    // Danach wieder normale Track-Farben anzeigen
    updateNeoPixelTrack();
}



//  scanAndBuildTrack() 

void scanAndBuildTrack() {
    // Alles zuruecksetzen
    track_length       = 0;
    playhead           = 0;
    active_playhead    = 0;
    current_note_index = start_note_index;
    module_miss_count  = 0;
    memset(module_addresses, 0, sizeof(module_addresses));

    Serial.println("[CHAIN] Starte Scan...");
    resetAllModules();

    digitalWrite(PIN_CHAIN_START, HIGH);  // Chain-Signal starten
    delay(200);

    for (int pos = 0; pos < MAX_TRACK; pos++) {
        bool found = false;

        // Alle 16 Adressen abfragen: wer hat chain_active=1?
        for (uint8_t addr = 0x10; addr <= 0x1F; addr++) {
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() != 0) continue;  // Kein Modul da

            Wire.requestFrom(addr, (uint8_t)4);
            if (Wire.available() < 4) continue;

            uint8_t dir          = Wire.read();   // 0=STRAIGHT 1=LEFT 2=RIGHT
            uint8_t kick         = Wire.read();   // Kick-Bitmuster
            uint8_t snare        = Wire.read();   //snare
            uint8_t chain_active = Wire.read();   // 1 = dieses Modul ist dran

            if (chain_active != 1) continue; 

            // Dieses Modul ist Position pos: speichern
            if      (dir == 1) track[track_length] = LEFT;
            else if (dir == 2) track[track_length] = RIGHT;
            else               track[track_length] = STRAIGHT;

            kick_pattern[track_length]     = kick;
            snare_pattern[track_length]    = snare;
            module_addresses[track_length] = addr;  // Adresse merken 

            const char* d = (dir==1)?"LEFT":(dir==2)?"RIGHT":"STRAIGHT";
            Serial.printf("[CHAIN] Pos %d: 0x%02X | %s | kick=%d\n snare=%d\n",
                pos, addr, d, kick, snare);

            track_length++;
            found = true;

            // NEXT senden: leite Chain weiter
            Wire.beginTransmission(addr);
            Wire.write(0x01);
            Wire.endTransmission();
            delay(50);
            break;
        }

        if (!found) {
            //  Track endet hier
            Serial.printf("[CHAIN] Pos %d: kein Modul – Track endet.\n", pos);
            break;
        }
    }

    digitalWrite(PIN_CHAIN_START, LOW);  // Chain-Signal beenden
    Serial.printf("[CHAIN] Fertig: %d Module erkannt.\n", track_length);
    updateNeoPixelTrack();  // LEDs sofort aktualisieren
}

// 

// Liest genau eine komplette PyBadge-Zeile, ohne den Audio-/Beat-Loop zu blockieren.
bool readPyBadgeCommand(String &command) {
    while (Serial2.available() > 0) {
        char c = (char)Serial2.read();

        if (c == '\n') {
            command = pybadge_command_buffer;
            pybadge_command_buffer = "";
            command.trim();
            return command.length() > 0;
        }

        if (c != '\r') {
            if (pybadge_command_buffer.length() < 64) {
                pybadge_command_buffer += c;
            } else {
                // Defekte/zu lange Zeile verwerfen.
                pybadge_command_buffer = "";
            }
        }
    }

    return false;
}


void setup() {
    // Initialisierung der seriellen USB-Verbindung fuer den PC-Monitor
    Serial.begin(115200);
    delay(1000);

    // Initialisierung von Hardware-UART2 fuer die Kommunikation mit dem PyBadge
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("ESP32 UART Steuerung Bereit...");


    // I2C als Master starten (SDA=GPIO21, SCL=GPIO22)
    Wire.begin();

    // Chain-Pin konfigurieren
    pinMode(PIN_CHAIN_START, OUTPUT);
    digitalWrite(PIN_CHAIN_START, LOW);

    // NeoPixel initialisieren und alle LEDs ausschalten
    strip.begin();
    strip.setBrightness(60);  // Helligkeit 0-255 (60 = angenehm)
    strip.show();

   // Start-Variablen initialisieren
    state.bpm = STD_BPM;
    state.note = circleOfFifths[current_note_index]; // Startnote "C"
    state.status = "STOPPED";


   // Bluetooth A2DP Source starten (Hier den Namen des Lautsprechers eintragen)
    Serial.println("[SYSTEM] Verbinde mit Bluetooth-Lautsprecher...");
    a2dp_source.start("HUAWEI CM510", get_audio_data);


    // Kurz warten damit alle Module hochgefahren sind
    delay(1000);

    // Direkt beim Start nach Modulen scannen
    scanAndBuildTrack();
}


void loop() {
    unsigned long current_time = millis();

     // Potentiometer BPM lesen
    int pot_value = analogRead(POT_PIN);
    int new_bpm   = map(pot_value, 0, 4095, MIN_BPM, MAX_BPM);
    
    // Nur aktualisieren wenn Änderung > 5 BPM (verhindert Zittern)
    if (abs(new_bpm - state.bpm) > 4) {
        state.bpm = new_bpm;
        Serial.printf("[POT] BPM: %d\n", state.bpm);
    }

      // Serielle Eingabe
    if (Serial.available()) {
        char c = Serial.read();
        c = toupper(c);  // Kleinbuchstaben in Grossbuchstaben umwandeln

        if (c == 'X') {
            track_length = 0;
             playhead = 0;
             // Note auch auf "C" zuruecksetzen
             active_playhead = 0;
            start_note_index = 0; 
            current_note_index = start_note_index;
            state.bpm = STD_BPM;
            state.note = circleOfFifths[current_note_index];
             // Status auf STOPPED setzen und PyBadge ins Menue schicken
            state.status = "STOPPED";
            module_miss_count = 0;
            Serial2.println("CMD_MENU");  
            updateNeoPixelTrack();

            Serial.println("[SYSTEM] Track wurde zurueckgesetzt. Gehe zum Menue.");
        }
        else if (c == 'I') {
            // Neuer Chain-Scan (Module neu einlesen)
            scanAndBuildTrack();
        }
        // MANUELLER FALLBACK 
        else if ((c == 'L' || c == 'R' || c == 'S') && track_length < MAX_TRACK) {
            if (c == 'L') track[track_length] = LEFT;
            else if (c == 'R') track[track_length] = RIGHT;
            else               track[track_length] = STRAIGHT;
            kick_pattern[track_length]     = 0;  
             snare_pattern[track_length]    = 0;
            module_addresses[track_length] = 0;  
            track_length++;
             Serial.printf("[TRACK] Block hinzugefuegt: %c (Aktuelle Gesamtlaenge: %d)\n", c, track_length);
            updateNeoPixelTrack();
        }
        else if (c == 'P') {
            // START (manuell, auch ohne PyBadge)
            state.status = "RUNNING";
            unsigned long now = millis();
             // Ersten Takt sofort auslösen
            unsigned long beat_interval = 60000UL / state.bpm;
                last_beat_time  = now - beat_interval;
                last_subbeat_ms = now;

                current_subbeat  = 0;
                kick_bits_active = 0;
                snare_bits_active = 0;
                module_miss_count = 0;

                Serial.println("[SYSTEM] START (manuell)");
        }

        else if (c == 'O') {
            // STOP (manuell, auch ohne PyBadge)
            state.status = "STOPPED";
            Serial2.println("CMD_MENU");
            Serial.println("[SYSTEM] STOP (manuell)");
        }
       
        else if (c == 'K') {
    // Kick manuell pro Modul setzen
    // Format: K1=5 -> Modul 1 bekommt Kick-Muster 5

    delay(100);

    String input = "";

    while (Serial.available()) {
        input += (char)Serial.read();
        delay(10);
    }

    input.trim();

    int sep = input.indexOf('=');

    if (sep > 0) {
        int modul = input.substring(0, sep).toInt() - 1;
        int wert = input.substring(sep + 1).toInt();

        wert = constrain(wert, 0, 15);

        if (modul >= 0 && modul < track_length) {
            kick_pattern[modul] = wert;

            Serial.printf(
                "[KICK] Modul %d -> Muster %d (0b%d%d%d%d)\n",
                modul + 1,
                wert,
                (wert >> 3) & 1,
                (wert >> 2) & 1,
                (wert >> 1) & 1,
                wert & 1
            );
        }
        else {
            Serial.printf(
                "[KICK] Fehler: Modul %d existiert nicht!\n",
                modul + 1
            );
        }
    }
    else {
        Serial.println("[KICK] Format: K1=5 (Modul=Muster)");
    }
}
        // Kick manuell pro Modul setzen
        else if (c == 'N') {
    // Snare manuell pro Modul setzen
    // Format: N1=5 -> Modul 1 bekommt Snare-Muster 5

    delay(100);

    String input = "";

    while (Serial.available()) {
        input += (char)Serial.read();
        delay(10);
    }

    input.trim();

    int sep = input.indexOf('=');

    if (sep > 0) {
        int modul = input.substring(0, sep).toInt() - 1;
        int wert = input.substring(sep + 1).toInt();

        wert = constrain(wert, 0, 15);

        if (modul >= 0 && modul < track_length) {
            snare_pattern[modul] = wert;

            Serial.printf(
                "[SNARE] Modul %d -> Muster %d (0b%d%d%d%d)\n",
                modul + 1,
                wert,
                (wert >> 3) & 1,
                (wert >> 2) & 1,
                (wert >> 1) & 1,
                wert & 1
            );
        }
        else {
            Serial.printf(
                "[SNARE] Fehler: Modul %d existiert nicht!\n",
                modul + 1
            );
        }
    }
    else {
        Serial.println("[SNARE] Format: N1=5 (Modul=Muster)");
    }
}

    // Playback-Engine (BPM-Timing mit millis)
    // Laeuft nur, wenn der Status "RUNNING" ist und der Track Bloecke enthaelt
    // Zuerst wird faelliger neuer Beat gestartet. Danach werden Viertel 2 bis 4 verarbeitet. Dadurch werden keine Subbeats des vorherigen Takts  versehentlich im neuen Takt ausgeloest.

    if (String(state.status) == "RUNNING" && track_length > 0) {

    unsigned long beat_interval = 60000UL / state.bpm;
    subbeat_interval = beat_interval / 4;


   
    // Neuen Beat beziehungsweise neuen Takt starten
    

    while (current_time - last_beat_time >= beat_interval) {

        // Soll-Zeit weiterschieben, damit kein Timing-Drift entsteht
        last_beat_time += beat_interval;

        // Viertel-Raster des neuen Takts starten
        last_subbeat_ms = last_beat_time;
        current_subbeat = 0;

        // Dieses Modul ist fuer alle vier Viertel aktiv
        active_playhead = playhead;

        // Kick- und Snare-Pattern einmal am Taktanfang lesen
        readKickSnareLive(active_playhead);

        kick_bits_active =
            kick_pattern[active_playhead];

        snare_bits_active =
            snare_pattern[active_playhead];

        // Richtung des aktuellen Moduls auswerten
        BlockType current_block =
            track[active_playhead];

        if (current_block == RIGHT) {
            current_note_index =
                (current_note_index + 1) % 12;
        }
        else if (current_block == LEFT) {
            current_note_index =
                (current_note_index - 1 + 12) % 12;
        }

        state.note =
            circleOfFifths[current_note_index];

        // Tatsaechlicher Zeitpunkt, an dem das Audio startet
        const unsigned long trigger_time = millis();

        // Melodie starten
        current_frequency =
            note_frequencies[current_note_index];

        sound_start_time = trigger_time;
        sound_duration = beat_interval + 12UL;

        Serial.printf(
            "[BEAT] Pos:%d/%d | %s | Note:%s "
            "| kick=0b%d%d%d%d | snare=0b%d%d%d%d\n",

            active_playhead + 1,
            track_length,

            (
                current_block == STRAIGHT
                    ? "STRAIGHT"
                    : current_block == LEFT
                        ? "LEFT"
                        : "RIGHT"
            ),

            state.note,

            (kick_bits_active >> 3) & 1,
            (kick_bits_active >> 2) & 1,
            (kick_bits_active >> 1) & 1,
            kick_bits_active & 1,

            (snare_bits_active >> 3) & 1,
            (snare_bits_active >> 2) & 1,
            (snare_bits_active >> 1) & 1,
            snare_bits_active & 1
        );

      
        // Viertel 1: Kick
        

        if (kick_bits_active & 0x01) {
            kick_current_frequency =
                KICK_FREQUENCY;

            kick_start_time =
                trigger_time;

            kick_duration = max(
                35UL,
                subbeat_interval * 45UL / 100UL
            );

            kick_trigger_id++;

            Serial.printf(
                "[KICK] Pos:%d Viertel:1\n",
                active_playhead + 1
            );
        }

        
        // Viertel 1: Snare
        

        if (snare_bits_active & 0x01) {
            snare_current_frequency =
                SNARE_FREQUENCY;

            snare_start_time =
                trigger_time;

            snare_duration = max(
                45UL,
                subbeat_interval * 55UL / 100UL
            );

            snare_trigger_id++;

            Serial.printf(
                "[SNARE] Pos:%d Viertel:1\n",
                active_playhead + 1
            );
        }

        updateNeoPixelTrack();

        // Erst jetzt zum naechsten Modul wechseln
        playhead++;

        if (playhead >= track_length) {
            playhead = 0;
            current_note_index =
                start_note_index;
        }
    }

    
    //  Viertel 2 bis 4 verarbeiten
   

    while (
        subbeat_interval > 0 &&
        current_subbeat < 3 &&
        current_time - last_subbeat_ms >= subbeat_interval
    ) {
        // Soll-Zeit weiterschieben, damit das Raster stabil bleibt
        last_subbeat_ms += subbeat_interval;

        current_subbeat++;

        //  den echten Startzeitpunkt verwenden
        const unsigned long trigger_time = millis();

        
        // Kick auf dem aktuellen Viertel
        

        if (
            kick_bits_active &
            (1U << current_subbeat)
        ) {
            kick_current_frequency =
                KICK_FREQUENCY;

            kick_start_time =
                trigger_time;

            kick_duration = max(
                35UL,
                subbeat_interval * 45UL / 100UL
            );

            kick_trigger_id++;

            Serial.printf(
                "[KICK] Pos:%d Viertel:%d\n",
                active_playhead + 1,
                current_subbeat + 1
            );
        }

       
        // Snare auf dem aktuellen Viertel
        

        if (
            snare_bits_active &
            (1U << current_subbeat)
        ) {
            snare_current_frequency =
                SNARE_FREQUENCY;

            snare_start_time =
                trigger_time;

            snare_duration = max(
                45UL,
                subbeat_interval * 55UL / 100UL
            );

            snare_trigger_id++;

            Serial.printf(
                "[SNARE] Pos:%d Viertel:%d\n",
                active_playhead + 1,
                current_subbeat + 1
            );
        }
    }
}

 //  Befehle vom PyBadge empfangen (UART Read) 
    String command;

if (readPyBadgeCommand(command)) {
    Serial.print("Befehl: ");
    Serial.println(command);

    Serial.print("[UART] Befehl von PyBadge: ");
    Serial.println(command);

        if (command == "INIT_OK") {
            // PyBadge hat sich erfolgreich verbunden!
            blinkPurple();
        }
        else if (command == "CHECK_START") {
            // Check: Sind physikalische Track-Module verbunden?
            if (track_length > 0) {
                // Track vorhanden! Erlaubnis fuer F1-Animation an PyBadge senden (Musik wartet noch)
                Serial2.println("CMD_START_OK");
                Serial.println("[SYSTEM] CHECK_START OK. Warte auf F1-Animation...");
            } else {
                // Kein Track! Start ablehnen und Fehler-Screen triggern
                Serial2.println("CMD_NO_TRACK");
                Serial.println("[ERROR] START abgelehnt: Keine Module am Chain erkannt!");
            }
        }
        else if (command == "START_MUSIC") {
            // F1-Animation des PyBadge ist beendet! Echtes Racing (Musik) startet jetzt!
            state.status   = "RUNNING";
                unsigned long now = millis();
             // beat_interval muss innerhalb dieses Blocks neu berechnet werden.
            unsigned long beat_interval =
                60000UL / (unsigned long)state.bpm;

            // Ersten Beat sofort starten
            last_beat_time    = now - beat_interval;
            last_subbeat_ms   = now;

            current_subbeat   = 0;

            kick_bits_active  = 0;
            snare_bits_active = 0;


            // Eventuell noch aktive Percussion stoppen 
            kick_current_frequency = 0;
            snare_current_frequency = 0;

            module_miss_count = 0;      
            Serial.println("[SYSTEM] START_MUSIC -> Rennerlaubnis erteilt. Go!");
        }
        else if (command == "STOP") {
            state.status = "STOPPED";
            Serial2.println("CMD_MENU");  
            Serial.println("[SYSTEM] STOP -> Zurueck ins Menue.");
        }
        else if (command == "NOTE_UP") {
            if (String(state.status) == "STOPPED") {
                start_note_index   = (start_note_index + 1) % 12;
                current_note_index = start_note_index;
                state.note         = circleOfFifths[current_note_index];
                Serial.printf("[SYSTEM] Startnote geaendert auf: %s\n", state.note);
            }
        }
        else if (command == "NOTE_DOWN") {
            if (String(state.status) == "STOPPED") {
                start_note_index   = (start_note_index - 1 + 12) % 12;
                current_note_index = start_note_index;
                state.note         = circleOfFifths[current_note_index];
                Serial.printf("[SYSTEM] Startnote geaendert auf: %s\n", state.note);
            }
        }
    } 

    // --- Sende aktuelle Displaydaten an PyBadge ---
    
    // Interval von 500ms auf 150ms verkuerzt, damit die UI flüssig mit den Beats updatet
    if (current_time - last_send_time > 150) {
        last_send_time = current_time;
        // Format: "Note,BPM,Status\n" -> Wird vom PyBadge gesplittet
        Serial2.printf("%s,%d,%s\n", state.note, state.bpm, state.status);
        }
    
}
}
    
    // Sende aktuelle Displaydaten an PyBadge
    // Interval von 500ms auf 150ms verkuerzt, damit die UI flüssig mit den Beats updatet

         // Debug-Ausgabe auf dem PC-Serienmonitor (auskommentiert um Spam zu vermeiden, bei Bedarf aktivieren)
        // Serial.printf("SEND -> %s,%d,%s\n", state.note, state.bpm, state.status);

    
    


