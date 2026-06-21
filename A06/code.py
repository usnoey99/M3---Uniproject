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

# =====================================
# 2. DISPLAY-SETUP (Renn-Hintergrund und UI)
# =====================================
display = board.DISPLAY
main_group = displayio.Group()

# --- [Hintergrundbild laden (bg.bmp)] ---
# Das gesamte Hintergrundbild (160x128) ersetzt Himmel, Gras und Straße
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

display.root_group = main_group

# =====================================
# Initialisierung der Statusvariablen
# =====================================
current_bpm = 120
# current_gear = 1
is_running = False

print("PyBadge Racing UI Bereit. Warte auf ESP32...")

# =====================================
# MAIN LOOP (Hauptschleife)
# =====================================
while True:
    # --- 1. Daten empfangen (UART Read) ---
    data = uart.readline()
    if data:
        try:
            msg = data.decode("utf-8").strip()
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

                is_running = (status == "RUNNING")

        except Exception as e:
            # Fehler abfangen, damit das Programm bei korrupten UART-Daten nicht abstürzt
            pass

    # --- 2. Tastenabfrage und Befehle senden (UART Write) ---
    if pybadger.button.a:
        uart.write(b"BPM_UP\n")
        print("Gesendet: BPM_UP")
        time.sleep(0.2)  # Tastenentprellung (Debouncing)

    if pybadger.button.b:
        uart.write(b"BPM_DOWN\n")
        print("Gesendet: BPM_DOWN")
        time.sleep(0.2)

    if pybadger.button.start:
        uart.write(b"START\n")
        print("Gesendet: START")
        time.sleep(0.2)

    if pybadger.button.select:
        uart.write(b"STOP\n")
        print("Gesendet: STOP")
        time.sleep(0.2)

    # --- 3. Straßen-Animation (Fahreffekt) ---
    if is_running:
        # Liniengeschwindigkeit abhängig von BPM (verstärkt das Geschwindigkeitsgefühl)
        speed = int(current_bpm / 40) + 1

        for line in lines:
            line.y += speed
            # Wenn die Linie den unteren Bildschirmrand verlässt, zurück zum Horizont (Loop)
            if line.y > 128:
                line.y = 64

    # Bildwiederholrate (Framerate) kontrollieren
    time.sleep(0.01)
