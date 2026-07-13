
// Bluetooth + ATtiny I2C + Chain + NeoPixel


#include <Arduino.h>
#include "BluetoothA2DPSource.h"  
#include <Wire.h>                  
#include <math.h>                  
#include <Adafruit_NeoPixel.h>    

// Definition der Pins fuer UART2
#define RXD2 16   
#define TXD2 17   

// Chain-Kabel 
#define PIN_CHAIN_START 4

// BPM-Grenzen fuer Systemsicherheit
#define MAX_BPM 240  // Maximale BPM-Grenze, um UI-Lag oder Abstuerze zu verhindern
#define MIN_BPM 60   // Minimale BPM-Grenze
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
const int MAX_TRACK = 18; // Maximale Track-Laenge
BlockType track[MAX_TRACK]; // Array zum Speichern der Track-Bloecke
uint8_t   kick_pattern[MAX_TRACK]; 
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

//  Kick-Frequenz 
#define KICK_FREQUENCY 220.0f

// Systemzustand 
struct SystemState {
    int bpm;            
    const char* note;   
    const char* status; 
};
SystemState state;

unsigned long last_send_time = 0;  

// Playback-Engine: Timer-Variable fuer Beats
unsigned long last_beat_time = 0;  

// Bluetooth und Audio-Synthesizer Variablen
BluetoothA2DPSource a2dp_source;

volatile float current_frequency = 0; // 0 bedeutet Stille
volatile unsigned long sound_duration = 0;
volatile unsigned long sound_start_time = 0;
 

// Kick-Ton
volatile float    kick_current_frequency   = 0;      // 0 bedeutet Stille
volatile uint32_t kick_start_time   = 0;      
volatile uint32_t kick_duration    = 0;      

// Sub-Beat System: 4 Viertel pro Beat
// Bit 0 = Viertel 1, Bit 1 = Viertel 2, Bit 2 = Viertel 3, Bit 3 = Viertel 4
volatile uint32_t subbeat_interval = 0;   
volatile uint32_t last_subbeat_ms  = 0;  
volatile uint8_t  kick_bits_active = 0;   
volatile uint8_t  current_subbeat  = 0;  

// Callback-Funktion fuer das Bluetooth-Audio-Streaming
int32_t get_audio_data(Frame *channels, int32_t channel_len) {
    static float sample_index = 0;
    static float kick_index   = 0;
    
    for (int i = 0; i < channel_len; i++) {
        int16_t sample = 0;

        // Melodie-Ton
        if (current_frequency > 0 && (millis() - sound_start_time < sound_duration)) {
            sample += (int16_t)(sin(sample_index) * 1000);
            sample_index += (2 * M_PI * current_frequency) / 44100.0;
            if (sample_index >= 2 * M_PI) sample_index -= 2 * M_PI;
        } else {
            sample_index = 0;
            if (current_frequency > 0) current_frequency = 0;
        }

        // Kick-Ton
        if (kick_current_frequency > 0 && (millis() - kick_start_time < kick_duration)) {
            float progress = (float)(millis() - kick_start_time) / (float)kick_duration;
            float decay    = 1.0f - progress;
            sample += (int16_t)(sin(kick_index) * 1500 * decay);
            kick_index += (2.0f * M_PI * kick_current_frequency) / 44100.0f;
            if (kick_index >= 2.0f * M_PI) kick_index -= 2.0f * M_PI;
        } else {
            kick_index = 0;
            if (kick_current_frequency > 0) kick_current_frequency = 0;
        }

        channels[i].channel1 = sample;
        channels[i].channel2 = sample;
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
// readKickLive()
// Gibt true zurueck wenn Modul erreichbar, false wenn nicht (Watchdog!)
bool readKickLive(int position) {
    if (position >= track_length) return true;
    uint8_t addr = module_addresses[position];
    if (addr == 0) return true;

    // 4 Bytes vom ATtiny lesen: direction, kick, snare, chain_active
 Wire.requestFrom(addr, (uint8_t)4);
    delay(20);
    if (Wire.available() >= 4) {
        Wire.read();                           // direction
        kick_pattern[position] = Wire.read();  // kick
        Wire.read();                           // snare
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

//  scanAndBuildTrack() 

void scanAndBuildTrack() {
    // Alles zuruecksetzen
    track_length       = 0;
    playhead           = 0;
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
            Wire.read();                           // snare ueberspringen
            uint8_t chain_active = Wire.read();   // 1 = dieses Modul ist dran

            if (chain_active != 1) continue; 

            // Dieses Modul ist Position pos: speichern
            if      (dir == 1) track[track_length] = LEFT;
            else if (dir == 2) track[track_length] = RIGHT;
            else               track[track_length] = STRAIGHT;

            kick_pattern[track_length]     = kick;
            module_addresses[track_length] = addr;  // Adresse merken 

            const char* d = (dir==1)?"LEFT":(dir==2)?"RIGHT":"STRAIGHT";
            Serial.printf("[CHAIN] Pos %d: 0x%02X | %s | kick=%d\n",
                pos, addr, d, kick);

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

      // Serielle Eingabe
    if (Serial.available()) {
        char c = Serial.read();
        c = toupper(c);  // Kleinbuchstaben in Grossbuchstaben umwandeln

        if (c == 'X') {
            track_length = 0;
             playhead = 0;
             // Note auch auf "C" zuruecksetzen
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
            // Format: K1=5 → Modul 1 bekommt Kick-Muster 5
            delay(100);
            String input = "";
            while (Serial.available()) {
                input += (char)Serial.read();
                delay(10);
            }
            input.trim();
            
            // Input splitten: "1=5" → Modul 1, Wert 5
            int sep = input.indexOf('=');
            if (sep > 0) {
                int modul = input.substring(0, sep).toInt() - 1; // 0-basiert
                int wert  = input.substring(sep + 1).toInt();
                wert      = constrain(wert, 0, 15);
                
                if (modul >= 0 && modul < track_length) {
                    kick_pattern[modul] = wert;
                    Serial.printf("[KICK] Modul %d → Muster %d (0b%d%d%d%d)\n",
                        modul + 1, wert,
                        (wert>>3)&1, (wert>>2)&1,
                        (wert>>1)&1, wert&1);
                } else {
                    Serial.printf("[KICK] Fehler: Modul %d existiert nicht!\n", modul + 1);
                }
            } else {
                Serial.println("[KICK] Format: K1=5 (Modul=Muster)");
            }
        }
    }

    // Playback-Engine (BPM-Timing mit millis)
    // Laeuft nur, wenn der Status "RUNNING" ist und der Track Bloecke enthaelt

    if (String(state.status) == "RUNNING" && track_length > 0) {

        // Berechne Millisekunden pro Schlag (Interval = 60000 / BPM)
        unsigned long beat_interval = 60000 / state.bpm;
        subbeat_interval = beat_interval / 4;  // Ein Viertel = Beat/4

//zuerts subbeat?
            while (
        subbeat_interval > 0 &&
        current_subbeat < 3 &&
        current_time - last_subbeat_ms >= subbeat_interval) {
        // Nicht current_time benutzen, damit kein Timing-Drift entsteht
        last_subbeat_ms += subbeat_interval;
        current_subbeat++;

                if (kick_bits_active & (1 << current_subbeat)) {
                    kick_current_frequency = KICK_FREQUENCY;
                    kick_start_time        = current_time;
                    kick_duration          = subbeat_interval * 0.2;
                   Serial.printf("[KICK] Pos:%d Viertel:%d\n", active_playhead + 1, current_subbeat + 1);
                }     
        }
        
        if (current_time - last_beat_time >= beat_interval) {
            last_beat_time += beat_interval; //timing bleibt exact
            last_subbeat_ms = last_beat_time; //beginn des neuen taktes
            current_subbeat = 0;  // Erstes Viertel

            // Kick live lesen mit Watchdog gerade kein watchsog für test
            // Aktuelles Modul lesen, bevor der Playhead weitergeht
            
            // Dieses Modul bleibt für alle vier Viertel aktiv
            active_playhead = playhead;

            readKickLive(active_playhead);
            kick_bits_active = kick_pattern[active_playhead];

            // Notenberechnung basierend auf Richtung
            BlockType current_block = track[active_playhead];
            if      (current_block == RIGHT) {current_note_index = (current_note_index + 1) % 12;}
            else if (current_block == LEFT)  {current_note_index = (current_note_index - 1 + 12) % 12;}
            // STRAIGHT: Note unveraendert

            state.note = circleOfFifths[current_note_index];

            // Melodie-Ton starten (90% des ganzen Beats)
            // Die Note klingt durch alle 4 Viertel hindurch
            current_frequency  = note_frequencies[current_note_index];
            sound_start_time   = current_time;
            sound_duration     = beat_interval * 0.9;

            Serial.printf("[BEAT] Pos:%d/%d | %s | Note:%s | kick=0b%d%d%d%d\n",
                active_playhead + 1, track_length,
                (current_block==STRAIGHT?"STRAIGHT"
                :current_block==LEFT?"LEFT":"RIGHT"),
                state.note,
                (kick_bits_active>>3)&1, (kick_bits_active>>2)&1,
                (kick_bits_active>>1)&1, kick_bits_active&1);

                if (kick_bits_active & 0x01) {
                 kick_current_frequency = KICK_FREQUENCY;
                    kick_start_time        = current_time;
                         kick_duration          = subbeat_interval * 0.2;

             Serial.printf(
                "[KICK] Pos:%d Viertel:1\n",
                active_playhead + 1
        );
    }

   updateNeoPixelTrack();


            // erst jetzt playhead weiter
        playhead++;
        if (playhead >= track_length) {
        playhead           = 0;
        current_note_index = start_note_index;
            }
        }
    }       

     // Befehle vom PyBadge empfangen (Read)
    if (Serial2.available()) {
        String command = Serial2.readStringUntil('\n');
        command.trim();   // Entfernt Whitespaces und Zeilenumbruecke

        Serial.print("Befehl: ");
        Serial.println(command);

// Zustandsaenderung basierend auf dem empfangenen Befehl
        if (command == "BPM_UP") {
            // Mit MAX_BPM begrenzen
            if (state.bpm < MAX_BPM) {
                state.bpm += 10;
            } else {
                Serial.println("[SYSTEM] Maximales BPM-Limit erreicht!");
            }
        }
        else if (command == "BPM_DOWN") {
            // Mit MIN_BPM begrenzen
            if (state.bpm > MIN_BPM) {
                state.bpm -= 10;
            } else {
                Serial.println("[SYSTEM] Minimales BPM-Limit erreicht!");
            }
        }

        else if (command == "START") {
           
            state.status   = "RUNNING";
            unsigned long now  = millis();  // Timing synchronisieren, um sofort zu starten
            last_beat_time    = now;
            last_subbeat_ms   = now;
            current_subbeat   = 0;
            kick_bits_active  = 0;

            module_miss_count = 0;      // Watchdog zuruecksetzen
        }
        else if (command == "STOP") {
        
            state.status = "STOPPED";
            Serial2.println("CMD_MENU");  
             Serial.println("[SYSTEM] Track wurde zurueckgesetzt. Gehe zum Menue.");
        }

        // * Startnote ueber das D-Pad aendern *
        else if (command == "NOTE_UP") {
            // Die Startnote darf nur geaendert werden, wenn die Wiedergabe pausiert ist
            if (String(state.status) == "STOPPED") {
                // Einen Schritt im Quintenzirkel vorwaerts (Modulo 12 fuer den Loop)
                start_note_index   = (start_note_index + 1) % 12;
                current_note_index = start_note_index;
                state.note         = circleOfFifths[current_note_index];
                 Serial.printf("[SYSTEM] Startnote geaendert auf: %s\n", state.note);
            }
        }
        else if (command == "NOTE_DOWN") {
            // Die Startnote darf nur geaendert werden, wenn die Wiedergabe pausiert ist
            if (String(state.status) == "STOPPED") {
                // Einen Schritt im Quintenzirkel rueckwaerts (+12 verhindert negative Werte)
                start_note_index   = (start_note_index - 1 + 12) % 12;
                current_note_index = start_note_index;
                state.note         = circleOfFifths[current_note_index];
                Serial.printf("[SYSTEM] Startnote geaendert auf: %s\n", state.note);
            }
        }
        else if (command == "SCAN") {
            // Optional: PyBadge kann auch Scan ausloesen
            scanAndBuildTrack();
        }
    }

    
    // Sende aktuelle Displaydaten an PyBadge
    // Interval von 500ms auf 150ms verkuerzt, damit die UI flüssig mit den Beats updatet

    if (current_time - last_send_time > 150) {
        last_send_time = current_time;

        // Format: "Note,BPM,Status\n" -> Wird vom PyBadge gesplittet

        Serial2.printf("%s,%d,%s\n",
            state.note,
            state.bpm,
            state.status
        );
         // Debug-Ausgabe auf dem PC-Serienmonitor (auskommentiert um Spam zu vermeiden, bei Bedarf aktivieren)
        // Serial.printf("SEND -> %s,%d,%s\n", state.note, state.bpm, state.status);

        }
    }


