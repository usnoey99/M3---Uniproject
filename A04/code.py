import time
import board
import displayio
import terminalio
import busio
from adafruit_pybadger import pybadger
from adafruit_display_text import label

# =====================================
# DISPLAY SETUP
# =====================================
display = board.DISPLAY
main_group = displayio.Group()
display.root_group = main_group

note_label = label.Label(terminalio.FONT, text="NOTE: -", scale=2, x=10, y=30)
bpm_label = label.Label(terminalio.FONT, text="BPM: -", scale=2, x=10, y=70)
status_label = label.Label(terminalio.FONT, text="STATUS: INIT", scale=1, x=10, y=120)

main_group.append(note_label)
main_group.append(bpm_label)
main_group.append(status_label)

# =====================================
# UART SETUP (115200 Baud)
# =====================================
# Aktiviert Hardware-UART ueber die TX/RX-Pins des PyBadge
uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0.05)

print("PyBadge UI Bereit. Warte auf ESP32...")

# =====================================
# MAIN LOOP
# =====================================
while True:
    # Daten vom ESP32 empfangen (Read)
    data = uart.readline()
    if data:
        try:
            msg = data.decode("utf-8").strip()
            # Erwartetes Format: "Note,BPM,Status" (z.B. "C,120,RUNNING")
            parts = msg.split(",")
            if len(parts) == 3:
                note_label.text = "NOTE: " + parts[0]
                bpm_label.text = "BPM: " + parts[1]
                status_label.text = "STATUS: " + parts[2]
        except Exception as e:
            print("UART Read Error:", e)

    # Tasten abfragen und UART-Befehle an ESP32 senden (Write)

    if pybadger.button.a:
        uart.write(b"BPM_UP\n")
        print("Gesendet: BPM_UP")
        time.sleep(0.2)

    if pybadger.button.b:
        uart.write(b"BPM_DOWN\n")
        print("Gesendet: BPM_DOWN")
        time.sleep(0.2) # Debouncing (Entprellung der Taste)

    if pybadger.button.start:
        uart.write(b"START\n")
        print("Gesendet: START")
        time.sleep(0.2)

    if pybadger.button.select:
        uart.write(b"STOP\n")
        print("Gesendet: STOP")
        time.sleep(0.2)

    time.sleep(0.01)