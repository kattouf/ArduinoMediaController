#define VOL_DOWN_BUTTON_PIN 13
#define VOL_UP_BUTTON_PIN 12
#define PREV_BUTTON_PIN 9
#define PLAY_BUTTON_PIN 8
#define NEXT_BUTTON_PIN 7

#define PREV_BUTTON_OUTPUT_CODE 101
#define PLAY_BUTTON_OUTPUT_CODE 102
#define NEXT_BUTTON_OUTPUT_CODE 103
#define VOL_DOWN_BUTTON_OUTPUT_CODE 104
#define VOL_UP_BUTTON_OUTPUT_CODE 105

boolean volDownButtonUp = true;
boolean volUpButtonUp = true;
boolean prevButtonUp = true;
boolean playButtonUp = true;
boolean nextButtonUp = true;


void setup() {
  pinMode(VOL_DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VOL_UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  handleButtonOnClicks(VOL_DOWN_BUTTON_PIN, &volDownButtonUp);
  handleButtonOnClicks(VOL_UP_BUTTON_PIN, &volUpButtonUp);
  handleButtonOnClicks(PREV_BUTTON_PIN, &prevButtonUp);
  handleButtonOnClicks(PLAY_BUTTON_PIN, &playButtonUp);
  handleButtonOnClicks(NEXT_BUTTON_PIN, &nextButtonUp);
}

// Common

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
    case VOL_DOWN_BUTTON_PIN:
      outputCode = VOL_DOWN_BUTTON_OUTPUT_CODE;
      break;
    case VOL_UP_BUTTON_PIN:
      outputCode = VOL_UP_BUTTON_OUTPUT_CODE;
      break;
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
