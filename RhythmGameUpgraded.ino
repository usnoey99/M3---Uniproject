/**
 * 
 * Our main file
 * 
 */
 
// We need the Adafruit Arcada Library for our basic functionality
#include <Adafruit_Arcada.h>
// We can also include our own files in the project
#include "images.h"

// Global variables
Adafruit_Arcada arcada;

File musicFile;
bool musicIsPlaying = false;
uint32_t bpmTiming = 400;
uint32_t beatTime;

int32_t timingOffset;
bool buttonPressedForLastBeat;
bool buttonReleasedForLastBeat;

int32_t offsetX = 0;
int32_t offsetY = 0;
uint8_t directionOffset = 0;
uint8_t beatDirection = 0;
bool directionUpdate = false;

bool inMenu = true;
bool easyMode = true;
bool menuButtonReleased = true;
bool inBlack = false;



/**
 * The Arduino setup function (called once on startup)
 */
void setup() {
  // initialize arcada library
  if (!arcada.arcadaBegin())
  {
    errorReporter("Fatal error: Arcada library failed.");
  }

  // initialize the file system on the flash (first flashing should be done via CircuitPython)
  if (!arcada.filesysBegin()) { // arcada.filesysBeginMSD() for "USB-Stick" mode
    errorReporter("Fatal error: Filesystem failed.");
  }

  // initialize serial connection
  Serial.begin(9600);
  Serial.println("Booting into game...");
  
  // configure board display and enable frame buffer mode
  arcada.displayBegin();
  arcada.setBacklight(255);
  arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT);

  Serial.println("Initializing display...");

  // we start our loop in the menu
  inMenu = true;
  
  Serial.println("Finished booting!");
}


/**
 * The Arduino loop function (called repeatedly)
 */
void loop() {
  // let's just pretend it's a state machine
  if (inMenu) {
    handleMenu();
  } else if (!inMenu && inBlack){
    handleBlack();
  } else {
    handleGameplay();
  }

  // render the frame buffer to the screen
  arcada.blitFrameBuffer(0, 0, true, false);
  

  // perform standard board functions (cooperative multitasking)
  yield();
}

void handleBlack() {
  GFXcanvas16 *canvas = arcada.getCanvas();
  canvas->fillScreen(ARCADA_BLACK);
  arcada.blitFrameBuffer(0, 0, true, false);
  delay(1500);
  loadMusic();
  randomSeed(millis());
  inBlack = false;
}

/**
 * Handle menu input and rendering
 */
void handleMenu() {
  uint8_t buttonState = arcada.readButtons();
  // Serial.println(buttonState);

  // start pressed: load music and switch to game state
  if (buttonState & ARCADA_BUTTONMASK_START) {
    inBlack = true;
    inMenu = false;
  }

  // select pressed: toggle difficulty
  if (buttonState & ARCADA_BUTTONMASK_SELECT) {
    if (menuButtonReleased) {
      menuButtonReleased = false;
      easyMode = !easyMode;
    }
  } else {
    menuButtonReleased = true;
  }

 // use standard rendering functions without directly updating the screen
  GFXcanvas16 *canvas = arcada.getCanvas();
  canvas->fillScreen(ARCADA_WHITE);
  
  canvas->setTextSize(2);
  canvas->setTextColor(ARCADA_BLUE);
  canvas->setCursor(2, 2);
  canvas->print("Not your");
  canvas->setTextColor(ARCADA_PURPLE);
  canvas->setCursor(18, 18);
  canvas->print("Ordinary");
  canvas->setTextColor(ARCADA_RED);
  canvas->setCursor(34, 34);
  canvas->print("Percussion");
  canvas->setTextColor(ARCADA_ORANGE);
  canvas->setCursor(50, 50);
  canvas->print("Escapade");

  if (easyMode) {
    canvas->drawRGBBitmap(ARCADA_TFT_WIDTH/2 - 32, ARCADA_TFT_HEIGHT - 64, drum_easy, 64, 64);
  } else {
    canvas->drawRGBBitmap(ARCADA_TFT_WIDTH/2 - 32, ARCADA_TFT_HEIGHT - 64, drum_hard, 64, 64);
  }
  
  
}


/*
 * Handle game rendering and partial input
 */
void handleGameplay() {
  // music main loop
  if (musicIsPlaying) {
    // fill music buffer if needed
    if (arcada.WavReadyForData()) {
      wavStatus status = arcada.WavReadFile();
    }
  } 

  // handle button release (did that here because in the interrupt buttons sometimes don't seem to be debounced...)
  uint8_t buttonState = arcada.readButtons();
 
  if (!(buttonState & ARCADA_BUTTONMASK_A)) {
    buttonReleasedForLastBeat = true;
  }

  // render main loop
  uint32_t timeNow = millis();

  GFXcanvas16 *canvas = arcada.getCanvas();

  // calculate circle size from time offset (offset has to be signed but time is unsigned!)
  // bit shift divides value by 2
  int32_t circleSize = abs((int32_t)(beatTime - timeNow))>>1;

  if (circleSize > 100) {
    circleSize = 100;
  }

  switch (directionOffset){
    case 0:
      offsetX = circleSize-10;
      offsetY = 0;
      break;
    case 1:
      offsetX = 0;
      offsetY = -circleSize+10;
      break;
    case 2:
      offsetX = -circleSize+10;
      offsetY = 0;
      break;
    case 3:
      offsetX = 0;
      offsetY = circleSize-10;
      break;
    default:
      offsetX = circleSize-10;
      offsetY = 0;
      break;
  }

  canvas->fillScreen(ARCADA_BLACK);

  // render a filled circle and then a smaller filled circle to get a border of chosen thickness
  if ((int32_t)(beatTime - timeNow) > 0) {
    canvas->fillCircle(ARCADA_TFT_WIDTH/2+offsetX, ARCADA_TFT_HEIGHT/2+offsetY, circleSize, ARCADA_BLUE);
  } else {
    canvas->fillCircle(ARCADA_TFT_WIDTH/2+offsetX, ARCADA_TFT_HEIGHT/2+offsetY, circleSize, ARCADA_BLUE);
  }
  canvas->fillCircle(ARCADA_TFT_WIDTH/2+offsetX, ARCADA_TFT_HEIGHT/2+offsetY, circleSize-5, ARCADA_BLACK);

  // render indicator circle if needed
  if (timeNow - beatTime > 0 && timeNow - beatTime < 100) {
    if (buttonPressedForLastBeat) {
      if (abs(timingOffset) < 50) {
        // good
        canvas->fillCircle(ARCADA_TFT_WIDTH/2, ARCADA_TFT_HEIGHT/2, 10, ARCADA_GREEN);
      } else if (abs(timingOffset) < 90) {
        // barely ok
        canvas->fillCircle(ARCADA_TFT_WIDTH/2, ARCADA_TFT_HEIGHT/2, 10, ARCADA_YELLOW);
      } else {
        // bad
        canvas->fillCircle(ARCADA_TFT_WIDTH/2, ARCADA_TFT_HEIGHT/2, 10, ARCADA_RED);
      }
    } else {
      // missed a beat and hit the button before the next beat occured
      canvas->fillCircle(ARCADA_TFT_WIDTH/2, ARCADA_TFT_HEIGHT/2, 10, ARCADA_RED);
    }
  } else {
    canvas->fillCircle(ARCADA_TFT_WIDTH/2, ARCADA_TFT_HEIGHT/2, 10, ARCADA_BLUE);
  }
}


/*
 * Prepares the PyBadge to play music
 */
void loadMusic() {
  // open the first wav file in the music folder
  musicFile = arcada.openFileByIndex("/music", 0, O_READ, "wav");
  if (!musicFile) {
    errorReporter("Fatal error: No sound file present.");
  }

  Serial.println("Loading sound file...");
  
  uint32_t sampleRate;

  // load wav information, enable speakers and set interrupt for sampling
  wavStatus status = arcada.WavLoad(musicFile, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    arcada.enableSpeaker(true);
    // create own interrupt
    arcada.timerCallback(sampleRate, timedInterruptCallback);
  } else {
    errorReporter("Fatal error: Sound file corrupted.");
  }
}


/*
 * Minimal error reporter (adapted from Adafruit examples - without the LED blinking stuff)
 */
void errorReporter(const char *error) {
  Serial.begin(9600);
  Serial.println(error);

  // busy wait to keep USB connections, etc. running
  while (true) {
    yield();
  }
}


/*
 * Interrupt for music sampling and game updates (called with 8kHz on our wav file - other sampling rates might not work)
 */
void timedInterruptCallback(void) {
  // we just entered the playing state
  // move first beat until music actually starts
  if (!musicIsPlaying) {
    beatTime = millis() + bpmTiming;
  }

  // play next sample
  wavStatus status = arcada.WavPlayNextSample();

  // update timing information
  uint32_t timeNow = millis();

  // if needed prepare calculations for next beat
  // called when beat missed
  if (timeNow >= beatTime + (bpmTiming>>1)) {
    beatTime = beatTime + bpmTiming;
    buttonPressedForLastBeat = false;
    beatDirection = directionOffset;
    directionUpdate = true;
  }

  // make sure that circle expansion direction changes according to beats
  if (directionUpdate && timeNow >= beatTime) {

    if (easyMode) {
      directionOffset = directionOffset + 1;
    } else {
      directionOffset = random(4);
    }
    
    if (directionOffset > 3) {
      directionOffset = 0;
    }
    
    directionUpdate = false;
  }

  // handle buttons
  uint8_t buttonState = arcada.readButtons();

  // a button always registers beat tap
  if (buttonState & ARCADA_BUTTONMASK_A) {
    if (!buttonPressedForLastBeat && buttonReleasedForLastBeat) {
      timingOffset = beatTime - timeNow;
      buttonPressedForLastBeat = true;
      buttonReleasedForLastBeat = false;
    }
  }

  // direction pad only registers beat tap in the right direction
  if (!buttonPressedForLastBeat && buttonReleasedForLastBeat && buttonState) {
    switch (beatDirection){
      case 0:
        if (buttonState & ARCADA_BUTTONMASK_RIGHT) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 1:
        if (buttonState & ARCADA_BUTTONMASK_UP) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 2:
        if (buttonState & ARCADA_BUTTONMASK_LEFT) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 3:
        if (buttonState & ARCADA_BUTTONMASK_DOWN) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      default:
        break;
    }

    buttonPressedForLastBeat = true;
    buttonReleasedForLastBeat = false;
  }

  // update playing state and hardware depending on wav playing status
  if (status == WAV_EOF) {
    // end interrupt
    arcada.timerStop();
    arcada.enableSpeaker(false);
    musicIsPlaying = false;
    inMenu = true;
    // close file handle
    musicFile.close();
  } else {
    musicIsPlaying = true;
  }
}
