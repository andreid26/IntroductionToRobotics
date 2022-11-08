// Joystick declarations
const int pinSW = 2;
const int pinX = A1;
const int pinY = A0;

int xAxisValue = 0;
int yAxisValue = 0;
int buttonState = LOW;

// Segments
const int pinA = 4;
const int pinB = 5;
const int pinC = 6;
const int pinD = 7;
const int pinE = 8;
const int pinF = 9;
const int pinG = 10;
const int pinDP = 11;

const int segSize = 8;

int segments[segSize] = { 
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};
byte segmentsState[segSize] = {
  LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW
};

// { UP, DOWN, LEFT, RIGHT }
int movements[segSize][4] = { 
  { 0, 6, 5, 1 },
  { -1, 5, 4, 0 },
  { 4, 1, 2, 5 },
  { 3, 0, 1, -1 },
  { 2, -1, 0, -2 },
  { -5, 1, 0, -4 },
  { -6, -3, 0, 0 },
  { 0, 0, -5, 0 }
};

byte moveUpIndex = 0;
byte moveDownIndex = 1;
byte moveLeftIndex = 2;
byte moveRightIndex = 3;

// Blinking segment
const int segmentToggleInterval = 200;
unsigned long lastSegmentToggleTime;

// Button debouncing
unsigned long lastDebounceTime = 0;
unsigned long lastButtonStateChanges;
const int debounceDelay = 50;
const int shortButtonPressingTime = 50;
const int longButtonPressingTime = 1000;
bool wasLongPressingCompleted = false;

byte buttonReading;
byte lastButtonReading;

// Workflow declarations
byte stateOne = 1;
byte stateTwo = 2;
byte currentState = stateOne;
byte currentSegmentState = LOW;
byte currentSegmentIndex = 7;

bool joyMoved = false;
int minThreshold = 300;
int maxThreshold = 900;

void setup() {
  initializePins();
  initializeDefaultState();
  Serial.begin(9600);
}

void loop() {
  if (currentState == stateOne) {
    checkSegmentBlinking();
  }
  readValuesFromJoystick();
  checkJoyMovement();
  checkButtonState();
  rerenderSegmentsState();
}

void initializePins() {
  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }
  pinMode(pinSW, INPUT_PULLUP);
}

void initializeDefaultState() {
  currentState = stateOne;
  for (int i = 0; i < segSize; i++) {
    segmentsState[i] = LOW;
  }
  currentSegmentIndex = 7;
  segmentsState[currentSegmentIndex] = currentSegmentState; // Decimal point  
}

void initializeStateOne() {
  currentState = stateOne;
}

void initializeStateTwo() {
  currentState = stateTwo;
  currentSegmentState = LOW;
  digitalWrite(segments[currentSegmentIndex], currentSegmentState);
}

void toggleState() {
  if (currentState == stateOne) {
    initializeStateTwo();
  } else {
    segmentsState[currentSegmentIndex] = currentSegmentState;
    initializeStateOne();
  }
}

void readValuesFromJoystick() {
  xAxisValue = analogRead(pinX);
  yAxisValue = analogRead(pinY);
}

void checkSegmentBlinking() {
  if (millis() - lastSegmentToggleTime >= segmentToggleInterval) {
    lastSegmentToggleTime = millis();
    currentSegmentState = !currentSegmentState;
    digitalWrite(segments[currentSegmentIndex], currentSegmentState);
  }
}

void checkButtonState() {
  buttonReading = !digitalRead(pinSW);

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
  return (currentState == stateOne && !wasLongPressingCompleted && buttonState == HIGH && (millis() - lastButtonStateChanges) >= longButtonPressingTime);  
}

void checkJoyMovement() {
  int movementIndex = -1;

  if (currentState == stateOne) {
    movementIndex = checkJoyMovementForStateOne();
  } else {
    checkJoyMovementForStateTwo();
  }

  if (isJoyOnInitialPosition()) {
    joyMoved = !joyMoved;
  }

  if (movementIndex != -1) {
    joyMoved = !joyMoved;
    moveCurrentSegment(movementIndex);
  }
}

int checkJoyMovementForStateOne() {
  if (joyMovedUp()) {
    return moveUpIndex;
  } else if (joyMovedDown()) {
    return moveDownIndex;
  } else if (joyMovedLeft()) {
    return moveLeftIndex;
  } else if (joyMovedRight()) {
    return moveRightIndex; 
  }
  return -1;
}

void checkJoyMovementForStateTwo() {
  if (joyMovedRight()) {
    joyMoved = !joyMoved;
    currentSegmentState = HIGH;
    digitalWrite(segments[currentSegmentIndex], currentSegmentState);
  } else if (joyMovedLeft()) {
    joyMoved = !joyMoved;
    currentSegmentState = LOW;
    digitalWrite(segments[currentSegmentIndex], currentSegmentState);
  } 
}

void setCurrentSegmentState(byte state) {
  segmentsState[currentSegmentIndex] = state;
}

void moveCurrentSegment(int movementIndex) {
  int movement = movements[currentSegmentIndex][movementIndex];

  if (movement != 0) {
    currentSegmentIndex = currentSegmentIndex + movement;
    digitalWrite(segments[currentSegmentIndex], HIGH);
    currentSegmentState = HIGH;
    lastSegmentToggleTime = millis();  
  }
}

void rerenderSegmentsState() {
  for (int i = 0; i < segSize; i++) {
    if (i != currentSegmentIndex) {
      digitalWrite(segments[i], segmentsState[i]);
    }
  }
}