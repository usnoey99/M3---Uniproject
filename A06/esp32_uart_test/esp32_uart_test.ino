#include <Arduino.h>
#include "BluetoothA2DPSource.h"
#include <math.h>

// Definition der Pins fuer UART2
#define RXD2 16
#define TXD2 17

// BPM-Grenzen fuer Systemsicherheit
#define MAX_BPM 240  // Maximale BPM-Grenze, um UI-Lag oder Abstuerze zu verhindern
#define MIN_BPM 60   // Minimale BPM-Grenze
#define STD_BPM 120  // Standard BPM


// Datenstruktur (Track-Array und Playhead)
enum BlockType { STRAIGHT, LEFT, RIGHT, EMPTY };
const int MAX_TRACK = 18;   // Maximale Track-Laenge
BlockType track[MAX_TRACK]; // Array zum Speichern der Track-Bloecke
int track_length = 0;       // Aktuelle Laenge des erstellten Tracks
int playhead = 0;           // Aktuelle Wiedergabeposition (Playhead)


//  Musik-Engine: Quintenzirkel (Circle of Fifths)
// Lookup-Table fuer die 12 Noten. Einfacher und schneller als mathematische Berechnungen.
const char* circleOfFifths[12] = {"C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "F"};
int start_note_index = 0; // Startet bei "C" (Index 0)
int current_note_index = start_note_index;

// Frequenzen (Hz) fuer die 12 Noten des Quintenzirkels (Basis: C4 bis B4)
const float note_frequencies[12] = {
    261.63, // C
    392.00, // G
    293.66, // D
    440.00, // A
    329.63, // E
    493.88, // B (H)
    369.99, // F#
    277.18, // C#
    415.30, // G#
    311.13, // D#
    466.16, // A#
    349.23  // F
};

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

// Callback-Funktion fuer das Bluetooth-Audio-Streaming
int32_t get_audio_data(Frame *channels, int32_t channel_len) {
    static float sample_index = 0;
    
    for (int i = 0; i < channel_len; i++) {
        // Ueberpruefen, ob ein Ton aktiv ist und die Zeit noch nicht abgelaufen ist
        if (current_frequency > 0 && (millis() - sound_start_time < sound_duration)) {
            // Sinuswelle berechnen (5000 ist die Lautstaerke/Amplitude)
            int16_t sample = sin(sample_index) * 5000; 
            channels[i].channel1 = sample; // left
            channels[i].channel2 = sample; // right
            
            // Index basierend auf Frequenz und Samplerate (44100 Hz) erhoehen
            sample_index += (2 * M_PI * current_frequency) / 44100.0;
            if (sample_index >= 2 * M_PI) sample_index -= 2 * M_PI;
        } else {
            // Stille ausgeben, wenn kein Ton spielt
            channels[i].channel1 = 0;
            channels[i].channel2 = 0;
            if (current_frequency > 0) current_frequency = 0; // Ton ausschalten
        }
    }
    return channel_len;
}

void setup() {
    // Initialisierung der seriellen USB-Verbindung fuer den PC-Monitor
    Serial.begin(115200);
    delay(1000); 
    
    // Initialisierung von Hardware-UART2 fuer die Kommunikation mit dem PyBadge
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    Serial.println("ESP32 UART Steuerung Bereit...");

    // Start-Variablen initialisieren
    state.bpm = STD_BPM;
    state.note = circleOfFifths[current_note_index]; // Startnote "C"
    state.status = "STOPPED";

    // Bluetooth A2DP Source starten (Hier den Namen des Lautsprechers eintragen)
    Serial.println("[SYSTEM] Verbinde mit Bluetooth-Lautsprecher...");
    a2dp_source.start("SoundCore 2", get_audio_data);
}

void loop() {
    unsigned long current_time = millis();

    // Serielle Eingabe
    if (Serial.available()) {
        char c = Serial.read();
        c = toupper(c); // Kleinbuchstaben in Grossbuchstaben umwandeln

        if (c == 'X') {
            track_length = 0;
            playhead = 0;
            current_note_index = start_note_index; // Note auch auf "C" zuruecksetzen
            state.bpm = STD_BPM;
            state.note = circleOfFifths[current_note_index];
            Serial.println("[SYSTEM] Track wurde zurueckgesetzt.");
        } 
        else if ((c == 'L' || c == 'R' || c == 'S') && track_length < MAX_TRACK) {
            if (c == 'L') track[track_length] = LEFT;
            else if (c == 'R') track[track_length] = RIGHT;
            else if (c == 'S') track[track_length] = STRAIGHT;
            
            track_length++;
            Serial.printf("[TRACK] Block hinzugefuegt: %c (Aktuelle Gesamtlaenge: %d)\n", c, track_length);
        }
    }

    // Playback-Engine (BPM-Timing mit millis)
    // Laeuft nur, wenn der Status "RUNNING" ist und der Track Bloecke enthaelt
    if (String(state.status) == "RUNNING" && track_length > 0) {
        
        // Berechne Millisekunden pro Schlag (Interval = 60000 / BPM)
        unsigned long beat_interval = 60000 / state.bpm;

        if (current_time - last_beat_time >= beat_interval) {
            last_beat_time = current_time;

            // Musik-Engine: Notenberechnung basierend auf Track-Richtung
            BlockType current_block = track[playhead];

            if (current_block == RIGHT) {
                // Im Uhrzeigersinn (+1 Schritt im Array, mit Modulo 12)
                current_note_index = (current_note_index + 1) % 12;
            } 
            else if (current_block == LEFT) {
                // Gegen den Uhrzeigersinn (-1 Schritt im Array)
                current_note_index = (current_note_index - 1 + 12) % 12;
            }
            // STRAIGHT aendert die Note nicht.

            // Aktualisiere aktuelle Note im System-Status
            state.note = circleOfFifths[current_note_index];

            // Audio-Trigger fuer die Audio-Synthese (BPM-synchronisiert)
            current_frequency = note_frequencies[current_note_index];
            sound_start_time = current_time;
            // Tonlaenge auf 80% des Beat-Intervalls setzen (Aendert sich dynamisch mit der BPM)
            sound_duration = beat_interval * 0.8;

            // Debug-Ausgabe auf dem PC-Serienmonitor
            Serial.printf("[PLAYBACK] Pos: %d/%d | Block: %s | Note: %s\n", 
                playhead + 1, 
                track_length, 
                (current_block == STRAIGHT ? "STRAIGHT" : (current_block == LEFT ? "LEFT" : "RIGHT")), 
                state.note);

            // Playhead weiterbewegen (Loop-Logik)
            playhead++;
            if (playhead >= track_length) {
                playhead = 0; // Zurueck zum Anfang des Tracks

                // Setze die Basisnote bei jedem neuen Loop auf 'C' zureuck.
                // Spaeter kann diese 0 durch eine Variable fuer die vom User gewaehlte Startnote ersetzt werden.
                current_note_index = start_note_index;

            }
        }
    }

    // Befehle vom PyBadge empfangen (Read)
    if (Serial2.available()) {
        String command = Serial2.readStringUntil('\n');
        command.trim(); // Entfernt Whitespaces und Zeilenumbruecke

        Serial.print("Befehl erhalten: ");
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
            state.status = "RUNNING";
            last_beat_time = millis(); // Timing synchronisieren, um sofort zu starten
        }
        else if (command == "STOP") {
            state.status = "STOPPED";
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