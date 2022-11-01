// Pins declaration
const int driversGreenPin = 7;
const int driversYellowPin = 4;
const int driversRedPin = 2;
const int peopleRedPin = 8;
const int peopleGreenPin = 9;
const int buttonPin = 10;
const int buzzerPin = 11;

/*
* State one: green light for cars, red light for people, no sound.
* State two: yellow for cars, red for peorple, no sound.
* State three: red for cars, green for people, beeping sound.
* State four: red for cars, blinking green for people, beeping sound faster than state three beeping.
*/
const byte stateOne = 1;
const byte stateTwo = 2;
const byte stateThree = 3;
const byte stateFour = 4;

// Duration of each state
const int stateOneDuration = 8000;
const int stateTwoDuration = 3000;
const int stateThreeDuration = 8000;
const int stateFourDuration = 4000;

// Buzzer
const int stateThreeBuzzerInterval = 1000;
const int stateThreeBuzzerFrequency = 1000;
const int stateFourBuzzerInterval = 400;
const int stateFourBuzzerFrequency = 1100;

unsigned long lastBuzzerToggleTime;
byte isBuzzerSoundOn = 0;

// Blinking green led
const int greenLedToggleInterval = 400;
unsigned long lastGreenLedToggleTime;
byte greenLedState = LOW;

// Debounce
unsigned long lastDebounceTime = 0;
const int debounceDelay = 50;
byte buttonReading;
byte lastButtonReading;

// Other constants
const int baudRate = 9600;

unsigned long lastStateChangesTime;
byte currentButtonState = 0;
byte currentState = stateOne;
byte isCycleInProgress = 0;

void setup() {
  initializePins();
  setStateOne();
  Serial.begin(baudRate);
}

void loop() {
  buttonReading = !digitalRead(buttonPin);

  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  lastButtonReading = buttonReading;

  if ((millis() - lastDebounceTime) >= debounceDelay) {
      if (buttonReading != currentButtonState) {
        currentButtonState = buttonReading;
      }
      if (currentButtonState == 1 && currentState == stateOne && !isCycleInProgress) {
        lastStateChangesTime = millis();
        isCycleInProgress = 1;
      }
  }

  if (currentState == stateOne && isFirstStateFinished()) {
    setStateTwo();
  }

  if (currentState == stateTwo && isSecondStateFinished()) {
    setStateThree();
  }

  if (currentState == stateThree && isThirdStateFinished()) {
    setStateFour();
  }

  if (currentState == stateFour && isFourthStateFinished()) {
    setStateOne();
    noTone(buzzerPin);
    isCycleInProgress = 0;
  }

  if (currentState == stateThree && (millis() - lastBuzzerToggleTime) >= stateThreeBuzzerInterval) {
    toggleBuzzerSound(stateThreeBuzzerFrequency, stateThreeBuzzerInterval);
  }

  if (currentState == stateFour) {
    if ((millis() - lastBuzzerToggleTime) >= stateFourBuzzerInterval) {
      toggleBuzzerSound(stateFourBuzzerFrequency, stateFourBuzzerInterval);
    }
    if (millis() - lastGreenLedToggleTime >= greenLedToggleInterval) {
      toggleGreenLedLight();
    }
  }
}

void initializePins() {
  pinMode(driversGreenPin, OUTPUT);
  pinMode(driversYellowPin, OUTPUT);
  pinMode(driversRedPin, OUTPUT);
  pinMode(peopleRedPin, OUTPUT);
  pinMode(peopleGreenPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

boolean isFirstStateFinished() {
  return (isCycleInProgress && (millis() - lastStateChangesTime) >= stateOneDuration);
}

boolean isSecondStateFinished() {
  return (millis() - lastStateChangesTime) >= stateTwoDuration;
}

boolean isThirdStateFinished() {
  return (millis() - lastStateChangesTime) >= stateThreeDuration;
}

boolean isFourthStateFinished() {
  return (millis() - lastStateChangesTime) >= stateFourDuration;
}
 
void setStateOne() {
  currentState = stateOne;
  digitalWrite(driversGreenPin, HIGH);
  digitalWrite(driversYellowPin, LOW);
  digitalWrite(driversRedPin, LOW);
  digitalWrite(peopleRedPin, HIGH);
  digitalWrite(peopleGreenPin, LOW);
}

void setStateTwo() {
  currentState = stateTwo;
  lastStateChangesTime = millis();
  digitalWrite(driversGreenPin, LOW);
  digitalWrite(driversYellowPin, HIGH);
}

void setStateThree() {
  currentState = stateThree;
  lastStateChangesTime = millis();
  digitalWrite(driversYellowPin, LOW);
  digitalWrite(driversRedPin, HIGH);  
  digitalWrite(peopleRedPin, LOW);
  digitalWrite(peopleGreenPin, HIGH);
  toggleBuzzerSound(stateThreeBuzzerFrequency, stateThreeBuzzerInterval);
}

void setStateFour() {
  currentState = stateFour;
  lastStateChangesTime = millis();
  toggleGreenLedLight();
  toggleBuzzerSound(stateFourBuzzerFrequency, stateFourBuzzerInterval);
}

void toggleBuzzerSound(int frequency, int duration) {
  lastBuzzerToggleTime = millis();

  if (!isBuzzerSoundOn) {
    tone(buzzerPin, frequency, duration);
  } else {
    noTone(buzzerPin);
  }
  isBuzzerSoundOn = !isBuzzerSoundOn;
}

void toggleGreenLedLight() {
  lastGreenLedToggleTime = millis();
  greenLedState = !greenLedState;
  digitalWrite(peopleGreenPin, greenLedState);
}