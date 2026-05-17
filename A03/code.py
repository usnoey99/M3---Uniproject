# =====================================
# RACETRACK - Prototype
# PyBadge + CircuitPython
# =====================================

import time
import board
import audiocore
import audioio
import displayio
import terminalio

from adafruit_pybadger import pybadger
from adafruit_display_text import label

# =====================================
# AUDIO SETUP
# =====================================

pybadger._speaker_enable.value = True

audio = audioio.AudioOut(board.SPEAKER)

# =====================================
# DISPLAY SETUP
# =====================================

display = board.DISPLAY

main_group = displayio.Group()

display.root_group = main_group

# -------------------------------------
# NOTE LABEL
# -------------------------------------

note_label = label.Label(terminalio.FONT, text="NOTE: C", scale=2, x=10, y=30)

# -------------------------------------
# BPM LABEL
# -------------------------------------

bpm_label = label.Label(terminalio.FONT, text="BPM: 120", scale=2, x=10, y=70)

# -------------------------------------
# TRACK LABEL
# -------------------------------------

track_label = label.Label(terminalio.FONT, text="TRACK: straight", scale=1, x=10, y=110)

# -------------------------------------
# STATUS LABEL
# -------------------------------------

status_label = label.Label(terminalio.FONT, text="[ PRESS START ]", scale=1, x=10, y=140)

main_group.append(note_label)
main_group.append(bpm_label)
main_group.append(track_label)
main_group.append(status_label)

# =====================================
# CIRCLE OF FIFTHS
# =====================================

circle = ["F", "C", "G", "D", "A", "E", "B"]

# Start at C
current_index = 1

# =====================================
# TRACK LAYOUT
# =====================================

track = ["straight", "right", "straight", "left", "straight", "straight", "right"]

current_step = 0

# =====================================
# BPM SYSTEM
# =====================================

bpm = 120

min_bpm = 60
max_bpm = 240

step_interval = 60 / bpm

last_step_time = time.monotonic()

# =====================================
# LOOP STATE
# =====================================

is_running = False

# =====================================
# PLAY NOTE
# =====================================

wave_file = None


def play_note(note):

    global wave_file

    filename = "/music/" + note + ".wav"
    print("PLAY:", filename)

    if audio.playing:
        audio.stop()

    if wave_file is not None:
        wave_file.close()
        wave_file = None

    try:
        wave_file = open(filename, "rb")
        wave = audiocore.WaveFile(wave_file)
        audio.play(wave)
        print("PLAYING:", filename)
    except OSError:
        print("WAV FILE NOT FOUND:", filename)


# =====================================
# UPDATE DISPLAY
# =====================================


def update_display(note, current_track):

    note_label.text = "NOTE: " + note

    bpm_label.text = "BPM: " + str(bpm)

    track_label.text = "TRACK: " + current_track


# =====================================
# ADVANCE STEP
# =====================================


def advance_step():

    global current_index
    global current_step

    current_track = track[current_step]

    # ---------------------------------
    # HARMONIC MOVEMENT
    # ---------------------------------

    if current_track == "right":

        current_index += 1
    elif current_track == "left":

        current_index -= 1
    # straight = stay

    # ---------------------------------
    # WRAP AROUND
    # ---------------------------------

    current_index %= len(circle)

    current_note = circle[current_index]

    # ---------------------------------
    # DEBUG OUTPUT
    # ---------------------------------

    print("----------------------")
    print("STEP:", current_step)
    print("TRACK:", current_track)
    print("NOTE:", current_note)
    print("BPM:", bpm)

    # ---------------------------------
    # PLAY AUDIO
    # ---------------------------------

    play_note(current_note)

    # ---------------------------------
    # UPDATE VISUALS
    # ---------------------------------

    update_display(current_note, current_track)

    # ---------------------------------
    # NEXT STEP
    # ---------------------------------

    current_step += 1

    if current_step >= len(track):

        current_step = 0


# =====================================
# STARTUP
# =====================================

print("======================")
print("RACETRACK START")
print("======================")

update_display("C", "straight")

# =====================================
# MAIN LOOP
# =====================================

while True:

    # ---------------------------------
    # START BUTTON
    # ---------------------------------

    if pybadger.button.start:

        if not is_running:

            is_running = True
            last_step_time = time.monotonic()
            status_label.text = "[ RUNNING ]"

            print("=== LOOP START ===")

        time.sleep(0.2)

    # ---------------------------------
    # SELECT BUTTON
    # ---------------------------------

    if pybadger.button.select:

        if is_running:

            is_running = False

            if audio.playing:
                audio.stop()

            status_label.text = "[ STOPPED ]"

            print("=== LOOP STOP ===")

        time.sleep(0.2)

    # ---------------------------------
    # BPM DOWN (BUTTON A)
    # ---------------------------------

    if pybadger.button.a:

        bpm -= 10

        if bpm < min_bpm:

            bpm = min_bpm

        step_interval = 60 / bpm

        print("BPM DOWN:", bpm)

        update_display(circle[current_index], track[current_step])

        time.sleep(0.2)

    # ---------------------------------
    # BPM UP (BUTTON B)
    # ---------------------------------

    if pybadger.button.b:

        bpm += 10

        if bpm > max_bpm:

            bpm = max_bpm

        step_interval = 60 / bpm

        print("BPM UP:", bpm)

        update_display(circle[current_index], track[current_step])

        time.sleep(0.2)

    # ---------------------------------
    # BPM CLOCK - when is_running
    # ---------------------------------

    if is_running:

        current_time = time.monotonic()

        if current_time - last_step_time > step_interval:

            last_step_time = current_time

            advance_step()
