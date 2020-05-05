#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // RS, E, D4, D5, D6, D7

// ui constants

byte heartChar[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};
#define HEART_CHAR_ID 0

byte emptyHeartChar[8] = {
  B00000,
  B01010,
  B10101,
  B10001,
  B10001,
  B01010,
  B00100,
  B00000
};
#define EMPTY_HEART_CHAR_ID 1

byte pauseChar[8] = {
  B00000,
  B00000,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011
};
#define PAUSE_CHAR_ID 2

byte equalizerState1[8] = {
  B00001,
  B00001,
  B00001,
  B00101,
  B10101,
  B11101,
  B11111,
  B11111
};
byte equalizerState2[8] = {
  B00000,
  B00000,
  B00010,
  B11011,
  B11011,
  B11011,
  B11011,
  B11111
};
byte equalizerState3[8] = {
  B00010,
  B00010,
  B10010,
  B10010,
  B11011,
  B11011,
  B11111,
  B11111
};
byte equalizerState4[8] = {
  B00000,
  B00000,
  B01010,
  B01110,
  B11110,
  B11110,
  B11111,
  B11111
};
byte equalizerState5[8] = {
  B00000,
  B00000,
  B00000,
  B01010,
  B11110,
  B11111,
  B11111,
  B11111
};
byte *equalizerStates[8] = { equalizerState1, equalizerState2, equalizerState3, equalizerState4, equalizerState5  };
#define EQUALIZER_FIRST_STATE_CHAR_ID 3
#define EQUALIZER_LAST_STATE_CHAR_ID 7

#define EQUALIZER_REFRESH_PERIOD 200

#define TEXT_SHIFT_DELAY 3000
#define TEXT_SHIFT_PERIOD 300
#define TEXT_MAX_SIZE 14

#define IS_LIKED_UI_DATA_TAG 101
#define TRACK_INFO_UI_DATA_TAG 102
#define IS_PLAYING_UI_DATA_TAG 103

const String TRACK_INFO_DELIMETER = "<~>";

// controls constants

#define VOLUME_POT_PIN A0
#define PLAY_BUTTON_PIN 8
#define LIKE_BUTTON_PIN 10

#define PREV_ACTION_CODE 101
#define PLAY_ACTION_CODE 102
#define NEXT_ACTION_CODE 103
#define VOLUME_UP_ACTION_CODE 104
#define VOLUME_DOWN_ACTION_CODE 105
#define LIKE_ACTION_CODE 106
#define VOLUME_POT_VALUE_OUTPUT_START_CODE 200
#define VOLUME_POT_VALUE_OUTPUT_END_CODE 299

#define BUTTON_TAP_THRESHOLD 300

// controls data

int lastClickedButtonPin = 0;
unsigned long lastClickedButtonTimestamp = 0;

boolean playButtonUp = true;
int playButtonClicksCount = 0;
boolean likeButtonUp = true;
int likeButtonClicksCount = 0;
int prevPotValue = 0;

// input data processing

boolean commandParsing = false;
String command = "";

boolean dataParsing = false;
String data = "";

// ui data

unsigned long equalizerLastUpdateTimestamp = 0;
byte equalizerCurrentState = EQUALIZER_FIRST_STATE_CHAR_ID;

boolean playingIndicatorValue = false;
boolean likeIndicatorValue = false;

unsigned long textLastShiftTimestamp = 0;
String title = "";
String artist = "";
int titleOffset = 0;
int artistOffset = 0;

// life cycle

void setup() {
  // controls
  pinMode(VOLUME_POT_PIN, INPUT);
  pinMode(PLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LIKE_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  // ui
  lcd.begin(16, 2);

  lcd.createChar(HEART_CHAR_ID, heartChar);
  lcd.createChar(EMPTY_HEART_CHAR_ID, emptyHeartChar);
  lcd.createChar(PAUSE_CHAR_ID, pauseChar);

  for (int i = 0; i <= EQUALIZER_LAST_STATE_CHAR_ID - EQUALIZER_FIRST_STATE_CHAR_ID; i++) {
    lcd.createChar(EQUALIZER_FIRST_STATE_CHAR_ID + i, equalizerStates[i]);
  }

  renderInitialData();
}

void loop() {
  // controls
  handlePotChanges(VOLUME_POT_PIN, &prevPotValue, VOLUME_POT_VALUE_OUTPUT_START_CODE, VOLUME_POT_VALUE_OUTPUT_END_CODE);
  handleButtonClicks(PLAY_BUTTON_PIN, &playButtonUp, &playButtonClicksCount);
  handleButtonClicks(LIKE_BUTTON_PIN, &likeButtonUp, &likeButtonClicksCount);

  checkButtonPendingClicks(PLAY_BUTTON_PIN, &playButtonClicksCount);
  checkButtonPendingClicks(LIKE_BUTTON_PIN, &likeButtonClicksCount);

  // ui
  parseUICommand();
  parseUIData();
  invokeUICommand();

  updateUIState();
}

// read and render ui data

void parseUICommand() {
  if (Serial.available() <= 0) {
    return;
  }

  char incomingChar = commandParsing ? Serial.read() : Serial.peek();

  if (!commandParsing) {
    if (incomingChar == '[') {
      Serial.read();
      command = "";
      commandParsing = true;
    }
    return;
  }

  if (incomingChar == ']') {
    commandParsing = false;
    return;
  }

  if (incomingChar >= '0' && incomingChar <= '9') {
    command += incomingChar;
  } else {
    command = "";
    commandParsing = false;
  }
}

void parseUIData() {
  if (commandParsing) {
    return;
  }

  if (Serial.available() <= 0) {
    return;
  }

  char incomingChar = dataParsing ? Serial.read() : Serial.peek();

  if (!dataParsing) {
    if (incomingChar == '{') {
      Serial.read();
      data = "";
      dataParsing = true;
    }
    return;
  }

  if (incomingChar == '}') {
    dataParsing = false;
    return;
  }

  data += incomingChar;
}

void invokeUICommand() {
  if (commandParsing || dataParsing) {
    return;
  }

  int commandNum = command.toInt();
  if (commandNum <= 0 || data.length() <= 0) {
    return;
  }

  switch (commandNum) {
    case IS_PLAYING_UI_DATA_TAG:
      if (data == "1" || data == "0") {
        boolean playing = data.toInt() == 1;
        renderIsPlaying(playing);
      }
      break;
    case IS_LIKED_UI_DATA_TAG:
      if (data == "1" || data == "0") {
        boolean liked = data.toInt() == 1;
        renderIsLiked(liked);
      }
      break;
    case TRACK_INFO_UI_DATA_TAG: {
        int delimeterIndex = data.indexOf(TRACK_INFO_DELIMETER);
        Serial.println(delimeterIndex);
        String newTitle = data.substring(0, delimeterIndex);
        String newArtist = data.substring(delimeterIndex + TRACK_INFO_DELIMETER.length(), data.length());
        renderTrackInfo(newTitle, newArtist);
      }
      break;
    default:
      break;
  }

  command = "";
  data = "";
}

void updateUIState() {
  shiftTextIfNeeded(title, 0, &titleOffset);
  shiftTextIfNeeded(artist, 1, &artistOffset);

  showNextEqualizerSymbolIfNeeded();
}

void renderInitialData() {
  renderText("MEDIA", 0);
  renderText("CONTROLLER", 1);

  lcd.setCursor(15, 0);
  int likeChar = likeIndicatorValue ? HEART_CHAR_ID : EMPTY_HEART_CHAR_ID;
  lcd.write(byte(likeChar));

  lcd.setCursor(15, 1);
  int playingChar = playingIndicatorValue ? EQUALIZER_FIRST_STATE_CHAR_ID : PAUSE_CHAR_ID;
  lcd.write(byte(playingChar));
}

void renderIsPlaying(boolean newValue) {
  if (playingIndicatorValue == newValue) {
    return;
  }

  playingIndicatorValue = newValue;
  lcd.setCursor(15, 1);
  int playingChar = playingIndicatorValue ? EQUALIZER_FIRST_STATE_CHAR_ID : PAUSE_CHAR_ID;
  lcd.write(byte(playingChar));
}

void renderIsLiked(boolean newValue) {
  if (likeIndicatorValue == newValue) {
    return;
  }

  likeIndicatorValue = newValue;
  lcd.setCursor(15, 0);
  int likeChar = likeIndicatorValue ? HEART_CHAR_ID : EMPTY_HEART_CHAR_ID;
  lcd.write(byte(likeChar));
}

void renderTrackInfo(String newTitle, String newArtist) {
  if (title == newTitle && artist == newArtist) {
    return;
  }

  title = newTitle;
  artist = newArtist;

  renderText(title, 0);
  renderText(artist, 1);

  resetTextShiftingState();
}

void renderText(String text, int line) {
  int textLength = text.length();
  for (int i = 0; i < TEXT_MAX_SIZE; i++) {
    lcd.setCursor(i, line);
    if (i < textLength) {
      lcd.write(text[i]);
    } else {
      lcd.write(' ');
    }
  }
}

void showNextEqualizerSymbolIfNeeded() {
  if (!playingIndicatorValue) {
    unsigned long equalizerLastUpdateTimestamp = 0;
    byte equalizerCurrentState = EQUALIZER_FIRST_STATE_CHAR_ID;
    return;
  }

  unsigned long currentTime = millis();
  if (currentTime - equalizerLastUpdateTimestamp > EQUALIZER_REFRESH_PERIOD) {
    byte nextState = equalizerCurrentState + 1;
    equalizerCurrentState = nextState > EQUALIZER_LAST_STATE_CHAR_ID ? EQUALIZER_FIRST_STATE_CHAR_ID : nextState;

    lcd.setCursor(15, 1);
    lcd.write(equalizerCurrentState);

    equalizerLastUpdateTimestamp = currentTime;
  }
}

void shiftTextIfNeeded(String text, int line, int *offset) {
  const int textLength = text.length();
  const int textOversize = textLength - TEXT_MAX_SIZE;

  if (textOversize <= 0) {
    return;
  }

  int period = (*offset == 0 || *offset == -textOversize) ? TEXT_SHIFT_DELAY : TEXT_SHIFT_PERIOD;

  unsigned long currentTime = millis();
  if (currentTime - textLastShiftTimestamp > period) {

    *offset = *offset - 1;
    if (*offset < -textOversize) {
      *offset = 0;
    }

    for (int i = 0; i < TEXT_MAX_SIZE; i++) {
      int shiftedIndex = i - *offset;
      lcd.setCursor(i, line);
      if (shiftedIndex >= 0 && shiftedIndex < textLength) {
        lcd.write(text[shiftedIndex]);
      } else {
        lcd.write(' ');
      }
    }

    textLastShiftTimestamp = currentTime;
  }
}

void resetTextShiftingState() {
  textLastShiftTimestamp = millis();
  titleOffset = 0;
  artistOffset = 0;
}

// handle controls data

void handlePotChanges(int pin, int *prevValue, int startCode, int endCode) {
  int value = analogRead(VOLUME_POT_PIN);
  int threshold = 10;

  if (abs(value - *prevValue) > threshold) {
    int outputCode = map(value, 100, 1000, startCode, endCode);
    Serial.print(outputCode);

    *prevValue = value;
  }
}

void handleButtonClicks(int pin, boolean *wasUp, int *clicksCount) {
  boolean isUp = digitalRead(pin);

  if (*wasUp && !isUp) {
    // handle "bounce" effect
    delay(10);

    isUp = digitalRead(pin);
    if (!isUp) {
      *clicksCount += 1;
      lastClickedButtonTimestamp = millis();
      lastClickedButtonPin = pin;
    }
  }

  *wasUp = isUp;
}

void checkButtonPendingClicks(int pin, int *clicksCount) {
  if (lastClickedButtonPin == pin && millis() - lastClickedButtonTimestamp < BUTTON_TAP_THRESHOLD) {
    return;
  }

  if (*clicksCount <= 0) {
    return;
  }

  generateOutputAction(pin, *clicksCount);
  *clicksCount = 0;
}

void generateOutputAction(int pin, int clicksCount) {
  int outputCode;

  switch (pin) {
    case PLAY_BUTTON_PIN:
      switch (clicksCount) {
        case 1:
          outputCode = PLAY_ACTION_CODE;
          break;
        case 2:
          outputCode = NEXT_ACTION_CODE;
          break;
        case 3:
          outputCode = PREV_ACTION_CODE;
          break;
        default:
          outputCode = -1;
      }
      break;
    case LIKE_BUTTON_PIN:
      switch (clicksCount) {
        case 1:
          outputCode = LIKE_ACTION_CODE;
          break;
        default:
          outputCode = -1;
      }
      break;
    default:
      outputCode = -1;
  }

  if (outputCode > 0) {
    Serial.print(outputCode);
  }
}
