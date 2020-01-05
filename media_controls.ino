#define VOLUME_POT_PIN A0
#define PREV_BUTTON_PIN 9
#define PLAY_BUTTON_PIN 8
#define NEXT_BUTTON_PIN 7

#define PREV_BUTTON_OUTPUT_CODE 101
#define PLAY_BUTTON_OUTPUT_CODE 102
#define NEXT_BUTTON_OUTPUT_CODE 103

#define VOLUME_POT_VALUE_OUTPUT_START_CODE 200
#define VOLUME_POT_VALUE_OUTPUT_END_CODE 299

int prevPotValue = 0;
boolean prevButtonUp = true;
boolean playButtonUp = true;
boolean nextButtonUp = true;

void setup() {
  pinMode(PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  handlePotOnChanges(VOLUME_POT_PIN, &prevPotValue, VOLUME_POT_VALUE_OUTPUT_START_CODE, VOLUME_POT_VALUE_OUTPUT_END_CODE);
  handleButtonOnClicks(PREV_BUTTON_PIN, &prevButtonUp);
  handleButtonOnClicks(PLAY_BUTTON_PIN, &playButtonUp);
  handleButtonOnClicks(NEXT_BUTTON_PIN, &nextButtonUp);
}

// Common

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
    default:
      outputCode = -1;
  }

  Serial.print(outputCode);
}
