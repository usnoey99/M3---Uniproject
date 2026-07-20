import time
import board
import busio
import displayio
import terminalio
import adafruit_imageload
import gc  # Garbage Collector
from adafruit_pybadger import pybadger
from adafruit_display_text import label

# =====================================
# 1. UART-KONFIGURATION (115200 Baud)
# =====================================
uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0.05)

display = board.DISPLAY

# =====================================
# 2. SCREEN 0: BOOT (Warten auf ESP32)
# =====================================
boot_group = displayio.Group()
boot_label = label.Label(terminalio.FONT, text="CONNECTING \nTRACK STATION...", color=0x00FF00, x=20, y=60)
boot_group.append(boot_label)


# =====================================
# 3. SCREEN 1: MENU (Startbildschirm)
# =====================================
menu_group = displayio.Group()

# --- [Reiner Code-Hintergrund fuer das Menue (Dunkelblau)] ---
color_bitmap = displayio.Bitmap(160, 128, 1)
color_palette = displayio.Palette(1)
color_palette[0] = 0x111133  # Dunkelblau
menu_bg = displayio.TileGrid(color_bitmap, pixel_shader=color_palette, x=0, y=0)
menu_group.append(menu_bg)

# --- [Menue Texte] ---
# Titel
title_label = label.Label(terminalio.FONT, text="CHOOSE YOUR FIRST NOTE", color=0xFFFFFF, x=15, y=20)
# Riesige Anzeige fuer die aktuell gewaehlte Note (Scale=4)
menu_note_group = displayio.Group(scale=4, x=60, y=64)
menu_note_label = label.Label(terminalio.FONT, text="C", color=0xFF9900, x=0, y=0)
menu_note_group.append(menu_note_label)
# Anweisungstext
hint_label = label.Label(terminalio.FONT, text="Press >>START<< to race", color=0x00FFFF, x=12, y=110)

menu_group.append(title_label)
menu_group.append(menu_note_group)
menu_group.append(hint_label)


# =====================================
# 4. SCREEN 2: WARNUNG (Kein Track)
# =====================================
warn_group = displayio.Group()
warn_bg_palette = displayio.Palette(1)
warn_bg_palette[0] = 0xAA0000  # Dunkelrot fuer Warnung
warn_bg = displayio.TileGrid(color_bitmap, pixel_shader=warn_bg_palette, x=0, y=0)
warn_group.append(warn_bg)

warn_title = label.Label(terminalio.FONT, text="WARNING!", color=0xFFFFFF, x=35, y=40, scale=2)
warn_desc = label.Label(terminalio.FONT, text="No Track Connected", color=0xFFFFFF, x=25, y=80)
warn_group.append(warn_title)
warn_group.append(warn_desc)



# =====================================
# 5. SCREEN 3: GAME (Rennbildschirm)
# =====================================
main_group = displayio.Group()

# --- Hintergrundbild laden (bg.bmp) ---
bg_bitmap = displayio.OnDiskBitmap("/bg.bmp")
bg = displayio.TileGrid(bg_bitmap, pixel_shader=bg_bitmap.pixel_shader, x=0, y=0)
main_group.append(bg)

# --- 5 animierte Fahrbahnmarkierungen ---
line_bitmap = displayio.Bitmap(2, 8, 1)
line_palette = displayio.Palette(1)
line_palette[0] = 0xFFFFFF  # Weiß
lines = []
for i in range(5):
    line = displayio.TileGrid(line_bitmap, pixel_shader=line_palette, x=78, y=64 + (i * 13))
    lines.append(line)
    main_group.append(line)

# Auto Sprites laden fuer die Reifen-Animation
gc.collect()

car1_bitmap, car1_palette = adafruit_imageload.load("/porsche911.bmp", bitmap=displayio.Bitmap, palette=displayio.Palette)
# Transparenz für den Auto-Hintergrund setzen
bg_index1 = car1_bitmap[0, 0]
car1_palette.make_transparent(bg_index1)
car1 = displayio.TileGrid(car1_bitmap, pixel_shader=car1_palette, x=50, y=85)

car2_bitmap, car2_palette = adafruit_imageload.load("/porsche911_2.bmp", bitmap=displayio.Bitmap, palette=displayio.Palette)
bg_index2 = car2_bitmap[0, 0]
car2_palette.make_transparent(bg_index2)
car2 = displayio.TileGrid(car2_bitmap, pixel_shader=car2_palette, x=50, y=85)
car2.hidden = True

main_group.append(car1)
main_group.append(car2)
car_anim_counter = 0


# --- [Text-UI] ---
text_group = displayio.Group(scale=2, x=5, y=10)
shadow_note = label.Label(terminalio.FONT, text="NOTE: -", color=0x818181, x=2, y=2)
note_label = label.Label(terminalio.FONT, text="NOTE: -", color=0xFF9900, x=1, y=1)
text_group.append(shadow_note)
text_group.append(note_label)
main_group.append(text_group)

# =====================================
# SYSTEM-START (Zuerst Menue anzeigen)
# =====================================
display.root_group = boot_group  # Startet mit dem Boot-Screen
current_screen = "BOOT"
is_running = False

current_bpm = 120  # Wird im Hintergrund fuer die Animationsgeschwindigkeit verwendet
warn_timer = 0

# Flankenerkennung fuer das D-Pad
prev_up = False
prev_down = False

print("PyBadge Racing UI Bereit. Warte auf ESP32...")

# =====================================
# MAIN LOOP (Hauptschleife)
# =====================================
while True:
    # --- 0. Aktuellen Tastenstatus ganz am Anfang des Loops auslesen
    curr_up = pybadger.button.up
    curr_down = pybadger.button.down

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

                # Im Menue wieder auf Lila (Wartestatus) schalten
                pybadger.pixels.fill((128, 0, 128))

            elif msg == "CMD_NO_TRACK":
                current_screen = "WARN"
                display.root_group = warn_group
                warn_timer = time.monotonic()

            elif msg == "CMD_START_OK":
                # Track vorhanden! F1 Racing Start-Animation beginnt
                pybadger.pixels.fill((0, 0, 0)) # Lila aus
                time.sleep(0.2)

                # PyBadge LEDs: 0 (links) bis 4 (rechts). Von rechts nach links rot einschalten
                for i in range(4, -1, -1):
                    pybadger.pixels[i] = (255, 0, 0)
                    time.sleep(0.5)

                time.sleep(0.5) # Kurzes Warten fuer Spannung

                # LEDs ausschalten, Bildschirm wechseln & Spiel starten
                pybadger.pixels.fill((0, 0, 0))
                current_screen = "GAME"
                display.root_group = main_group
                is_running = True

                # Finaler Befehl an ESP32: Musik starten!
                uart.write(b"START_MUSIC\n")

            else:
                parts = msg.split(",")
                if len(parts) == 3:
                    note = parts[0]
                    current_bpm = int(parts[1])
                    status = parts[2]

                    # Wenn wir noch im Boot-Screen sind, schalte bei Datenempfang das Menue frei
                    if current_screen == "BOOT":
                        current_screen = "MENU"
                        display.root_group = menu_group

                        # Bestaetigung an ESP32 senden
                        uart.write(b"INIT_OK\n")

                        # Nach dem Booten NeoPixel dauerhaft in Lila leuchten lassen
                        pybadger.pixels.brightness = 0.5
                        pybadger.pixels.fill((128, 0, 128))

                    # UI aktualisieren (Nur Note sichtbar, BPM fliesst unsichtbar in die Animation)
                    note_label.text = "NOTE: " + note
                    shadow_note.text = "NOTE: " + note
                    menu_note_label.text = note

                    if status == "RUNNING":
                        current_screen = "GAME"
                        display.root_group = main_group
                        is_running = True

                    elif status == "STOPPED" and current_screen == "GAME":
                        # Falls am PC ein manueller Stop ausgeloest wurde
                        current_screen = "MENU"
                        display.root_group = menu_group
                        is_running = False
                        pybadger.pixels.fill((128, 0, 128))


        except Exception as e:
            # Fehler abfangen, damit das Programm bei korrupten UART-Daten nicht abstürzt
            pass

    # --- 2. Timer-Logik fuer Warnungs-Bildschirm ---
    if current_screen == "WARN":
        if time.monotonic() - warn_timer > 2.5:
            current_screen = "MENU"
            display.root_group = menu_group

    # --- 3. Tastenabfrage (Flankenerkennung) ---
    if current_screen == "MENU":
        if curr_up and not prev_up:
            uart.write(b"NOTE_UP\n")
        elif curr_down and not prev_down:
            uart.write(b"NOTE_DOWN\n")
        elif pybadger.button.start:
            # Nicht direkt starten, sondern erst ESP32 nach Tracks fragen (CHECK)
            uart.write(b"CHECK_START\n")
            time.sleep(0.2)

    elif current_screen == "GAME":
        # A und B Tasten geloescht, da das Pedal das BPM-Handling uebernimmt
        if pybadger.button.select:
            uart.write(b"STOP\n")
            time.sleep(0.2)

    # --- 4. Straßen- & Reifen-Animation ---
    # Nutzt das vom physischen Pedal gesendete BPM (current_bpm) fuer dynamische Geschwindigkeit
    if current_screen == "GAME" and is_running:
        speed = int(current_bpm / 40) + 1

        for line in lines:
            line.y += speed
            if line.y > 128:
                line.y = 64

        car_anim_counter += 1
        anim_threshold = max(1, 10 - speed)

        if car_anim_counter >= anim_threshold:
            car1.hidden = not car1.hidden
            car2.hidden = not car1.hidden
            car_anim_counter = 0

    # Tastenstatus sichern
    prev_up = curr_up
    prev_down = curr_down
    time.sleep(0.01)
