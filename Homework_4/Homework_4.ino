// Joystick declarations
const int pinSW = 2;
const int pinX = A1;
const int pinY = A0;

int xAxisValue = 0;
int yAxisValue = 0;
int buttonState = LOW;
bool joyMoved = false;
int minThreshold = 300;
int maxThreshold = 800;

// Shift register
const int latchPin = 11; // STCP to 12 on Shift Register
const int clockPin = 10; // SHCP to 11 on Shift Register
const int dataPin = 12; // DS to 14 on Shift Register

// Segments
const int displayCount = 4;
const int defaultDigitEncoding = B11111100;

const int segD1 = 7;
const int segD2 = 6;
const int segD3 = 5;
const int segD4 = 4;

int displayDigits[displayCount] = {
  segD1, segD2, segD3, segD4
};
int displayDigitsEncodings[displayCount] = {
  defaultDigitEncoding, defaultDigitEncoding, defaultDigitEncoding, defaultDigitEncoding
};

// { LEFT, RIGHT }
int movements[displayCount][2] = {
  { 1, 0 },
  { 1, -1 },
  { 1, -1 },
  { 0, -1 }
};

byte moveLeftIndex = 0;
byte moveRightIndex = 1;

// Encondings
const int encodingsNumber = 16;

int byteEncodings[encodingsNumber] = {
  B11111100, // 0 
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110  // F
};

// Blinking DP
const int blinkingToggleInterval = 200;
unsigned long lastBlinkingToggleTime;
byte isDecimalPointActive =  0;

// Button debouncing
unsigned long lastDebounceTime = 0;
unsigned long lastButtonStateChanges;
const int debounceDelay = 50;
const int shortButtonPressingTime = 50;
const int longButtonPressingTime = 1000;
bool wasLongPressingCompleted = false;

byte buttonReading;
byte lastButtonReading;

// State
byte firstState = 1;
byte secondState = 2;
byte currentState;
byte currentDisplayIndex;
byte currentDisplayEncoding;
byte currentEncodingIndex;

  
void setup() {
  initialize();  
  Serial.begin(9600);
}

void loop() {
  renderDigits();
  readValuesFromJoystick();
  checkButtonState();

  if (currentState == firstState) {
    loopFirstState();
  }
} 

void initialize() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(pinSW, INPUT_PULLUP);

  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  initializeDefaultState();
}

void initializeDefaultState() {
  currentState = firstState;
  currentDisplayIndex = 0;
  currentDisplayEncoding = defaultDigitEncoding;
  isDecimalPointActive = 0;

  for (int i = 0; i < displayCount; i++) {
    displayDigitsEncodings[i] = defaultDigitEncoding;
  }
}

void initializeFirstState() {
  currentState = firstState;
  currentDisplayEncoding = displayDigitsEncodings[currentDisplayIndex];
  isDecimalPointActive = 0;
}

void initializeSecondState() {
  currentState = secondState;
  currentDisplayEncoding = displayDigitsEncodings[currentDisplayIndex];
  
  for(int i = 0; i < encodingsNumber; i++) {
    if (byteEncodings[i] == currentDisplayEncoding) {
      currentEncodingIndex = i;
    }
  }
}

void loopFirstState() {
  checkBlinking();
}

void checkBlinking() {
  if (millis() - lastBlinkingToggleTime >= blinkingToggleInterval) {
    lastBlinkingToggleTime = millis();

    if (isDecimalPointActive) {
      currentDisplayEncoding -= 1;
    } else {
      currentDisplayEncoding += 1;
    }

    isDecimalPointActive = !isDecimalPointActive;
  }
}

void renderDigits() {
  for (int i = 0; i < displayCount; i++) {
    activateDisplay(i);

    if (currentState == secondState || i != currentDisplayIndex) {
      if (i != currentDisplayIndex) {
        writeReg(displayDigitsEncodings[i]);
      } else {
        writeReg(displayDigitsEncodings[i] + 1);
      }
    } else {
      writeReg(currentDisplayEncoding);
    }
    digitalWrite(displayDigits[i], LOW);
    delay(5);
  }
}

void activateDisplay(int displayNumber) {
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);      
  }
}

void writeReg(int digit) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, digit);
    digitalWrite(latchPin, HIGH);
}

void readValuesFromJoystick() {
  xAxisValue = analogRead(pinX);
  yAxisValue = analogRead(pinY);

  checkJoyMovement();
}

void checkJoyMovement() {
  int movementIndex = -1;

  if (currentState == firstState) {
    movementIndex = checkJoyMovementForFirstState();
  } else {
    checkJoyMovementForSecondState();
  }

  if (isJoyOnInitialPosition()) {
    joyMoved = !joyMoved;
  }

  if (movementIndex != -1) {
    joyMoved = !joyMoved;
    moveCurrentDisplay(movementIndex);
  }
}

int checkJoyMovementForFirstState() {
  if (joyMovedRight()) {
    return moveRightIndex;
  }
  if (joyMovedLeft()) {
    return moveLeftIndex;
  }

  return -1;
}

void checkJoyMovementForSecondState() {
  if (joyMovedUp() && currentEncodingIndex < encodingsNumber - 1) {
    joyMoved = !joyMoved;
    currentEncodingIndex += 1;
    displayDigitsEncodings[currentDisplayIndex] = byteEncodings[currentEncodingIndex];
  }
  if (joyMovedDown() && currentEncodingIndex > 0) {
    joyMoved = !joyMoved;
    currentEncodingIndex -= 1;
    displayDigitsEncodings[currentDisplayIndex] = byteEncodings[currentEncodingIndex];
  }
}

bool joyMovedLeft() {
  return !joyMoved && ((xAxisValue < minThreshold) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedRight() {
  return !joyMoved && ((xAxisValue > maxThreshold) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedUp() {
  return !joyMoved && ((yAxisValue > maxThreshold) && isValueBetweenThresholdValues(xAxisValue));
}

bool joyMovedDown() {
  return !joyMoved && ((yAxisValue < minThreshold) && isValueBetweenThresholdValues(xAxisValue));
}

bool isJoyOnInitialPosition() {
  return (joyMoved && isValueBetweenThresholdValues(xAxisValue) && isValueBetweenThresholdValues(yAxisValue));
}

bool isValueBetweenThresholdValues(int value) {
  return (value >= minThreshold && value <= maxThreshold);
}

bool meetsShortPressingRequirements() {
  return (((millis() - lastButtonStateChanges) >= shortButtonPressingTime) && ((millis() - lastButtonStateChanges) < longButtonPressingTime));
}

bool meetsLongPressingRequirements() {
  return (currentState == firstState && !wasLongPressingCompleted && buttonState == HIGH && (millis() - lastButtonStateChanges) >= longButtonPressingTime);  
}

void moveCurrentDisplay(int movementIndex) {
  int movement = movements[currentDisplayIndex][movementIndex];

  if (movement != 0) {
    currentDisplayIndex = currentDisplayIndex + movement;
    currentDisplayEncoding = displayDigitsEncodings[currentDisplayIndex];
    isDecimalPointActive = 0;
    lastBlinkingToggleTime = millis(); 
  }
}

void checkButtonState() {
  buttonReading = !digitalRead(pinSW);
  Serial.println(buttonReading);

  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  lastButtonReading = buttonReading;

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonReading != buttonState) {
      if (buttonState == HIGH && buttonReading == LOW) {
        if (meetsShortPressingRequirements()) {
          toggleState();
        }
      }

      lastButtonStateChanges = millis();
      buttonState = buttonReading;
      wasLongPressingCompleted = false;
    } else if (meetsLongPressingRequirements()) {
      initializeDefaultState();
      wasLongPressingCompleted = !wasLongPressingCompleted;
    }
  }
}

void toggleState() {
  if (currentState == firstState) {
    initializeSecondState();
  } else {
    initializeFirstState();
  }
}