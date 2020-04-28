// constants

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

#define LIKE_INDICATION_LIKED_INPUT_CODE 101
#define LIKE_INDICATION_DISLIKED_INPUT_CODE 102

#define BUTTON_TAP_THRESHOLD 300

// controls data

int lastClickedButtonPin = 0;
unsigned int lastClickedButtonTimestamp = 0;

boolean playButtonUp = true;
int playButtonClicksCount = 0;
boolean likeButtonUp = true;
int likeButtonClicksCount = 0;
int prevPotValue = 0;

// ui data

boolean likeIndicatorValue = false;
String serialInputAcc;

// life cycle

void setup() {
  pinMode(VOLUME_POT_PIN, INPUT);
  pinMode(PLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LIKE_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  // controls
  handlePotChanges(VOLUME_POT_PIN, &prevPotValue, VOLUME_POT_VALUE_OUTPUT_START_CODE, VOLUME_POT_VALUE_OUTPUT_END_CODE);
  handleButtonClicks(PLAY_BUTTON_PIN, &playButtonUp, &playButtonClicksCount);
  handleButtonClicks(LIKE_BUTTON_PIN, &likeButtonUp, &likeButtonClicksCount);

  checkButtonPendingClicks(PLAY_BUTTON_PIN, &playButtonClicksCount);
  checkButtonPendingClicks(LIKE_BUTTON_PIN, &likeButtonClicksCount);

  //  ui
  updateLikeIndicatorState();
}

// handle ui data

void updateLikeIndicatorState() {

  while (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar >= '0' && incomingChar <= '9' && serialInputAcc.length() <= 3) {
      serialInputAcc += incomingChar;
    } else {
      serialInputAcc = "";
    }
  }

  if (serialInputAcc.length() != 3) {
    return;
  }

  boolean needToIndicateLiked = serialInputAcc == String(LIKE_INDICATION_LIKED_INPUT_CODE);
  boolean needToIndicateDisliked = serialInputAcc == String(LIKE_INDICATION_DISLIKED_INPUT_CODE);

  serialInputAcc = "";

  boolean newLikeIndicatorValue = likeIndicatorValue;
  if (needToIndicateLiked) {
    newLikeIndicatorValue = true;
  }
  if (needToIndicateDisliked) {
    newLikeIndicatorValue = false;
  }

  if (likeIndicatorValue == newLikeIndicatorValue) {
    return;
  }

  likeIndicatorValue = newLikeIndicatorValue;
  if (likeIndicatorValue) {
    //    analogWrite(LIKE_LED_PIN, 64);
  } else {
    //    digitalWrite(LIKE_LED_PIN, LOW);
  }
}

// handle controls data

void handlePotChanges(int pin, int *prevValue, int startCode, int endCode) {
  int value = analogRead(VOLUME_POT_PIN);
  int threshold = 10;
  if (abs(value - *prevValue) > threshold) {
    int outputCode = map(value, 0, 1023, startCode, endCode);
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
