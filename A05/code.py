import time
import board
import busio
import displayio
import terminalio
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

# --- [Hintergrund: Himmel, Gras, Straße] ---
sky_bitmap = displayio.Bitmap(160, 64, 1)
sky_palette = displayio.Palette(1)
sky_palette[0] = 0x5C94FC  # Himmelblau
sky = displayio.TileGrid(sky_bitmap, pixel_shader=sky_palette, x=0, y=0)

grass_bitmap = displayio.Bitmap(160, 64, 1)
grass_palette = displayio.Palette(1)
grass_palette[0] = 0x34C924  # Grün
grass = displayio.TileGrid(grass_bitmap, pixel_shader=grass_palette, x=0, y=64)

road_bitmap = displayio.Bitmap(80, 64, 1)
road_palette = displayio.Palette(1)
road_palette[0] = 0x808080  # Grau
road = displayio.TileGrid(road_bitmap, pixel_shader=road_palette, x=40, y=64)

main_group.append(sky)
main_group.append(grass)
main_group.append(road)

# --- [3 animierte Fahrbahnmarkierungen] ---
line_bitmap = displayio.Bitmap(4, 15, 1)
line_palette = displayio.Palette(1)
line_palette[0] = 0xFFFFFF  # Weiß
lines = []
for i in range(3):
    line = displayio.TileGrid(line_bitmap, pixel_shader=line_palette, x=78, y=64 + (i * 20))
    lines.append(line)
    main_group.append(line)

# --- [Eigenes Auto] ---
car_bitmap = displayio.Bitmap(20, 16, 1)
car_palette = displayio.Palette(1)
car_palette[0] = 0xFF0000  # Rot
car = displayio.TileGrid(car_bitmap, pixel_shader=car_palette, x=70, y=105)
main_group.append(car)

# --- [Text-UI] ---
# GEAR ist vorerst statisch (Oktave noch nicht im ESP32 implementiert)
gear_label = label.Label(terminalio.FONT, text="GEAR: 1", color=0xFFFFFF, x=5, y=10)
note_label = label.Label(terminalio.FONT, text="NOTE: -", color=0xFFFF00, x=65, y=10)
bpm_label = label.Label(terminalio.FONT, text="BPM: -", color=0x00FFFF, x=110, y=10)

main_group.append(gear_label)
main_group.append(note_label)
main_group.append(bpm_label)

display.root_group = main_group

# =====================================
# Initialisierung der Statusvariablen
# =====================================
current_bpm = 120
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

        except Exception as e:
            pass

    # --- 2. Tastenabfrage und Befehle senden (UART Write) ---
    # Prototyp-Phase: Da das Hardware-Pedal noch fehlt, steuern A/B temporär die BPM

    if pybadger.button.a:
        uart.write(b"BPM_UP\n")
        print("Gesendet: BPM_UP")
        time.sleep(0.2)

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
        # Liniengeschwindigkeit abhängig von BPM (Das temporäre Gaspedal)
        speed = int(current_bpm / 40) + 1

        for line in lines:
            line.y += speed
            # Wenn die Linie den unteren Bildschirmrand verlässt, zurück zum Horizont (Loop)
            if line.y > 128:
                line.y = 64

    # Bildwiederholrate (Framerate) kontrollieren
    time.sleep(0.01)