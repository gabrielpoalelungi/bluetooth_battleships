//include the library
#include "LedControlMS.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

/*
Digital 8 is conneted to DIN (Data IN)
Digital 9 is connected to CLK (CLocK)
Digital 10 is connected to CS (LOAD)
*/


#define NBR_MTX 2
LedControl lc=LedControl(8, 9, 10, NBR_MTX);
SoftwareSerial MyBlue(2, 3); // RX | TX
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
int turnLedOnTimer = 1;

void setup() { // initalizes and sets up the initial values. Declaring function setup.
  /* The display module is in power-saving mode on startup.
  Do a wakeup call */
  cli();
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
  
  MyBlue.begin(9600);
  Serial.begin(9600); // setting data rate as 9600 bits per second for serial data communication to computer
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
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
  lcd.print("Wait for");
  lcd.setCursor(0, 1);
  lcd.print("opponent.");
  lcd.setCursor(0, 0);
  while(opponentReady == 0) {
    if (MyBlue.available() > 0) {
      opponentReady = MyBlue.read();
    }
  }
  configureBoards();
  lc.clearAll();
  lcd.clear();
  lcd.print("Good luck!");
  delay(2000);
  lcd.clear();
}

ISR(TIMER1_COMPA_vect) {
  if (turnLedOnTimer == 1) {
    
    for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++) 
        if (enemyBoard[i][j] == 3){
         lc.setLed(0, i, j, true);
      }
    turnLedOnTimer = 0;
  } else {
    
    for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
        if (enemyBoard[i][j] == 3) {
         lc.setLed(0, i, j, false);
      }
    turnLedOnTimer = 1;
  }
}

void setBoat(int boatLength) {
  // Set boat length
  
  lcd.clear();
  lcd.print("boatlength = ");
  lcd.print(boatLength);
  lcd.setCursor(0, 1);
  lcd.print("Wait 2 s");
  lcd.setCursor(0, 0);
  delay(1000);
  lcd.clear();
  lcd.print("boatlength = ");
  lcd.print(boatLength);
  lcd.setCursor(0, 1);
  lcd.print("Wait 1 s");
  lcd.setCursor(0, 0);
  delay(1000);
  lcd.clear();

  lcd.print("Select H/V");
  // Set orientation
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Opt selected!");
      delay(1000);
      lcd.clear();
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      orientation = 1;
      lcd.clear();
      lcd.print("Select H/V");
      lcd.setCursor(0, 1);
      lcd.print("vertical");
      lcd.setCursor(0, 0);
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      orientation = 0;
      lcd.clear();
      lcd.print("Select H/V");
      lcd.setCursor(0, 1);
      lcd.print("horizontal");
      lcd.setCursor(0, 0);
      delay(300);
    }
  }

  lcd.print("Select row");
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Opt selected!");
      delay(1000);
      lcd.clear();
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      if (startColumn > 0)
        startColumn--;
      lcd.clear();
      lcd.print("Select row");
      lcd.setCursor(0, 1);
      lcd.print(startColumn + 1);
      lcd.setCursor(0, 0);
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      if (startColumn < 7)
        startColumn++;
      lcd.clear();
      lcd.print("Select row");
      lcd.setCursor(0, 1);
      lcd.print(startColumn + 1);
      lcd.setCursor(0, 0);
      delay(300);
    }
  }

  lcd.print("Select column");
  while(1) {
    if (digitalRead(selectButton) == LOW) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Opt selected!");
      delay(1000);
      lcd.clear();
      break;
    }
    if (digitalRead(decreaseButton) == LOW) {
      if (startRow > 0)
        startRow--;
      lcd.clear();
      lcd.print("Select column");
      lcd.setCursor(0, 1);
      lcd.print(startRow + 1);
      lcd.setCursor(0, 0);
      delay(300);
    }
    if (digitalRead(increaseButton) == LOW) {
      if (startRow < 7)
        startRow++;
      lcd.clear();
      lcd.print("Select column");
      lcd.setCursor(0, 1);
      lcd.print(startRow + 1);
      lcd.setCursor(0, 0);
      delay(300);
    }
  }
  lcd.print("Press select");
  lcd.setCursor(0, 1);
  lcd.print("for next boat");
  lcd.setCursor(0, 0);
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
      lcd.clear();
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
    lcd.clear();
    lcd.print("WINNER");
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    while(1) {
      lc.writeString(0,"WINNER");
      lc.clearAll();
      delay(1000);
    }
  }
  if (myTurn == 1) {
    digitalWrite(turnLED, HIGH);
    int row = 0, column = 0;
    lcd.clear();
    lcd.print("Select rowToHit");
    while(1) {
      if (digitalRead(selectButton) == LOW) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opt selected!");
        delay(1000);
        lcd.clear();
        break;
      }
      if (digitalRead(decreaseButton) == LOW) {
        if (column > 0)
          column--;
        lcd.clear();
        lcd.print("Select rowToHit");
        lcd.setCursor(0, 1);
        lcd.print(column + 1);
        lcd.setCursor(0, 0);
        delay(300);
      }
      if (digitalRead(increaseButton) == LOW) {
        if (column < 7)
          column++;
        lcd.clear();
        lcd.print("Select rowToHit");
        lcd.setCursor(0, 1);
        lcd.print(column + 1);
        lcd.setCursor(0, 0);
        delay(300);
      }
    }
    lcd.print("Select colToHit");
    while(1) {
      if (digitalRead(selectButton) == LOW) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opt selected!");
        delay(1000);
        lcd.clear();
        break;
      }
      if (digitalRead(decreaseButton) == LOW) {
        if (row > 0)
          row--;
        lcd.clear();
        lcd.print("Select colToHit");
        lcd.setCursor(0, 1);
        lcd.print(row + 1);
        lcd.setCursor(0, 0);
        delay(300);
      }
      if (digitalRead(increaseButton) == LOW) {
        if (row < 7)
          row++;
        lcd.clear();
        lcd.print("Select colToHit");
        lcd.setCursor(0, 1);
        lcd.print(row + 1);
        lcd.setCursor(0, 0);
        delay(300);
      }
    }
    if (enemyBoard[row][column] == 1) {
      enemyBoard[row][column] = 3;
      lcd.print("GJ! Hit again!");
      lcd.setCursor(0, 1);
      
      spotsLeft--;
      lcd.print(spotsLeft);
      lcd.print(" spots left!");
      lcd.setCursor(0, 0);
      delay(2500);
      lcd.clear();
    } else if (enemyBoard[row][column] == 2 || enemyBoard[row][column] == 3) {
      lcd.print("ALREADY HIT!");
      lcd.setCursor(0, 1);
      lcd.print("TRY AGAIN!");
      lcd.setCursor(0, 0);
      delay(1000);
      lcd.clear();
    }
    else {
      lcd.print("MISS! Opponent");
      lcd.setCursor(0, 1);
      lcd.print("turn!");
      lcd.setCursor(0, 0);
      enemyBoard[row][column] = 2;
      lc.setLed(0, row, column, true);
      myTurn = 0;
      MyBlue.write(69);
      MyBlue.write(1);
      digitalWrite(turnLED, LOW);
      delay(2500);
      lcd.clear();
    }
  } else {
    lcd.print("Wait for");
    lcd.setCursor(0, 1);
    lcd.print("opponent.");
    lcd.setCursor(0, 0);
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
      lcd.clear();
      lcd.print("LOSER!");
      TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
      while (1) {
        lc.writeString(0,"LOSER");
        lc.clearAll();
        delay(1000);
      }
    }
  }
  delay(500);
}
