import time
import board
import busio
import displayio
import terminalio
import adafruit_imageload
from adafruit_pybadger import pybadger
from adafruit_display_text import label

# =====================================
# 1. UART-KONFIGURATION (115200 Baud)
# =====================================
uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0.05)

display = board.DISPLAY

# =====================================
# 2. SCREEN 1: MENU (Startbildschirm)
# =====================================
menu_group = displayio.Group()

# --- [Reiner Code-Hintergrund fuer das Menue (Dunkelblau)] ---
color_bitmap = displayio.Bitmap(160, 128, 1)
color_palette = displayio.Palette(1)
color_palette[0] = 0x111133  # Dunkelblau (Retro-Feeling)
menu_bg = displayio.TileGrid(color_bitmap, pixel_shader=color_palette, x=0, y=0)
menu_group.append(menu_bg)

# --- [Menue Texte] ---
# Titel
title_label = label.Label(terminalio.FONT, text="CHOOSE START NOTE", color=0xFFFFFF, x=25, y=20)
# Riesige Anzeige fuer die aktuell gewaehlte Note (Scale=4)
menu_note_group = displayio.Group(scale=4, x=55, y=64)
menu_note_label = label.Label(terminalio.FONT, text="C", color=0xFF9900, x=0, y=0)
menu_note_group.append(menu_note_label)
# Anweisungstext
hint_label = label.Label(terminalio.FONT, text="Press START to Race", color=0x00FFFF, x=25, y=110)

menu_group.append(title_label)
menu_group.append(menu_note_group)
menu_group.append(hint_label)

# =====================================
# 3. SCREEN 2: GAME (Rennbildschirm)
# =====================================
main_group = displayio.Group()

# --- [Hintergrundbild laden (bg.bmp)] ---
bg_bitmap, bg_palette = adafruit_imageload.load("/bg.bmp",
                                                bitmap=displayio.Bitmap,
                                                palette=displayio.Palette)
bg = displayio.TileGrid(bg_bitmap, pixel_shader=bg_palette, x=0, y=0)
main_group.append(bg)

# --- [5 animierte Fahrbahnmarkierungen] ---
line_bitmap = displayio.Bitmap(2, 8, 1)
line_palette = displayio.Palette(1)
line_palette[0] = 0xFFFFFF  # Weiß
lines = []
for i in range(5):
    line = displayio.TileGrid(line_bitmap, pixel_shader=line_palette, x=78, y=64 + (i * 13))
    lines.append(line)
    main_group.append(line)

# --- [Eigenes Auto Sprite laden (porsche911.bmp 60x42)] ---
car_bitmap, car_palette = adafruit_imageload.load("/porsche911.bmp",
                                                  bitmap=displayio.Bitmap,
                                                  palette=displayio.Palette)

# Transparenz für den Auto-Hintergrund setzen
# Index 0 ist in der Regel die Hintergrundfarbe (transparent)
car_palette.make_transparent(0)

# Auto platzieren (X=60, da das Bild 40px breit ist -> (160-60)/2 = 50 für die Mitte)
car = displayio.TileGrid(car_bitmap, pixel_shader=car_palette, x=50, y=85)
main_group.append(car)

# --- [Text-UI] ---
text_group = displayio.Group(scale=2)

shadow_gear = label.Label(terminalio.FONT, text="GEAR: -", color=0x818181, x=3, y=6)
shadow_note = label.Label(terminalio.FONT, text="NOTE: -", color=0x818181, x=3, y=16)
shadow_bpm = label.Label(terminalio.FONT, text="BPM: -", color=0x818181, x=3, y=27)

gear_label = label.Label(terminalio.FONT, text="GEAR: -", color=0xFFFFFF, x=2, y=5)
note_label = label.Label(terminalio.FONT, text="NOTE: -", color=0xD9B3FF, x=2, y=15)
bpm_label = label.Label(terminalio.FONT, text="BPM: -", color=0xFFB84D, x=2, y=26)

text_group.append(shadow_gear)
text_group.append(shadow_note)
text_group.append(shadow_bpm)

text_group.append(gear_label)
text_group.append(note_label)
text_group.append(bpm_label)

main_group.append(text_group)

# =====================================
# SYSTEM-START (Zuerst Menue anzeigen)
# =====================================
display.root_group = menu_group # Startbildschirm ist das Menue
current_screen = "MENU" # Statusvariable fuer den aktuellen Bildschirm

current_bpm = 120
is_running = False

# Variablen fuer die Flankenerkennung (Edge Detection)
# Speichert den Zustand der Tasten aus dem vorherigen Frame
prev_up = False
prev_down = False
prev_a = False
prev_b = False

print("PyBadge Racing UI Bereit. Zeige Menue...")

# =====================================
# MAIN LOOP (Hauptschleife)
# =====================================
while True:
    # --- 0. Aktuellen Tastenstatus ganz am Anfang des Loops auslesen
    curr_up = pybadger.button.up
    curr_down = pybadger.button.down
    curr_a = pybadger.button.a
    curr_b = pybadger.button.b

    # --- 1. Daten empfangen (UART Read) ---
    data = uart.readline()
    if data:
        try:
            msg = data.decode("utf-8").strip()

            # Eingabe "X"
            if msg == "CMD_MENU":
                current_screen = "MENU"
                display.root_group = menu_group
                is_running = False

            else:
                parts = msg.split(",")
                if len(parts) == 3:
                    note = parts[0]
                    current_bpm = int(parts[1])
                    status = parts[2]
                    # angschaltung noch fest
                    current_gear = 1

                    # UI-Text aktualisieren
                    gear_label.text = "GEAR: " + str(current_gear)
                    note_label.text = "NOTE: " + note
                    bpm_label.text = "BPM: " + str(current_bpm)

                    shadow_gear.text = "GEAR: " + str(current_gear)
                    shadow_note.text = "NOTE: " + note
                    shadow_bpm.text = "BPM: " + str(current_bpm)

                    # Update Menu-UI (Riesige Note in der Mitte)
                    menu_note_label.text = note
                    is_running = (status == "RUNNING")

        except Exception as e:
            # Fehler abfangen, damit das Programm bei korrupten UART-Daten nicht abstürzt
            pass

    # --- 2. Tastenabfrage und Befehle senden (UART Write) ---
    if current_screen == "MENU":
        # Im Menue: Nur Note waehlen oder Spiel starten
        # Ist die Taste JETZT gedrueckt, war aber VORHER NICHT gedrueckt?
        if curr_up and not prev_up:
            uart.write(b"NOTE_UP\n")
        elif curr_down and not prev_down:
            uart.write(b"NOTE_DOWN\n")
        elif pybadger.button.start:
            # START druecken -> Bildschirm wechseln und Musik starten!
            current_screen = "GAME"
            display.root_group = main_group  # Umschalten auf Rennbildschirm!
            uart.write(b"START\n")
            time.sleep(0.2) # Screen-Wechsel braucht kleinen Delay

    elif current_screen == "GAME":
        # Im Spiel: BPM steuern oder Spiel beenden
        # Auch BPM_UP und BPM_DOWN profitieren von Edge Detection (kein Stottern mehr)
        if curr_a and not prev_a:
            uart.write(b"BPM_UP\n")
        elif curr_b and not prev_b:
            uart.write(b"BPM_DOWN\n")
        elif pybadger.button.select:
            uart.write(b"STOP\n")
            time.sleep(0.2)

    # --- 3. Straßen-Animation (Fahreffekt) ---
    # Laeuft nur, wenn wir im Game-Screen sind UND is_running True ist
    if current_screen == "GAME" and is_running:
        # Liniengeschwindigkeit abhängig von BPM (verstärkt das Geschwindigkeitsgefühl)
        speed = int(current_bpm / 40) + 1

        for line in lines:
            line.y += speed
            # Wenn die Linie den unteren Bildschirmrand verlässt, zurück zum Horizont (Loop)
            if line.y > 128:
                line.y = 64

    # Tastenstatus fuer die naechste Runde speichern
    prev_up = curr_up
    prev_down = curr_down
    prev_a = curr_a
    prev_b = curr_b

    # Bildwiederholrate (Framerate) kontrollieren
    time.sleep(0.01)
