#include <Arduino.h>

HardwareSerial ESPSerial(1);

const int hallRows[8] = {
  22, 23, 24, 25, 26, 27, 28, 29
};

const int hallCols[8] = {
  30, 31, 32, 33, 34, 35, 36, 37
};

const int ledRows[8] = {
  38, 39, 40, 41, 42, 43, 44, 45
};

const int ledCols[8] = {
  46, 47, 48, 49, 50, 51, 52, 53
};

bool board[8][8];
bool ledBoard[8][8];

char pieces[8][8];

String espLine = "";

bool waitingForDestination = false;

int sourceRow = -1;
int sourceCol = -1;

unsigned long lastScan = 0;

unsigned long changeTime[8][8];

const unsigned long scanInterval = 20;
const unsigned long stableTime = 100;

const char files[] = "abcdefgh";


void setup() {

  Serial.begin(115200);

  ESPSerial.begin(
    115200,
    SERIAL_8N1,
    19,
    18
  );

  for (int r = 0; r < 8; r++) {

    pinMode(hallRows[r], OUTPUT);
    digitalWrite(hallRows[r], HIGH);

    pinMode(ledRows[r], OUTPUT);
    digitalWrite(ledRows[r], HIGH);
  }

  for (int c = 0; c < 8; c++) {

    pinMode(hallCols[c], INPUT_PULLUP);

    pinMode(ledCols[c], OUTPUT);
    digitalWrite(ledCols[c], LOW);
  }

  clearLEDs();

  setStartingPosition();

  delay(500);

  Serial.println("MEGA STARTED");
  Serial.println("BOARD READY");
}


void loop() {

  scanHallSensors();

  checkESP();

  updateLEDs();
}


void scanHallSensors() {

  if (millis() - lastScan < scanInterval) {
    return;
  }

  lastScan = millis();

  bool newBoard[8][8];

  for (int r = 0; r < 8; r++) {

    for (int x = 0; x < 8; x++) {
      digitalWrite(hallRows[x], HIGH);
    }

    digitalWrite(hallRows[r], LOW);

    delayMicroseconds(100);

    for (int c = 0; c < 8; c++) {

      newBoard[r][c] = !digitalRead(hallCols[c]);

    }
  }

  for (int r = 0; r < 8; r++) {

    for (int c = 0; c < 8; c++) {

      if (newBoard[r][c] != board[r][c]) {

        if (changeTime[r][c] == 0) {
          changeTime[r][c] = millis();
        }

        if (millis() - changeTime[r][c] >= stableTime) {

          board[r][c] = newBoard[r][c];

          handleSquareChange(r, c);

          changeTime[r][c] = 0;
        }
      }
      else {

        changeTime[r][c] = 0;
      }
    }
  }
}


void handleSquareChange(int r, int c) {

  if (!waitingForDestination) {

    if (!board[r][c]) {

      sourceRow = r;
      sourceCol = c;

      waitingForDestination = true;

      Serial.print("SOURCE: ");
      Serial.println(squareName(r, c));
    }

    return;
  }

  if (board[r][c]) {

    String source = squareName(sourceRow, sourceCol);

    String destination = squareName(r, c);

    String move = source + destination;

    Serial.print("MOVE DETECTED: ");
    Serial.println(move);

    movePiece(
      sourceRow,
      sourceCol,
      r,
      c
    );

    waitingForDestination = false;

    sourceRow = -1;
    sourceCol = -1;

    sendFEN();
  }
}


void movePiece(
  int sr,
  int sc,
  int dr,
  int dc
) {

  pieces[dr][dc] = pieces[sr][sc];

  pieces[sr][sc] = '.';
}


void setStartingPosition() {

  const char* starting[8] = {

    "rnbqkbnr",
    "pppppppp",
    "........",
    "........",
    "........",
    "........",
    "PPPPPPPP",
    "RNBQKBNR"

  };

  for (int r = 0; r < 8; r++) {

    for (int c = 0; c < 8; c++) {

      pieces[r][c] = starting[r][c];

      board[r][c] = starting[r][c] != '.';

      ledBoard[r][c] = false;

      changeTime[r][c] = 0;
    }
  }
}


void sendFEN() {

  String fen = "";

  for (int r = 0; r < 8; r++) {

    int empty = 0;

    for (int c = 0; c < 8; c++) {

      if (pieces[r][c] == '.') {

        empty++;
      }

      else {

        if (empty > 0) {

          fen += String(empty);

          empty = 0;
        }

        fen += pieces[r][c];
      }
    }

    if (empty > 0) {

      fen += String(empty);
    }

    if (r < 7) {

      fen += "/";
    }
  }

  fen += " w - - 0 1";

  ESPSerial.print("FEN:");
  ESPSerial.println(fen);

  Serial.print("FEN: ");
  Serial.println(fen);
}


void checkESP() {

  while (ESPSerial.available()) {

    char c = ESPSerial.read();

    if (c == '\n') {

      espLine.trim();

      if (espLine.startsWith("MOVE:")) {

        String move = espLine.substring(5);

        move.trim();

        Serial.print("ESP MOVE: ");
        Serial.println(move);

        executeEngineMove(move);
      }

      espLine = "";
    }

    else {

      espLine += c;
    }
  }
}


void executeEngineMove(String move) {

  if (move.length() < 4) {
    return;
  }

  int sc = move[0] - 'a';

  int sr = 8 - (move[1] - '0');

  int dc = move[2] - 'a';

  int dr = 8 - (move[3] - '0');

  if (
    sr < 0 || sr > 7 ||
    sc < 0 || sc > 7 ||
    dr < 0 || dr > 7 ||
    dc < 0 || dc > 7
  ) {

    return;
  }

  movePiece(
    sr,
    sc,
    dr,
    dc
  );

  showMove(
    sr,
    sc,
    dr,
    dc
  );
}


void showMove(
  int sr,
  int sc,
  int dr,
  int dc
) {

  clearLEDs();

  ledBoard[sr][sc] = true;

  ledBoard[dr][dc] = true;

  Serial.print("LIGHT: ");

  Serial.print(
    squareName(sr, sc)
  );

  Serial.print(" -> ");

  Serial.println(
    squareName(dr, dc)
  );
}


void clearLEDs() {

  for (int r = 0; r < 8; r++) {

    for (int c = 0; c < 8; c++) {

      ledBoard[r][c] = false;
    }
  }
}


void updateLEDs() {

  static int row = 0;

  for (int r = 0; r < 8; r++) {

    digitalWrite(
      ledRows[r],
      HIGH
    );
  }

  for (int c = 0; c < 8; c++) {

    digitalWrite(
      ledCols[c],
      LOW
    );
  }

  digitalWrite(
    ledRows[row],
    LOW
  );

  for (int c = 0; c < 8; c++) {

    if (ledBoard[row][c]) {

      digitalWrite(
        ledCols[c],
        HIGH
      );
    }
  }

  delayMicroseconds(1000);

  digitalWrite(
    ledRows[row],
    HIGH
  );

  row++;

  if (row >= 8) {

    row = 0;
  }
}


String squareName(int r, int c) {

  String result = "";

  result += files[c];

  result += String(8 - r);

  return result;
}
