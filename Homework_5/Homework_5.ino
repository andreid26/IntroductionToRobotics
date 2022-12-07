#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define MIN_THRESHOLD 300
#define MAX_THRESHOLD 900
#define DEBOUNCE_DELAY 50

#define LCD_WELCOME 0
#define LCD_MENU 1
#define LCD_START 2
#define LCD_HIGHSCORE 3
#define LCD_SETTINGS 4
#define LCD_INFO 5
#define LCD_HTP 6
#define LCD_GAMEOVER 7

#define LCD_BRIGHTNESS_ADDR 0
#define MATRIX_BRIGHTNESS_ADDR 1
#define SOUND_CONTROL_ADDR 2
#define HIGHSCORE_VALUE_ADDR 3
#define HIGHSCORE_NAME_ADDR_START 4
#define HIGHSCORE_NAME_ADDR_STOP 13

#define LOW_SPEED  220
#define MEDIUM_SPEED  180
#define HIGH_SPEED  120

struct Point {
  byte i;
  byte j;
};

// LCD declarations
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte brightnessPin = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte currentLcdDisplay = LCD_WELCOME;
byte currentMenuOption = LCD_START;

// Matrix declarations
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

byte matrix[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

// Joystick declarations
const byte pinSW = 2;
const byte pinX = A1;
const byte pinY = A0;

int xAxisValue = 0;
int yAxisValue = 0;
byte buttonState = LOW;
bool joyMoved = false;

const byte up = 1;
const byte down = 2;
const byte left = 3;
const byte right = 4;
byte currentDirection = 0;

// Button debouncing
unsigned long lastDebounceTime = 0;
unsigned long lastButtonStateChanges;

byte buttonReading;
byte lastButtonReading;

// How to play
String htpText = "The player uses a joystick to move the snake around. As the snake finds food,it eats the food and grows larger. The game ends when the snake either moves off the screen or moves into itself.";
byte textStartPosition = 0;

// EEPROM values
byte eepromLcdBrightness;
byte eepromMatrixBrightness;
byte eepromSoundControl;
byte eepromHighscoreValue;
String eepromHighscoreName;

// Game configurations
String settings[] = {
  "NAME:",
  "DIFFICULTY:",
  "SOUND:",
  "LCD BR:",
  "MAT BR:",
  "BACK"
};
byte settingsNameIndex = 0;
byte settingsDiffIndex = 1;
byte settingsSoundIndex = 2;
byte settingsLcdBrIndex = 3;
byte settingsMatBrIndex = 4;
byte settingsBackIndex = 5;
byte settingsCurrentIndex = settingsNameIndex;
byte isSettingsOptionLocked = 0;

byte settingsMatrixTurnedOn = 0;

// Global declarations
String name = "NEW";

byte isGameRunning = 0;
byte gameDifficulty = 0;
byte gameScore = 0;
byte gameLives = 3;
byte speed = LOW_SPEED;
byte foodEaten = 0;

Point currentPosition;
Point foodPosition;

unsigned long lastMoveTime = 0;


void initializePins() {
  Serial.begin(9600);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(brightnessPin, OUTPUT);
}

void readValuesFromEeprom() {
  EEPROM.get(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
  EEPROM.get(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
  EEPROM.get(SOUND_CONTROL_ADDR, eepromSoundControl);
  EEPROM.get(HIGHSCORE_VALUE_ADDR, eepromHighscoreValue);

  for (byte i = HIGHSCORE_NAME_ADDR_START; i <= HIGHSCORE_NAME_ADDR_STOP; i++) {
    byte charFromAddr;
    EEPROM.get(i, charFromAddr);

    if (charFromAddr) {
      eepromHighscoreName += char(charFromAddr);      
    }
  }  
}

void initializeLCD() {
  lcd.begin(16, 2);
  analogWrite(brightnessPin, eepromLcdBrightness);
  showDisplay();
}

void initializeMatrix() {
  lc.shutdown(0, false);
  lc.setIntensity(0, eepromMatrixBrightness);
  lc.clearDisplay(0);
}

void showDisplay() {
  lcd.clear();
  switch (currentLcdDisplay) {
    case LCD_WELCOME: {
      showWelcomeDisplay();
      break;
    }
    case LCD_MENU: {
      showMenuDisplay();
      break;
    }
    case LCD_START: {
      showRunningGameDisplay();
      break;
    }
    case LCD_HIGHSCORE: {
      showHighscoreDisplay();
      break;
    }
    case LCD_SETTINGS: {
      showSettingsDisplay();
      break;
    }
    case LCD_INFO: {
      showInfoDisplay();
      break;
    }
    case LCD_HTP: {
      textStartPosition = 0;
      showHTPDisplay();
      break;
    }
    case LCD_GAMEOVER: {
      showGameOverDisplay();
      break;
    }
  }
}

void showWelcomeDisplay() {
  lcd.setCursor(4, 0);
  lcd.print("WELCOME!");
  lcd.setCursor(0, 1);
  lcd.print("Press joy button");
}

void showMenuDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("------MENU------");
  lcd.setCursor(0, 1);
  lcd.print(getCurrentOptionMessage());
}

void showRunningGameDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("--RUNNING GAME--");
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(gameScore));
}

void showHighscoreDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("---HIGH SCORE---");
  lcd.setCursor(0, 1);
  lcd.print(eepromHighscoreName + " - " + eepromHighscoreValue);
}

void showSettingsDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("----SETTINGS----");
  lcd.setCursor(0, 1);

  String msg = settings[settingsCurrentIndex];

  if (settingsCurrentIndex == settingsNameIndex) msg += name;
  if (settingsCurrentIndex == settingsDiffIndex) msg += getDifficultyMessage();
  if (settingsCurrentIndex == settingsSoundIndex) msg += getSoundMessage();
  if (settingsCurrentIndex == settingsLcdBrIndex) msg += eepromLcdBrightness;
  if (settingsCurrentIndex == settingsMatBrIndex) msg += eepromMatrixBrightness;

  if (!isSettingsOptionLocked) {
    lcd.print("> " + msg);
  } else {
    lcd.print("* " + msg);
  }
}

void showInfoDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("------INFO------");
  lcd.setCursor(0, 1);
  lcd.print("GitHub:andreid26");
}

void showHTPDisplay() {
  lcd.setCursor(0, 0);
  lcd.print(htpText.substring(textStartPosition, textStartPosition + 16));
  lcd.setCursor(0, 1);
  lcd.print("     <    >     ");  
}

void showGameOverDisplay() {
  lcd.setCursor(0, 0);
  lcd.print("----GAMEOVER----");
  lcd.setCursor(0, 1);
  lcd.print("Your score: " + String(gameScore));
}

String getSoundMessage() {
  if (eepromSoundControl == 1) return "ON";

  return "OFF";
}

String getDifficultyMessage() {
  if (gameDifficulty == 0) return "L";
  if (gameDifficulty == 1) return "M";

  return "H";
}

String getCurrentOptionMessage() {
  switch (currentMenuOption) {
    case LCD_START: { return "> START"; break; }
    case LCD_HIGHSCORE: { return "> HIGHSCORE"; break; }
    case LCD_SETTINGS: { return "> SETTINGS"; break; }
    case LCD_INFO: { return "> INFO"; break; }
    case LCD_HTP: { return "> HOW TO PLAY"; break; }
    default: { return "> START"; break; }
  }
}

void renderMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      if (!settingsMatrixTurnedOn) {
        if (isGameRunning && (isSamePosition(currentPosition, row, col) || isSamePosition(foodPosition, row, col))) {
          lc.setLed(0, row, col, HIGH);
        } else {
          lc.setLed(0, row, col, matrix[row][col]);        
        }
      }
    }
  }
}

byte isSamePosition(Point position, byte row, byte col) {
  return row == position.i && col == position.j;
}

void loopJoystickFunctions() {
  readValuesFromJoystick();
  checkJoyMovement();
}

void readValuesFromJoystick() {
  readJoystickButton();

  xAxisValue = analogRead(pinX);
  yAxisValue = analogRead(pinY);
}

void checkJoyMovement() {
  if (joyMovedUp()) {
    handleJoyUpMovement();
  } else if (joyMovedDown()) {    
    handleJoyDownMovement();
  } else if (joyMovedLeft()) {
    handleJoyLeftMovement();
  } else if (joyMovedRight()) {
    handleJoyRightMovement();
  }

  if (isJoyOnInitialPosition()) {
    joyMoved = 0;
  }
}

void handleJoyUpMovement() {
  joyMoved = 1;
  byte modifyDisplay = 0;
  
  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_MENU && currentMenuOption > LCD_START) {
      currentMenuOption -= 1;
      modifyDisplay = 1;
    }

    if (currentLcdDisplay == LCD_SETTINGS) {
      modifyDisplay = handleUpMovementForSettings();
    }  
  } else {
    currentDirection = up;
  }

  if (modifyDisplay) {
    showDisplay();
  }
}

void handleJoyDownMovement() {
  joyMoved = 1;
  byte modifyDisplay = 0;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_MENU && currentMenuOption < LCD_HTP) {
      currentMenuOption += 1;
      modifyDisplay = 1;
    }

    if (currentLcdDisplay == LCD_SETTINGS) {
      modifyDisplay = handleDownMovementForSettings();
    }   
  } else {
    currentDirection = down;
  }

  if (modifyDisplay) {
    showDisplay();
  }
}

byte handleUpMovementForSettings() {
  if (!isSettingsOptionLocked && settingsCurrentIndex > settingsNameIndex) {
    settingsCurrentIndex -= 1;
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsDiffIndex && gameDifficulty < 3) {
    gameDifficulty += 1;
    return 1;
  }
      
  if (isSettingsOptionLocked && settingsCurrentIndex == settingsSoundIndex && eepromSoundControl == 0) {
    eepromSoundControl = 1;
    EEPROM.update(SOUND_CONTROL_ADDR, eepromSoundControl);
    return 1;        
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsLcdBrIndex && eepromLcdBrightness <= 240) {
    eepromLcdBrightness += 15;
    analogWrite(brightnessPin, eepromLcdBrightness);
    EEPROM.update(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsMatBrIndex && eepromMatrixBrightness < 15) {
    eepromMatrixBrightness += 1;
    lc.setIntensity(0, eepromMatrixBrightness);
    EEPROM.update(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
    return 1;
  }

  return 0;
}

byte handleDownMovementForSettings() {
  if (!isSettingsOptionLocked && settingsCurrentIndex < settingsBackIndex) {
    settingsCurrentIndex += 1;
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsDiffIndex && gameDifficulty > 0) {
    gameDifficulty -= 1;
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsSoundIndex && eepromSoundControl == 1) {
    eepromSoundControl = 0;
    EEPROM.update(SOUND_CONTROL_ADDR, eepromSoundControl);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsLcdBrIndex && eepromLcdBrightness >= 15) {
    eepromLcdBrightness -= 15;
    analogWrite(brightnessPin, eepromLcdBrightness);
    EEPROM.update(LCD_BRIGHTNESS_ADDR, eepromLcdBrightness);
    return 1;
  }

  if (isSettingsOptionLocked && settingsCurrentIndex == settingsMatBrIndex && eepromMatrixBrightness > 0) {
    eepromMatrixBrightness -= 1;
    lc.setIntensity(0, eepromMatrixBrightness);
    EEPROM.update(MATRIX_BRIGHTNESS_ADDR, eepromMatrixBrightness);
    return 1;
  }

  return 0;
}

void handleJoyLeftMovement() {
  joyMoved = 1;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_HTP && textStartPosition >= 5) {
      textStartPosition -= 5;
      showHTPDisplay();
    }
  } else {
    currentDirection = left;
  }
}

void handleJoyRightMovement() {
  joyMoved = 1;

  if (!isGameRunning) {
    if (currentLcdDisplay == LCD_HTP && textStartPosition <= 170) {
      textStartPosition += 5;
      showHTPDisplay();
    }
  } else {
    currentDirection = right;
  }
}

void readJoystickButton() {
  buttonReading = !digitalRead(pinSW);

  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  lastButtonReading = buttonReading;

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;

      if (buttonState == HIGH) {
        handleButtonPressing();
      }
    }
  }
}

void handleButtonPressing() {
  if (isGameRunning) return;

  if (currentLcdDisplay == LCD_WELCOME) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_MENU) {
    currentLcdDisplay = currentMenuOption;

    if (currentLcdDisplay == LCD_START) {
      startNewGame();
    }
    showDisplay();
  } else if (currentLcdDisplay == LCD_INFO) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_SETTINGS) {
    if (settingsCurrentIndex != settingsBackIndex) {
      isSettingsOptionLocked = !isSettingsOptionLocked;

      if (settingsCurrentIndex == settingsMatBrIndex) {
        if (isSettingsOptionLocked) {
          settingsMatrixTurnedOn = 1;
          turnOnMatrix();
        } else {
          settingsMatrixTurnedOn = 0;
          turnOffMatrix();
        }
      }
    } else {
      currentLcdDisplay = LCD_MENU;   
    }

    showDisplay();
  } else if (currentLcdDisplay == LCD_HIGHSCORE) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  } else if (currentLcdDisplay == LCD_HTP || currentLcdDisplay == LCD_GAMEOVER) {
    currentLcdDisplay = LCD_MENU;
    showDisplay();
  }
}

void turnOnMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, HIGH);
    }
  }
}

void turnOffMatrix() {
  for (byte row = 0; row < matrixSize; row++) {
    for (byte col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, LOW);
    }
  }
}

bool joyMovedLeft() {
  return !joyMoved && ((xAxisValue > MAX_THRESHOLD) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedRight() {
  return !joyMoved && ((xAxisValue < MIN_THRESHOLD) && isValueBetweenThresholdValues(yAxisValue));
}

bool joyMovedUp() {
  return !joyMoved && ((yAxisValue < MIN_THRESHOLD) && isValueBetweenThresholdValues(xAxisValue));
}

bool joyMovedDown() {
  return !joyMoved && ((yAxisValue > MAX_THRESHOLD) && isValueBetweenThresholdValues(xAxisValue));
}

bool isJoyOnInitialPosition() {
  return (joyMoved && isValueBetweenThresholdValues(xAxisValue) && isValueBetweenThresholdValues(yAxisValue));
}

bool isValueBetweenThresholdValues(int value) {
  return (value >= MIN_THRESHOLD && value <= MAX_THRESHOLD);
}

void readValuesFromSerial() {
  if (isSettingsOptionLocked && settingsCurrentIndex == settingsNameIndex) {
      String textFromSerial = Serial.readString();

      if (textFromSerial.length() > 0 && textFromSerial.length() <= 9) {
        name = textFromSerial;
        isSettingsOptionLocked = !isSettingsOptionLocked;
        showDisplay();
      }
  }
}

byte getSpeedBasedOnDifficulty() {
  if (gameDifficulty == 0) return LOW_SPEED;
  if (gameDifficulty == 1) return MEDIUM_SPEED;

  return HIGH_SPEED;
}

void resetGameData() {
  currentPosition.i = 4;
  currentPosition.j = 4;
  currentDirection = 0;

  gameScore = 0;
  gameLives = 3;

  generateRandomFood();
  foodEaten = 0;
}

void startNewGame() {
  resetGameData();
  isGameRunning = 1;
  speed = getSpeedBasedOnDifficulty();
}

byte generateRandomByte() {
  return random(7);
}

void generateRandomFood() {
  byte randomRow = generateRandomByte();
  byte randomCol = generateRandomByte();

  while (randomRow == currentPosition.i || randomRow == 0 || randomRow == 7) {
    randomRow = generateRandomByte();
  }
  while (randomCol == currentPosition.j || randomCol == 0 || randomCol == 7) {
    randomCol = generateRandomByte();
  }

  foodPosition.i = randomRow;
  foodPosition.j = randomCol;
}

void handleRunningGame() {
  if (isGameRunning && currentDirection != 0) {
    if (millis() - lastMoveTime >= speed) {
      lastMoveTime = millis();      
      movePointBasedOnDirection();

      if (currentPosition.i == foodPosition.i && currentPosition.j == foodPosition.j) {
        eatFood();
      }
    }
  }
}

void eatFood() {
  foodEaten += 1;
  if ((gameDifficulty == 0 && foodEaten >= 3) || (gameDifficulty == 1 && foodEaten >= 2) || (gameDifficulty == 2 && foodEaten >= 1)) {
    gameScore += 1;
    foodEaten = 0;
  }
  movePointBasedOnDirection();
        
  showDisplay();
  generateRandomFood();
}

void movePointBasedOnDirection() {
  switch (currentDirection) {
    case up: {
      moveUp();
      return;
    }
    case down: {
      moveDown();
      return;
    }
    case left: {
      moveLeft();
      return;
    }
    case right: {
      moveRight();
      return;
    }
  }
}

void gameOver() {
  isGameRunning = 0;
  currentLcdDisplay = LCD_GAMEOVER;
  showDisplay();

  if (gameScore > eepromHighscoreValue) {
    eepromHighscoreValue = gameScore;
    eepromHighscoreName = name;

    updateEepromHighscore();
  }
  resetGameData();
}

void updateEepromHighscore() {
  EEPROM.update(HIGHSCORE_VALUE_ADDR, eepromHighscoreValue);

  for (byte i = HIGHSCORE_NAME_ADDR_START; i <= HIGHSCORE_NAME_ADDR_STOP; i++) {
    EEPROM.update(i, 0);
  }
  for (byte i = 0; i < name.length(); i++) {
    EEPROM.update(HIGHSCORE_NAME_ADDR_START + i, byte(name[i]));
  }
}

void moveUp() {
  if (currentPosition.i > 0) {
    currentPosition.i -= 1;
  } else {
    gameOver(); 
  }
}

void moveDown() {
  if (currentPosition.i < 7) {
    currentPosition.i += 1;
  } else {
    gameOver();
  }
}

void moveLeft() {
  if (currentPosition.j > 0) {
    currentPosition.j -= 1;
  } else {
    gameOver();
  }
}

void moveRight() {
  if (currentPosition.j < 7) {
    currentPosition.j += 1;
  } else {
    gameOver();
  }
}

void setup() {
  initializePins();
  readValuesFromEeprom();
  initializeLCD();
  initializeMatrix();
}

void loop() {
  renderMatrix();
  loopJoystickFunctions();
  readValuesFromSerial();
  handleRunningGame();
}