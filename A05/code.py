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

# --- [Eigenes Auto Sprite laden (car.bmp)] ---
car_bitmap, car_palette = adafruit_imageload.load("/car.bmp",
                                                  bitmap=displayio.Bitmap,
                                                  palette=displayio.Palette)

# Transparenz für den Auto-Hintergrund setzen
# Index 0 ist in der Regel die Hintergrundfarbe (transparent)
car_palette.make_transparent(0)

# Auto platzieren (X=60, da das Bild 40px breit ist -> (160-40)/2 = 60 für die Mitte)
car = displayio.TileGrid(car_bitmap, pixel_shader=car_palette, x=60, y=100)
main_group.append(car)

# --- [Text-UI] ---
gear_label = label.Label(terminalio.FONT, text="GEAR: 1", color=0xFFFFFF, x=5, y=10)
note_label = label.Label(terminalio.FONT, text="NOTE: -", color=0xFFFF00, x=55, y=10)
bpm_label = label.Label(terminalio.FONT, text="BPM: -", color=0x00FFFF, x=110, y=10)

main_group.append(gear_label)
main_group.append(note_label)
main_group.append(bpm_label)

display.root_group = main_group

# =====================================
# Initialisierung der Statusvariablen
# =====================================
current_bpm = 120
current_gear = 1
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

                # UI-Text aktualisieren
                note_label.text = "NOTE: " + note
                bpm_label.text = "BPM: " + str(current_bpm)
                is_running = (status == "RUNNING")

                # angschaltung noch fest
                current_gear = 1

                gear_label.text = "GEAR: " + str(current_gear)
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