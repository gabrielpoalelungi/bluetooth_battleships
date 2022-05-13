//include the library
#include "LedControlMS.h"
#include <SoftwareSerial.h>

/*
Digital 8 is conneted to DIN (Data IN)
Digital 9 is connected to CLK (CLocK)
Digital 10 is connected to CS (LOAD)
*/


#define NBR_MTX 2
LedControl lc=LedControl(8, 9, 10, NBR_MTX);
SoftwareSerial MyBlue(2, 3); // RX | TX

int selectButton = 7;
int increaseButton = 6;
int decreaseButton = 5;
int turnLED = 4;
byte enemyBoard[8][8];
byte myBoard[8][8];

int enemyBoardReceived = 0;
int readyToRead = 0;
int sent = 0;
int opponentReady = 0;
int spotsLeft = 12;
int myTurn = 1;

int orientation;
int startRow = 0;
int startColumn = 0;
int boatLength = 3;
int select = 0;

void setup() { // initalizes and sets up the initial values. Declaring function setup.
  /* The display module is in power-saving mode on startup.
  Do a wakeup call */
  MyBlue.begin(9600);
  Serial.begin(9600); // setting data rate as 9600 bits per second for serial data communication to computer

  for (int i=0; i< NBR_MTX; i++){
    lc.shutdown(i,false); //keep the screen on
    lc.setIntensity(i,8); // set brightness to medium values
    lc.clearDisplay(i); //clear the display after each letter
  }
  for (int i=0; i <= 7; i++)
    for (int j = 0; j <= 7; j++){
    enemyBoard[i][j] = 0;
    myBoard[i][j] = 0;
  }
  pinMode(selectButton, INPUT_PULLUP);
  pinMode(decreaseButton, INPUT_PULLUP);
  pinMode(increaseButton, INPUT_PULLUP);
  pinMode(turnLED, OUTPUT);
  
  setBoat(3);
  setBoat(4);
  setBoat(5);
  MyBlue.write(1);
  Serial.println("Wait for opponent.");
  while(opponentReady == 0) {
    if (MyBlue.available() > 0) {
      opponentReady = MyBlue.read();
    }
  }
  configureBoards();
  lc.clearAll();
  Serial.println("Good luck!");
}

void setBoat(int boatLength) {
  // Set boat length
  Serial.print("Please choose options for boatLength = ");
  Serial.println(boatLength);

  Serial.println("Please select orientation! ");
  // Set orientation
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      Serial.println("ORIENTATION SELECTED!");
      delay(300);
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      orientation = 1;
      Serial.println("vertical");
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      orientation = 0;
      Serial.println("horizontal");
      delay(300);
    }
  }

  Serial.println("Please select startRow! ");
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      Serial.println("START ROW SELECTED!");
      delay(300);
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      if (startColumn > 0)
        startColumn--;
      Serial.println(startColumn + 1);
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      if (startColumn < 7)
        startColumn++;
      Serial.println(startColumn + 1);
      delay(300);
    }
  }

  Serial.println("Please select startColumn! ");
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      Serial.println("START COLUMN SELECTED!");
      delay(300);
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      if (startRow > 0)
        startRow--;
      Serial.println(startRow + 1);
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      if (startRow < 7)
        startRow++;
      Serial.println(startRow + 1);
      delay(300);
    }
  }

  while (1) {
    // Vertical
    if (orientation == 1) {
      for (int j = startColumn; j < startColumn + boatLength; j++) {
        lc.setLed(0, startRow, j, true);
        myBoard[startRow][j] = 1;
      }
    } else {
      // Horizontal
      for (int i = startRow; i < startRow + boatLength; i++) {
        lc.setLed(0, i, startColumn, true);
        myBoard[i][startColumn] = 1;
      }
    }
    
    if (digitalRead(selectButton) == LOW){
      orientation = 0;
      startRow = 0;
      startColumn = 0;
      boatLength = 3;
      delay(300);
      break;
    }
  }
}

void configureBoards() {
  if (sent == 0) {
    MyBlue.write(69);
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++){
        MyBlue.write(myBoard[i][j]);
      }
    }
    sent = 1;
  }
  if (enemyBoardReceived == 0) {
    while(readyToRead != 69) {
      if (MyBlue.available() > 0) {
        readyToRead = MyBlue.read();
      }
    }
    readyToRead = 0;
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        while(MyBlue.available() <= 0) {
        }
        enemyBoard[i][j] = MyBlue.read();
      }
    }
    enemyBoardReceived = 1;
  }
}

void loop() { //declaring function loop
  if (spotsLeft == 0) {
    MyBlue.write(69);
    MyBlue.write(10);
    while(1) {
      Serial.println("WINNER!");
      lc.writeString(0,"WINNER");
      lc.clearAll();
      delay(1000);
    }
  }
  if (myTurn == 1) {
    digitalWrite(turnLED, HIGH);
    int row = 0, column = 0;
    Serial.println("Please select row to hit! ");
    while(1) {
      if (digitalRead(selectButton) == LOW) {
        Serial.println("ROW TO HIT SELECTED!");
        delay(300);
        break;
      }
      if (digitalRead(decreaseButton) == LOW) {
        if (column > 0)
          column--;
        Serial.println(column + 1);
        delay(300);
      }
      if (digitalRead(increaseButton) == LOW) {
        if (column < 7)
          column++;
        Serial.println(column + 1);
        delay(300);
      }
    }
    Serial.println("Please select column to hit! ");
    while(1) {
      if (digitalRead(selectButton) == LOW) {
        Serial.println("COLUMN TO HIT SELECTED!");
        delay(300);
        break;
      }
      if (digitalRead(decreaseButton) == LOW) {
        if (row > 0)
          row--;
        Serial.println(row + 1);
        delay(300);
      }
      if (digitalRead(increaseButton) == LOW) {
        if (row < 7)
          row++;
        Serial.println(row + 1);
        delay(300);
      }
    }
    if (enemyBoard[row][column] == 1) {
      enemyBoard[row][column] = 2;
      Serial.println("HIT! YOUR TURN AGAIN!");
      lc.setLed(0, row, column, true);
      spotsLeft--;
      Serial.print(spotsLeft);
      Serial.println(" spots left to hit!");
    } else if (enemyBoard[row][column] == 2) {
      Serial.println("ALREADY HIT! TRY AGAIN!");
    }
    else {
      Serial.println("MISS! OPPONENT TURN");
      enemyBoard[row][column] = 2;
      myTurn = 0;
      MyBlue.write(69);
      MyBlue.write(1);
    }
  } else {
    digitalWrite(turnLED, LOW);
    Serial.println("Wait for opponent!");
    while(readyToRead != 69) {
      if (MyBlue.available() > 0) {
        readyToRead = MyBlue.read();
      }
    }
    readyToRead = 0;
    while(MyBlue.available() <= 0) {
    }
    myTurn = MyBlue.read();
    if (myTurn == 10) {
      while (1) {
        Serial.println("LOSER!");
        lc.writeString(0,"LOSER");
        lc.clearAll();
        delay(1000);
      }
    }
  }
  delay(500);
}
