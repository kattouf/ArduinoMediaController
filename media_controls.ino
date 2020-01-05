#define VOLUME_POT_PIN A0
#define PREV_BUTTON_PIN 9
#define PLAY_BUTTON_PIN 8
#define NEXT_BUTTON_PIN 7
#define LIKE_BUTTON_PIN 10
#define LIKE_LED_PIN 11

#define PREV_BUTTON_OUTPUT_CODE 101
#define PLAY_BUTTON_OUTPUT_CODE 102
#define NEXT_BUTTON_OUTPUT_CODE 103
#define VOLUME_UP_BUTTON_OUTPUT_CODE 104
#define VOLUME_DOWN_BUTTON_OUTPUT_CODE 105
#define LIKE_BUTTON_OUTPUT_CODE 106
#define VOLUME_POT_VALUE_OUTPUT_START_CODE 200
#define VOLUME_POT_VALUE_OUTPUT_END_CODE 299

#define LIKE_LED_INPUT_CODE 101

int prevPotValue = 0;
boolean prevButtonUp = true;
boolean playButtonUp = true;
boolean nextButtonUp = true;
boolean likeButtonUp = true;
boolean ledEnabled = false;
String serialInputAcc;

void setup() {
  pinMode(VOLUME_POT_PIN, INPUT);
  pinMode(PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LIKE_BUTTON_PIN, INPUT_PULLUP);

  pinMode(LIKE_LED_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  handlePotOnChanges(VOLUME_POT_PIN, &prevPotValue, VOLUME_POT_VALUE_OUTPUT_START_CODE, VOLUME_POT_VALUE_OUTPUT_END_CODE);
  handleButtonOnClicks(PREV_BUTTON_PIN, &prevButtonUp);
  handleButtonOnClicks(PLAY_BUTTON_PIN, &playButtonUp);
  handleButtonOnClicks(NEXT_BUTTON_PIN, &nextButtonUp);
  handleButtonOnClicks(LIKE_BUTTON_PIN, &likeButtonUp);

  updateLikeLEDState();
}

// Common

void updateLikeLEDState() {

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

  boolean needToToggleLEDState = serialInputAcc == String(LIKE_LED_INPUT_CODE);
  serialInputAcc = "";

  if (!needToToggleLEDState) {
    return;
  }

  ledEnabled = !ledEnabled;

  if (ledEnabled) {
    analogWrite(LIKE_LED_PIN, 64);
  } else {
    digitalWrite(LIKE_LED_PIN, LOW);
  }
}

void handlePotOnChanges(int pin, int *prevValue, int startCode, int endCode) {
  int value = analogRead(VOLUME_POT_PIN);
  int threshold = 10;
  if (abs(value - *prevValue) > threshold) {
    int outputCode = map(value, 0, 1023, startCode, endCode);
    Serial.print(outputCode);

    *prevValue = value;
  }
}

void handleButtonOnClicks(int pin, boolean *wasUp) {
  boolean isUp = digitalRead(pin);

  if (*wasUp && !isUp) {
    // handle "bounce" effect
    delay(10);

    isUp = digitalRead(pin);
    if (!isUp) {
      buttonClickHandler(pin);
    }
  }

  *wasUp = isUp;
}

void buttonClickHandler(int pin) {
  int outputCode;

  switch (pin) {
    case PREV_BUTTON_PIN:
      outputCode = PREV_BUTTON_OUTPUT_CODE;
      break;
    case PLAY_BUTTON_PIN:
      outputCode = PLAY_BUTTON_OUTPUT_CODE;
      break;
    case NEXT_BUTTON_PIN:
      outputCode = NEXT_BUTTON_OUTPUT_CODE;
      break;
    case LIKE_BUTTON_PIN:
      outputCode = LIKE_BUTTON_OUTPUT_CODE;
      break;
    default:
      outputCode = -1;
  }

  Serial.print(outputCode);
}
