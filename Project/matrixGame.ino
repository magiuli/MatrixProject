#include <EEPROM.h>
#include <LedControl.h>
#include <LiquidCrystal.h>
// DDefine driver pins
const byte dinPin = 12;   // pin 12 is connected to the MAX7219 pin 1
const byte clockPin = 9;  // pin 9 is connected to the CLK pin 13
const byte loadPin = 10;  // pin 10 is connected to LOAD pin 12

// Define LCD pins
const byte rs = 4;
const byte en = 8;
const byte v0 = 5;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 13;
const byte d7 = 2;

// Define joystick pins
const byte joystickXPin = A0;
const byte joystickYPin = A1;
const byte joystickButtonPin = A2;

// Define buzzer pin
const byte buzzerPin = 3;

// Initialize the LED matrix
LedControl ledMatrix = LedControl(dinPin, clockPin, loadPin, 1);  // DIN, CLK, LOAD, No. DRIVER

// Initialize lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Define joystick variables
short unsigned int joystickXValue;
short unsigned int joystickYValue;
bool joystickButtonState = HIGH;
bool buttonWasPressed = false;
bool joystickWasInResetPosition = true;
byte joystickDirection;

// Define joystick directions
enum Directions {
  standing = 0,
  up = 1,
  down = 2,
  left = 3,
  right = 4
};

// Define joystick thresholds and reference value
const short unsigned int joystickLowerLimit = 412;
const short unsigned int joystickUpperLimit = 612;
const short unsigned int joystickRestPosition = 512;

// Define debounce variables
byte reading = HIGH;
byte lastReading = HIGH;
unsigned long int lastDebounceTime = 0;
unsigned long int debounceDelay = 50;

// Define system settings
byte matrixBrightness;
byte lcdBrightness;
byte soundOn = true;

// Define LCD brightness thresholds and increment
const byte lcdMinBrightness = 80;
const byte lcdMaxBrightness = 180;
const byte lcdBrightnessIncrement = 5;

// Define matrix brightness thresholds and increment
const byte matrivMinBrightness = 1;
const byte matrixMaxBrightness = 15;
const byte matrixBrightnessIncrement = 1;

// Define the map and game variables
const byte mapSize = 16;
const byte visibleSize = 8;
const byte halfScreen = visibleSize / 2;
const float coverRate = 0.4;
const short int maxWallCount = 5;  //floor((mapSize - 1) * (mapSize - 1) * coverRate);
byte gameMap[mapSize][mapSize] = {};
short int wallCount = 0;
short int x, y;

const byte maxLifesCount = 1;
short int score = 0;

// Define variables for camera movement
byte thisRow, thisColumn, leftEdge, rightEdge, topEdge, bottomEdge;

// Define game objects and their animation durations
const byte objectCount = 6;
const short unsigned int animationDuration[objectCount] = { 1, 1, 1000, 200, 1, 1 };
const short unsigned int lightOnDuration[objectCount] = { 0, 1, 500, 100, 1, 1 };

enum ObjectsId {
  empty = 0,
  wall = 1,
  human = 2,
  bomb = 3,
  border = 4,
  explosion = 5
};

const short int welcomeMessageDuration = 1000;

// Define system modes
bool menuMode = true;
bool enterNameMode = false;
bool gameIsOn = false;
bool endScreen = false;

// Define leadeardboard variables
const byte highScoreCount = 3;
short int highScore[highScoreCount] = { 1000, 500, 200 };
const byte maxNameLength = 7;
char names[highScoreCount][maxNameLength] = { "Test1", "Test2", "Test3" };

// Define EEPROM addresses
const byte lcdBrightnessAddress = 0;
const byte matrixBrightnessAddress = 1;
const byte soundOnAddress = 2;
const byte highScoreAdress[highScoreCount] = { 3, 5, 7 };
const byte nameAdresses[highScoreCount] = { 10, 20, 30 };

// Define matrix images
const uint64_t images[] PROGMEM = {
  0xffffffffffffffff,  // fullScreen
  0x07083c665a5a663c,  // bomb image
  0x7e7e7e7e3c18183c,  // trophy image
  0x781898f8fc0e0703,  // wrench image
  0x1818001818181818,  // "i"(info) image
  0x7e7e660e1c180018,  // question mark image
  0x000066660000423c,  // smiley face image
  0x0000666600003c42   // sad face image
};

enum Images {
  fullScreen = 0,
  startGameImage = 1,
  highScoreImage = 2,
  settingsImage = 3,
  aboutImage = 4,
  howToPlayImage = 5,
  smileyFace = 6,
  sadFace = 7
};

const int imageCount = sizeof(images) / sizeof(uint64_t);

/*
  Define the main menu and the submenus with their options
  The options are defined as enums and the current option is stored in a variable
  The current menu is also stored in a variable
  The righcell and leftcell variables are used to print the arrows on the LCD to help the user navigate the submenus
*/

const byte optionsCount = 5;
enum Options {
  startGame = 0,
  highScores = 1,
  settings = 2,
  about = 3,
  howToPlay = 4,
  mainMenu = 5
} currentMenu = mainMenu;
byte currentOption = 0;

const byte settingsCount = 5;
enum Settings {
  changeLCDBrightness = 0,
  changeMatrixBrightness = 1,
  soundControl = 2,
  resetHighScore = 3,
  back = 4
};

byte currentSetting = changeLCDBrightness;
byte settingsRightCell = 13;
byte settingsLeftCell = 2;

const byte aboutSectionsCount = 4;
enum AboutSections {
  githubUser = 0,
  authorName = 1,
  gameName = 2,
  goBack = 3
};

byte currentAboutSection = githubUser;
byte aboutLeftCell = 3;
byte aboutRightCell = 11;

const byte howToPlaySectionsCount = 2;
enum HowToPlaySections {
  gameInstructions = 0,
  goBackHTP = 1  // HTP = How To Play
};

byte currentHowToPlaySection = gameInstructions;
const byte howToPlayLeftCell = 0;
const byte howToPlayRightCell = 14;

const byte highScoreSectionsCount = 2;
enum HighScoreSections {
  showHighScore = 0,
  goBackHS = 1  // HS = High Score
};

byte currentHighScoreSection = showHighScore;
const byte highScoreLeftCell = 1;
const byte highScoreRightCell = 14;

byte currentHighScore = 0;

// Define the text to be displayed in the author section of the about menu
const char authorNameMessage[] PROGMEM = "Author: Matache Iulian Gabriel";
byte authorNameMessageLength = sizeof(authorNameMessage) / sizeof(char);

// Define the text to be displayed in the instructions section of the how to play menu
const char instrunctions[] PROGMEM = "Destroy all the walls to win.";
byte instrunctionsLength = sizeof(instrunctions) / sizeof(char);

// Define cursor variables for the about and how to play menus
byte currentAboutCharacter = 0;
byte currentInstructionCharacter = 0;

// Variable used for retrieving the previous value of a setting in case the new value is not valid
byte backup;

// Variable used for updating the display
bool displayWasUpdated = true;

// Variable used for incrementing the value of a setting
unsigned long int lastIncrementTime = 0;
const short int incrementDelay = 200;

// A position is valid if it is inside the map and it is an empty tile
bool positionIsValid(byte x, byte y) {
  return (x >= 0 && x < mapSize && y >= 0 && y < mapSize && gameMap[x][y] == empty);
}

// Define the player class which manages the player's position, lifes and bombs
class Player {
private:
  // Define the player's position and the movement delay
  byte xPosition;
  byte yPosition;
  unsigned long int lastMoveTime = 0;
  const short int moveDelay = 300;

  // Define the player's lifes
  byte lifes;

  // Define the bomb variables
  bool bombWasPlaced = false;
  byte bombX = 0;
  byte bombY = 0;
  unsigned long int bombStartTime = 0;
  unsigned long int bombExplosionTime = 0;

  // Function for updating a position in the map if the "tile" is destroyable (aka not border)
  void destroy(byte targetX, byte targetY, byte ObjectsId) {
    if (gameMap[targetX][targetY] == border) {
      return;
    }
    gameMap[targetX][targetY] = ObjectsId;
  }

public:
  // Define the bomb duration and the dificulty increase delay
  short int bombDuration = 1500;

  Player(int xCoordonate = 0, int yCoordonate = 0, int lifesCount = maxLifesCount) {
    xPosition = xCoordonate;
    yPosition = yCoordonate;
    lifes = lifesCount;
  }

  void spawnPlayer() {
    // Reset the player's lifes and bomb duration
    lifes = maxLifesCount;
    bombDuration = 1500;
    // Search for an empty spot in the map and spawn the player there
    for (int thisRow = 0; thisRow < visibleSize; thisRow++) {
      for (int thisColumn = 0; thisColumn < visibleSize; thisColumn++) {
        if (gameMap[thisRow][thisColumn] == 0) {
          xPosition = thisRow;
          yPosition = thisColumn;
          gameMap[xPosition][yPosition] = human;
          return;
        }
      }
    }
  }
  // Define getters for the player's position and lifes
  byte getX() {
    return xPosition;
  }

  byte getY() {
    return yPosition;
  }

  byte getLives() {
    return lifes;
  }

  // Define the function for moving the player
  void movePLayer() {
    // Return if the movement delay has not passed
    if (millis() - lastMoveTime < moveDelay) {
      return;
    }

    // Found by trial and error
    // Move the player in the desired direction if the position is valid
    switch (joystickDirection) {
      case up:
        if (positionIsValid(xPosition + 1, yPosition)) {
          gameMap[xPosition][yPosition] = empty;
          xPosition++;
          gameMap[xPosition][yPosition] = human;
          myBuzz(movement);
          soundStarted = true;
        }
        break;
      case down:
        if (positionIsValid(xPosition - 1, yPosition)) {
          gameMap[xPosition][yPosition] = empty;
          xPosition--;
          gameMap[xPosition][yPosition] = human;
          myBuzz(movement);
          soundStarted = true;
        }
        break;
      case left:
        if (positionIsValid(xPosition, yPosition + 1)) {
          gameMap[xPosition][yPosition] = empty;
          yPosition++;
          gameMap[xPosition][yPosition] = human;
          myBuzz(movement);
          soundStarted = true;
        }
        break;
      case right:
        if (positionIsValid(xPosition, yPosition - 1)) {
          gameMap[xPosition][yPosition] = empty;
          yPosition--;
          gameMap[xPosition][yPosition] = human;
          myBuzz(movement);
          soundStarted = true;
        }
        break;
      default:
        break;
    }
    // Update the last movement time
    lastMoveTime = millis();
  }

  placeBomb() {
    if (bombWasPlaced) {
      // If there is a bomb on the map, update the bomb's location id so the player's id does not overwrite it
      gameMap[bombX][bombY] = bomb;
    } else if (buttonWasPressed) {
      // Place the bomb if the button was pressed and there is no bomb on the map
      buttonWasPressed = false;
      bombWasPlaced = true;
      bombX = xPosition;
      bombY = yPosition;
      bombStartTime = millis();
      bombExplosionTime = millis() + bombDuration;
    }
  }

  updateBomb() {
    // Return if there is no bomb on the map
    if (!bombWasPlaced) {
      return;
    }

    // Define the explosion duration and check if the bomb is still active
    short int explosionDuration = 300;
    if (millis() - bombStartTime < bombDuration) {
      return;
    }

    // Increment the score and decrement the wall count for each wall the bomb will destroy
    checkForWalls(bombX - 1, bombY);
    checkForWalls(bombX + 1, bombY);
    checkForWalls(bombX, bombY - 1);
    checkForWalls(bombX, bombY + 1);

    if (millis() - bombExplosionTime < explosionDuration) {
      // Create the explosion animation. Replace walls with explosions
      destroy(bombX, bombY, explosion);
      destroy(bombX - 1, bombY, explosion);
      destroy(bombX + 1, bombY, explosion);
      destroy(bombX, bombY - 1, explosion);
      destroy(bombX, bombY + 1, explosion);
      myBuzz(explosionSound);
      soundStarted = true;
    } else {
      // Replace explosions with empty tiles and reset the bomb variables
      destroy(bombX, bombY, empty);
      destroy(bombX - 1, bombY, empty);
      destroy(bombX + 1, bombY, empty);
      destroy(bombX, bombY - 1, empty);
      destroy(bombX, bombY + 1, empty);

      bombWasPlaced = false;
    }
  }

  // Define the function for increasing the dificulty
  // The dificulty is increased by decreasing the bomb duration
  // If you are not fast enough, the bomb will explode before you can run
  unsigned long int lastDificultyIncrease = 0;
  const short int dificultyIncreaseDelay = 1000;
  const byte dificultyIncreseFactor = 2;

  void increaseDificulty() {
    if (millis() - lastDificultyIncrease < dificultyIncreaseDelay) {
      return;
    }

    bombDuration = max(1, bombDuration - dificultyIncreseFactor);
    lastDificultyIncrease = millis();
    displayWasUpdated = true;
  }

  // Define the function for updating the player's health
  void updateHealth() {
    if (gameMap[xPosition][yPosition] == explosion) {
      lifes--;
      displayWasUpdated = true;
    }
  }

  // Check if there are no more walls on the map
  void checkWin() {
    if (wallCount == 0) {
      gameIsOn = false;
      endScreen = true;
      displayWasUpdated = true;
    }
  }

  // Check if the player has no more lifes
  void checkLoss() {
    if (lifes == 0) {
      gameIsOn = false;
      endScreen = true;
      displayWasUpdated = true;
    }
  }

} player;

// Define the function for increamaning the score and decreasing the wall count if a wall is destroyed
void checkForWalls(byte x, byte y) {
  if (gameMap[x][y] == wall) {
    wallCount--;
    score++;
    displayWasUpdated = true;
  }
}

void generateLevel() {
  // Reset game variables
  wallCount = 0;
  score = 0;
  // Reset the map
  clearMap();
  // Generate a new map
  generateMap();
}

// Function for clearing the map before generating a new one
void clearMap() {
  for (byte i = 0; i < mapSize; i++) {
    for (byte j = 0; j < mapSize; j++) {
      gameMap[i][j] = empty;
    }
  }
}

void generateMap() {
  // Create the border
  for (byte i = 0; i < mapSize; i++) {
    gameMap[0][i] = border;
    gameMap[mapSize - 1][i] = border;
    gameMap[i][0] = border;
    gameMap[i][mapSize - 1] = border;
  }
  // Randomly place walls until the desired number is reached
  while (wallCount < maxWallCount) {
    x = random(mapSize);
    y = random(mapSize);

    if (gameMap[x][y] == empty) {
      gameMap[x][y] = wall;
      wallCount++;
    }
  }
  // Spawn the player
  player.spawnPlayer();
}

// Functions for saving and retrieving settings from EEPROM
void saveSettings() {
  EEPROM.put(lcdBrightnessAddress, lcdBrightness);
  EEPROM.put(matrixBrightnessAddress, matrixBrightness);
  EEPROM.put(soundOnAddress, soundOn);
}

void retrieveSettings() {
  EEPROM.get(lcdBrightnessAddress, lcdBrightness);
  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  EEPROM.get(soundOnAddress, soundOn);
}

// Functions for saving and retrieving highscores and names from EEPROM
void saveHighScoreAndNames() {
  for (byte i = 0; i < highScoreCount; i++) {
    EEPROM.put(highScoreAdress[i], highScore[i]);
    for (byte j = 0; j < maxNameLength; j++) {
      EEPROM.put(nameAdresses[i] + j, names[i][j]);
    }
  }
}

void retrieveHighScoreAndNames() {
  for (byte i = 0; i < highScoreCount; i++) {
    EEPROM.get(highScoreAdress[i], highScore[i]);
    for (byte j = 0; j < maxNameLength; j++) {
      EEPROM.get(nameAdresses[i] + j, names[i][j]);
    }
  }
}

// Debounce logic
void debounceJoystickButton() {
  reading = digitalRead(joystickButtonPin);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != joystickButtonState) {
      joystickButtonState = reading;
      if (reading == LOW) {
        buttonWasPressed = true;
      }
    }
  }
  lastReading = reading;
}

// Function for interpreting the joystick position
void readJoystickMovement() {
  joystickXValue = analogRead(joystickXPin);
  joystickYValue = analogRead(joystickYPin);

  if (joystickInRestPosition(joystickXValue) && joystickInRestPosition(joystickYValue)) {
    joystickDirection = standing;
    joystickWasInResetPosition = true;
    return;
  }

  // Check if the joystick is moved more on the X axis or on the Y axis
  if (abs(joystickXValue - joystickRestPosition) > abs(joystickYValue - joystickRestPosition)) {
    if (joystickXValue > joystickUpperLimit) {
      joystickDirection = right;
    } else if (joystickXValue < joystickUpperLimit) {
      joystickDirection = left;
    }
  } else {
    if (joystickYValue < joystickLowerLimit) {
      joystickDirection = up;
    } else if (joystickYValue > joystickUpperLimit) {
      joystickDirection = down;
    }
  }
}

// Function for checking if the joystick is in the reset position
bool joystickInRestPosition(short int joystickAxes) {
  return (joystickLowerLimit < joystickAxes && joystickAxes < joystickUpperLimit);
}

// Function for displaying the opening message
void displayIntroMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   Welcome to"));
  lcd.setCursor(0, 1);
  lcd.print(F("    Bomberman"));
}

// Function for printing arrows which help the user navigate the options
void printArrows(bool left, bool right, byte leftCell, byte rightCell) {
  if (left) {
    lcd.setCursor(leftCell, 0);
    lcd.print(F("<"));
  }
  if (right) {
    lcd.setCursor(rightCell, 0);
    lcd.print(F(">"));
  }
}

// Function for navigating the menus by moving the joystick left and right
void updateCurrentOption(byte &optionToUpdate, byte firstOption, byte lastOption) {
  // Only update the option if the joystick was in the reset position
  if (!joystickWasInResetPosition) {
    return;
  }

  if (joystickDirection == right) {
    optionToUpdate = min(lastOption, optionToUpdate + 1);
    joystickWasInResetPosition = false;
    displayWasUpdated = true;

  } else if (joystickDirection == left) {
    optionToUpdate = max(firstOption, optionToUpdate - 1);
    joystickWasInResetPosition = false;
    displayWasUpdated = true;
  }
  myBuzz(changeMwnuOption);
  soundStarted = true;
}

// Function used for changing a setting or a value by moving the joystick up and down
void changeSetting(byte &valueToChange, byte minValue, byte maxValue, short int increment = 1) {
  if (millis() - lastIncrementTime < incrementDelay) {
    return;
  }
  backup = valueToChange;

  if (joystickDirection == up) {
    valueToChange += increment;

  } else if (joystickDirection == down) {
    valueToChange -= increment;
  }
  lastIncrementTime = millis();
  displayWasUpdated = true;

  // Use the backup value if the new value is not valid
  if (valueToChange > maxValue || valueToChange < minValue) {
    valueToChange = backup;
  }
}

/*
  Each menu has a display function and a handle function
  The handle function is called continously while the menu is active
*/

void menuLogic() {
  displayCurrentSection();
  handleCurrentSecion();
}

void displayCurrentSection() {
  // Print the welcome message for a short period of time
  if (millis() < welcomeMessageDuration) {
    return;
  }

  // Return if the display was not updated
  if (!displayWasUpdated) {
    return;
  }

  switch (currentMenu) {
    case mainMenu:
      displayMainMenu();
      break;
    case startGame:
      generateLevel();
      enterNameMode = true;
      menuMode = false;
      displayWasUpdated = true;
      break;
    case highScores:
      displayHighScore();
      break;
    case settings:
      displaySettings();
      break;
    case about:
      displayAbout();
      break;
    case howToPlay:
      displayHowToPlay();
      break;
    default:
      break;
  }
  displayWasUpdated = false;
}

void handleCurrentSecion() {
  switch (currentMenu) {
    case mainMenu:
      handleMainMenu();
      break;
    case highScores:
      handleHighScore();
      break;
    case settings:
      handleSettings();
      break;
    case about:
      handleAbout();
      break;
    case howToPlay:
      handleHowToPlay();
      break;
    default:
      break;
  }
}

void handleMainMenu() {
  updateCurrentOption(currentOption, 0, optionsCount - 1);

  if (buttonWasPressed) {
    currentMenu = currentOption;
    buttonWasPressed = false;
    displayWasUpdated = true;
  }
}

void displayMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   Main menu"));

  lcd.setCursor(0, 1);
  switch (currentOption) {
    case startGame:
      lcd.print(F("  Start Game >"));
      displayImage(images[startGameImage]);
      break;
    case highScores:
      lcd.print(F(" < High Score >"));
      displayImage(images[highScoreImage]);
      break;
    case settings:
      lcd.print(F("  < Settings >"));
      displayImage(images[settingsImage]);
      break;
    case about:
      lcd.print(F("   < About >"));
      displayImage(images[aboutImage]);
      break;
    case howToPlay:
      lcd.print(F(" < How to play?"));
      displayImage(images[howToPlayImage]);
      break;
    default:
      currentOption = startGame;
      break;
  }
}

// Function used for navigating back to the main menu
void goToMainMenu() {
  if (buttonWasPressed) {
    currentMenu = mainMenu;
    buttonWasPressed = false;
    displayWasUpdated = true;
    saveSettings();
  }
}

void handleHighScore() {
  updateCurrentOption(currentHighScoreSection, 0, highScoreSectionsCount - 1);

  switch (currentHighScoreSection) {
    case showHighScore:
      changeSetting(currentHighScore, 0, highScoreCount - 1, -1);
      break;
    case goBackHS:
      goToMainMenu();
      break;
    default:
      break;
  }
}

void displayHighScore() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("   High Score"));

  lcd.setCursor(0, 1);
  switch (currentHighScoreSection) {
    case showHighScore:
      lcd.print(currentHighScore + 1);
      lcd.print(F("."));
      lcd.print(names[currentHighScore]);
      lcd.print(F(": "));
      lcd.print(highScore[currentHighScore]);
      printArrows(false, true, highScoreLeftCell, highScoreRightCell);
      break;
    case goBackHS:
      lcd.print(F("      Back"));
      printArrows(true, false, highScoreLeftCell, highScoreRightCell);
      break;
    default:
      currentHighScoreSection = showHighScore;
      break;
  }
}

void handleSettings() {
  updateCurrentOption(currentSetting, 0, settingsCount - 1);

  switch (currentSetting) {
    case changeLCDBrightness:
      updateLCDBrightness();
      break;
    case changeMatrixBrightness:
      updateMatrixBrightness();
      break;
    case soundControl:
      toggleSound();
      break;
    case resetHighScore:
      resetHighScores();
      break;
    case back:
      goToMainMenu();
      break;
    default:
      break;
  }
}

void updateLCDBrightness() {
  changeSetting(lcdBrightness, lcdMinBrightness, lcdMaxBrightness, lcdBrightnessIncrement);
  analogWrite(v0, lcdBrightness);
}

void updateMatrixBrightness() {
  changeSetting(matrixBrightness, matrivMinBrightness, matrixMaxBrightness, matrixBrightnessIncrement);
  displayImage(images[0]);
  ledMatrix.setIntensity(0, matrixBrightness);
}

void toggleSound() {
  if (buttonWasPressed) {
    soundOn = !soundOn;
    buttonWasPressed = false;
    displayWasUpdated = true;
  }
}

void resetHighScores() {
  if (buttonWasPressed) {
    for (int i = 0; i < highScoreCount; i++) {
      highScore[i] = 0;
      strncpy(names[i], "N/A", sizeof(names[i]) - 1);
      names[i][sizeof(names[i]) - 1] = '\0';
    }
    saveHighScoreAndNames();
    buttonWasPressed = false;
  }
}

void displaySettings() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("    Settings"));

  lcd.setCursor(0, 1);
  switch (currentSetting) {
    case changeLCDBrightness:
      lcd.print(F("LCD Bright:"));
      lcd.print(lcdBrightness);
      printArrows(false, true, settingsLeftCell, settingsRightCell);
      break;
    case changeMatrixBrightness:
      lcd.print(F("Mat Bright: "));
      lcd.print(matrixBrightness);
      printArrows(true, true, settingsLeftCell, settingsRightCell);
      break;
    case soundControl:
      lcd.print(soundOn ? F("Turn sound Off") : F("Turn sound On"));
      printArrows(true, true, settingsLeftCell, settingsRightCell);
      break;
    case resetHighScore:
      lcd.print(F("Reset highscore"));
      printArrows(true, true, settingsLeftCell, settingsRightCell);
      break;
    case back:
      lcd.print(F("      Back"));
      printArrows(true, false, settingsLeftCell, settingsRightCell);
      break;
    default:
      currentSetting = changeLCDBrightness;
      break;
  }
}

void handleAbout() {
  updateCurrentOption(currentAboutSection, 0, aboutSectionsCount - 1);

  switch (currentAboutSection) {
    case authorName:
      changeSetting(currentAboutCharacter, 0, authorNameMessageLength - 1);
      break;
    case goBack:
      goToMainMenu();
      break;
    default:
      break;
  }
}

void displayAbout() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("     About"));

  lcd.setCursor(0, 1);
  switch (currentAboutSection) {
    case githubUser:
      lcd.print(F("Github: @magiuli"));
      printArrows(false, true, aboutLeftCell, aboutRightCell);
      break;
    case authorName:
      displayText(authorNameMessage, currentAboutCharacter);
      printArrows(true, true, aboutLeftCell, aboutRightCell);
      break;
    case gameName:
      lcd.print(F("Game: Bomberman"));
      printArrows(true, true, aboutLeftCell, aboutRightCell);
      break;
    case goBack:
      lcd.print(F("      Back"));
      printArrows(true, false, aboutLeftCell, aboutRightCell);
      break;
    default:
      currentAboutSection = githubUser;
      break;
  }
}

void handleHowToPlay() {
  updateCurrentOption(currentHowToPlaySection, 0, howToPlaySectionsCount - 1);

  switch (currentHowToPlaySection) {
    case gameInstructions:
      changeSetting(currentInstructionCharacter, 0, instrunctionsLength - 1);
      break;
    case goBackHTP:
      goToMainMenu();
      break;
    default:
      break;
  }
}

void displayHowToPlay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  How to play"));

  lcd.setCursor(0, 1);
  switch (currentHowToPlaySection) {
    case gameInstructions:
      displayText(instrunctions, currentInstructionCharacter);
      printArrows(false, true, howToPlayLeftCell, howToPlayRightCell);
      break;
    case goBackHTP:
      lcd.print(F("      Back"));
      printArrows(true, false, howToPlayLeftCell, howToPlayRightCell);
      break;
    default:
      currentHowToPlaySection = gameInstructions;
      break;
  }
}

// Function for displaying an image on the matrix
//  Source: https://www.instructables.com/id/Arduino-8x8-LED-Matrix-Tutorial-and-Display-Case/
void displayImage(uint64_t image) {
  const byte uint64Size = sizeof(uint64_t);

  for (int i = 0; i < uint64Size; i++) {
    byte row = (image >> i * uint64Size) & 0xFF;
    for (int j = 0; j < uint64Size; j++) {
      ledMatrix.setLed(0, i, j, bitRead(row, j));
    }
  }
}

// Display a text on the lcd starting from a given character
void displayText(const char *text, byte currentCharacter) {
  lcd.print((__FlashStringHelper *)(text + currentCharacter));
}

// Update the camera based on the player's position. This was an absolute nightmare.
// Found the solution by trial and error
void updateCamera() {
  if (player.getX() < halfScreen) {
    leftEdge = 0;
    rightEdge = visibleSize;
  } else if (player.getX() > mapSize - halfScreen) {
    leftEdge = mapSize - visibleSize;
    rightEdge = mapSize;
  } else {
    leftEdge = player.getX() - halfScreen;
    rightEdge = player.getX() + halfScreen;
  }

  if (player.getY() < halfScreen) {
    bottomEdge = 0;
    topEdge = visibleSize;
  } else if (player.getY() > mapSize - halfScreen) {
    bottomEdge = mapSize - visibleSize;
    topEdge = mapSize;
  } else {
    bottomEdge = player.getY() - halfScreen;
    topEdge = player.getY() + halfScreen;
  }
}

// Display the visible part of the map
// Each object's animation is displayed here
void updateDisplay() {
  for (thisRow = leftEdge; thisRow < rightEdge; thisRow++) {
    for (thisColumn = bottomEdge; thisColumn < topEdge; thisColumn++) {
      byte currentObject = gameMap[thisRow][thisColumn];
      if (millis() % animationDuration[currentObject] < lightOnDuration[currentObject]) {
        ledMatrix.setLed(0, thisRow - leftEdge, thisColumn - bottomEdge, true);
      } else {
        ledMatrix.setLed(0, thisRow - leftEdge, thisColumn - bottomEdge, false);
      }
    }
  }
}

// Display the score and the player's lifes
void displayGameData() {
  if (!displayWasUpdated) {
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Score: "));
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print(F("Lives: "));
  lcd.print(player.getLives());
  lcd.print(F(" X:"));
  lcd.print(player.bombDuration);

  displayWasUpdated = false;
}

// The main game logic
void gameLogic() {
  updateCamera();
  updateDisplay();
  displayGameData();
  player.movePLayer();
  player.placeBomb();
  player.updateBomb();
  player.increaseDificulty();
  player.updateHealth();
  player.checkWin();
  player.checkLoss();
}

// The end screen logic
void endGameLogic() {
  // Uppon pressing the button, the systen switches to the main menu
  if (buttonWasPressed) {
    buttonWasPressed = false;
    menuMode = true;
    endScreen = false;
    currentMenu = mainMenu;
    displayWasUpdated = true;
  }

  // We don't want to update the display if it was not changed
  if (!displayWasUpdated) {
    return;
  }

  // Display the end screen
  if (player.getLives() == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("   Game over"));
    lcd.setCursor(0, 1);
    lcd.print(F("Score: "));
    lcd.print(score);
    displayImage(images[sadFace]);
    displayWasUpdated = false;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("   You won!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Score: "));
    lcd.print(score);
    displayImage(images[smileyFace]);
    displayWasUpdated = false;
  }
}

// Name introduction logic (not functional). Skips directly to the game
unsigned char playerName[maxNameLength] = "";
byte currentLetter = 0;

void enterNameLogic() {
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print(F("Enter your name:"));
  // // updateCurrentOption(currentLetter, 0, maxNameLength - 1);
  // changeSetting(playerName[currentLetter], 65, 90, 1);

  // lcd.setCursor(currentLetter, 1);
  // lcd.print(playerName[currentLetter]);

  // if(buttonWasPressed){
  //   buttonWasPressed = false;
  //   currentLetter++;
  //   if(currentLetter == maxNameLength){
  //     enterNameMode = false;
  //     gameIsOn = true;
  //     displayWasUpdated = true;
  //   }
  // }
  enterNameMode = false;
  gameIsOn = true;
  displayWasUpdated = true;
}

// Buzzer logic. Not so logical. At least the buzzer can be turned on and off
enum BuzzerSounds {
  movement = 0,
  explosionSound = 1,
  changeMwnuOption = 2
};

short int buzzerSounds[3] = { 200, 300, 1000 };
short int buzzerSoundsDuration[3] = { 100, 100, 500 };

bool soundStarted = false;
unsigned long int lastSoundTime = 0;

void myBuzz(byte soundId) {
  if (!soundOn) {
    return;
  }

  if (soundStarted) {
    lastSoundTime = millis();
  }

  if (millis() - lastSoundTime < buzzerSoundsDuration[soundId]) {
    tone(buzzerPin, buzzerSounds[soundId], buzzerSoundsDuration[soundId]);
  } else {
    noTone(buzzerPin);
    soundStarted = false;
  }
}

void setup() {
  // Set the pins for the joystick
  pinMode(joystickXPin, INPUT);
  pinMode(joystickYPin, INPUT);
  pinMode(joystickButtonPin, INPUT_PULLUP);

  pinMode(buzzerPin, OUTPUT);
  // Set the LCD brightness pin
  pinMode(v0, OUTPUT);

  // Retrieve the settings and highscores from EEPROM
  retrieveSettings();
  retrieveHighScoreAndNames();

  // Use a random seed based on the analog input
  // Found on Stack Overflow
  randomSeed(analogRead(0));

  ledMatrix.shutdown(0, false);  // turn off power saving, enables display
  ledMatrix.setIntensity(0, matrixBrightness);
  ledMatrix.clearDisplay(0);  // clear screen

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  displayIntroMessage();
  analogWrite(v0, lcdBrightness);

  // Begin serial communication for debugging
  Serial.begin(9600);
  // saveHighScoreAndNames();
}

void loop() {
  debounceJoystickButton();
  readJoystickMovement();
  if (gameIsOn) {
    gameLogic();
  } else if (menuMode) {
    menuLogic();
  } else if (enterNameMode) {
    enterNameLogic();
  } else if (endScreen) {
    endGameLogic();
  }
}
