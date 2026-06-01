#include <Arduino.h>

// Definition der Pins fuer UART2
#define RXD2 16
#define TXD2 17

// Datenstruktur (Track-Array und Playhead)
enum BlockType { STRAIGHT, LEFT, RIGHT, EMPTY };
const int MAX_TRACK = 18;   // Maximale Track-Laenge
BlockType track[MAX_TRACK]; // Array zum Speichern der Track-Bloecke
int track_length = 0;       // Aktuelle Laenge des erstellten Tracks
int playhead = 0;           // Aktuelle Wiedergabeposition (Playhead)


struct SystemState {
    int bpm;
    const char* note;
    const char* status;
};
SystemState state;


unsigned long last_send_time = 0;

void setup() {
    // Initialisierung der seriellen USB-Verbindung fuer den PC-Monitor
    Serial.begin(115200);   
    
    // Initialisierung von Hardware-UART2 fuer die Kommunikation mit dem PyBadge
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); 
    
    Serial.println("ESP32 UART Steuerung Bereit...");

    // Dummy-Variablen fuer den ersten Test
    state.bpm = 120;
    state.note = "C";
    state.status = "STOPPED";
}

void loop() {
    // Serielle Eingabe
    if (Serial.available()) {
        char c = Serial.read();
        c = toupper(c); // Kleinbuchstaben in Grossbuchstaben umwandeln

        if (c == 'X') {
            track_length = 0;
            playhead = 0;
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

    // Befehle vom PyBadge empfangen (Read)
    if (Serial2.available()) {
        String command = Serial2.readStringUntil('\n');
        command.trim(); // Entfernt Whitespaces und Zeilenumbruecke

        Serial.print("Befehl erhalten: ");
        Serial.println(command);

        // Zustandsaenderung basierend auf dem empfangenen Befehl testen
        if (command == "BPM_UP") {
            state.bpm += 10;
        }
        else if (command == "BPM_DOWN") {
            if (state.bpm > 60) state.bpm -= 10;
        }
        else if (command == "START") {
            state.status = "RUNNING";
        }
        else if (command == "STOP") {
            state.status = "STOPPED";
        }
    }

    // Sende aktuelle Displaydaten an PyBadge alle 500ms (Write)
    unsigned long current_time = millis();
    if (current_time - last_send_time > 500) {
        last_send_time = current_time;

        // Format: "Note,BPM,Status\n" -> Wird vom PyBadge gesplittet
        Serial2.printf("%s,%d,%s\n",
            state.note,
            state.bpm,
            state.status
        );
        
        // Debug-Ausgabe auf dem PC-Serienmonitor
        Serial.printf("SEND -> %s,%d,%s\n",
            state.note,
            state.bpm,
            state.status
        );
    }
}