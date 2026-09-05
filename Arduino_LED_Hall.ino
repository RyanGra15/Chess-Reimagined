#include <Arduino.h>

HardwareSerial ESPSerial(1);

const int hallRows[] = {22, 23, 24, 25, 26, 27, 28, 29};
const int hallCols[] = {30, 31, 32, 33, 34, 35, 36, 37};

const int ledRows[] = {38, 39, 40, 41, 42, 43, 44, 45};
const int ledCols[] = {46, 47, 48, 49, 50, 51, 52, 53};

int board[64] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1
};

int oldBoard[64] = {0};

bool leds[8][8] = {false};

unsigned long lastLED = 0;
int ledDelay = 2;

int lifted = -1;
int placed = -1;

void setup() {
  ESPSerial.begin(115200);

  for(int i = 0;i<8;i++) {
    pinMode(hallRows[i], OUTPUT);
    digitalWrite(hallRows[i], LOW);

    pinMode(ledRows[i], OUTPUT);
    digitalWrite(ledRows[i], LOW);
  }

  for(int i = 0; i < 8; i++) {
    pinMode(hallCols[i], INPUT_PULLUP);

    pinMode(ledCols[i], OUTPUT);
    digitalWrite(ledCols[i], HIGH);
  }

  for(int i = 0;i<64;i++)
    oldBoard[i] = board[i];

  ESPSerial.println("MEGA STARTED");
}

void scanHall() {
  for(int row = 0; row<8; row++) {
    for(int x = 0;x<8;x++)
      digitalWrite(hallRows[x], LOW);

    digitalWrite(hallRows[row], HIGH);

    for(int col = 0;col<8;col++) {
      int a = digitalRead(hallCols[col]);

      delayMicroseconds(120);

      int b = digitalRead(hallCols[col]);

      int n = row * 8 + col;

      if(a == b) {
        if(a == LOW)
          board[n] = 1;
        else
          board[n] = 0;
      }
    }

    delayMicroseconds(100);
  }
}

void checkBoard() {
  int liftedSquare = -1;
  int newSquare = -1;

  for(int i = 0;i<64;i++) {
    if(board[i] == oldBoard[i])
      continue;

    if(oldBoard[i] == 1 && board[i] == 0) {
      liftedSquare = i;
      lifted = i;

      int r = i / 8;
      int c = i % 8;

      char letter = 'A' + c;
      int number = 8-r;

      ESPSerial.print("LIFTED: ");
      ESPSerial.print(letter);
      ESPSerial.println(number);
    }

    if(oldBoard[i] == 0 && board[i] == 1) {
      newSquare = i;
      placed = i;

      int r = i / 8;
      int c = i % 8;

      char letter = 'A' + c;
      int number = 8-r;

      ESPSerial.print("PLACED: ");
      ESPSerial.print(letter);
      ESPSerial.println(number);
    }
  }

  if(liftedSquare != -1 && newSquare != -1) {
    int r1 = liftedSquare / 8;
    int c1 = liftedSquare % 8;

    int r2 = newSquare / 8;
    int c2 = newSquare % 8;

    char f1 = 'A' + c1;
    char f2 = 'A' + c2;

    int n1 = 8-r1;
    int n2 = 8-r2;

    ESPSerial.print("MOVE: ");
    ESPSerial.print(f1);
    ESPSerial.print(n1);
    ESPSerial.print(" -> ");
    ESPSerial.print(f2);
    ESPSerial.println(n2);

    leds[r1][c1] = true;
    leds[r2][c2] = true;

    lifted = -1;
    placed = -1;
  }

  for(int i = 0;i<64;i++) {
    oldBoard[i] = board[i];
  }
}

void showLEDs() {
  if(millis() - lastLED < ledDelay)
    return;

  lastLED = millis();

  for(int row = 0; row<8; row++) {
    for(int i = 0;i<8;i++) {
      digitalWrite(ledRows[i], LOW);
      digitalWrite(ledCols[i], HIGH);
    }

    digitalWrite(ledRows[row], HIGH);

    for(int col = 0;col<8;col++) {
      if(leds[row][col])
        digitalWrite(ledCols[col], LOW);
    }

    delayMicroseconds(250);
  }
}

void loop() {
  scanHall();

  checkBoard();

  showLEDs();
}
