const int redInputPin = A0;
const int greenInputPin = A1;
const int blueInputPin = A2;

const int redPin = 11;
const int greenPin = 10;
const int bluePin = 9;

const int minAnalogValue = 0;
const int maxAnalogValue = 1023;
const int minLedValue = 0;
const int maxLedValue = 255;
const int baudRate = 9600;

int redInputValue;
int greenInputValue;
int blueInputValue;

void setup() {
  initializePins();
  Serial.begin(baudRate);
}

void loop() {
  readInputValues();
  mapInputValuesToLedValuesRange();
  writePinValues();
}

void initializePins() {
  pinMode(redInputPin, INPUT);
  pinMode(greenInputPin, INPUT);
  pinMode(blueInputPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void readInputValues() {
  redInputValue = analogRead(redInputPin);
  greenInputValue = analogRead(greenInputPin);
  blueInputValue = analogRead(blueInputPin);
}

void mapInputValuesToLedValuesRange() {
  redInputValue = map(redInputValue, minAnalogValue, maxAnalogValue, minLedValue, maxLedValue);
  greenInputValue = map(greenInputValue, minAnalogValue, maxAnalogValue, minLedValue, maxLedValue);
  blueInputValue = map(blueInputValue, minAnalogValue, maxAnalogValue, minLedValue, maxLedValue);
}

void writePinValues() {
  analogWrite(redPin, redInputValue);
  analogWrite(greenPin, greenInputValue);
  analogWrite(bluePin, blueInputValue);
}
