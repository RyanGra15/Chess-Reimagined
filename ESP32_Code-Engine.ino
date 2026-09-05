#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

const char* wifi = "wifi_name";
const char* wifi_password = "wifi_password";

HardwareSerial MegaSerial(2);

String line = "";

char board[8][8] = {
  {'r','n','b','q','k','b','n','r'},
  {'p','p','p','p','p','p','p','p'},
  {' ',' ',' ',' ',' ',' ',' ',' '},
  {' ',' ',' ',' ',' ',' ',' ',' '},
  {' ',' ',' ',' ',' ',' ',' ',' '},
  {' ',' ',' ',' ',' ',' ',' ',' '},
  {'P','P','P','P','P','P','P','P'},
  {'R','N','B','Q','K','B','N','R'}
};

bool whiteTurn = true;

bool whiteCastleKing = true;
bool whiteCastleQueen = true;
bool blackCastleKing = true;
bool blackCastleQueen = true;

String enPassant = "-";

int halfmove = 0;
int fullmove = 1;


void setup() {

  Serial.begin(115200);

  MegaSerial.begin(
    115200,
    SERIAL_8N1,
    16,
    17
  );

  WiFi.begin(wifi, wifi_password);

  Serial.print("WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected");
  Serial.println(WiFi.localIP());

  Serial.println("Chess ESP32 ready");
}


void loop() {

  while (MegaSerial.available()) {

    char c = MegaSerial.read();

    if (c == '\n') {

      line.trim();

      if (line.startsWith("MOVE:")) {

        String move = line.substring(5);
        move.trim();

        Serial.print("Received move: ");
        Serial.println(move);

        if (makeMove(move)) {

          String fen = makeFEN();

          Serial.print("FEN: ");
          Serial.println(fen);

          String bestMove = getMove(fen);

          if (bestMove != "") {

            Serial.print("Engine move: ");
            Serial.println(bestMove);

            MegaSerial.print("LIGHT: ");

            MegaSerial.print(bestMove.substring(0, 2));
            MegaSerial.print(",");
            MegaSerial.println(bestMove.substring(2, 4));

            makeEngineMove(bestMove);
          }
        }
      }

      line = "";
    }
    else {

      line += c;
    }
  }
}


bool makeMove(String move) {

  move.trim();

  int arrow = move.indexOf("->");

  if (arrow == -1) {

    Serial.println("Bad move format");
    return false;
  }

  String from = move.substring(0, arrow);
  String to = move.substring(arrow + 2);

  from.trim();
  to.trim();

  from.toLowerCase();
  to.toLowerCase();

  if (from.length() != 2 || to.length() != 2) {

    Serial.println("Bad square");
    return false;
  }

  int fromCol = from[0] - 'a';
  int fromRow = 8 - (from[1] - '0');

  int toCol = to[0] - 'a';
  int toRow = 8 - (to[1] - '0');

  if (
    fromCol < 0 || fromCol > 7 ||
    fromRow < 0 || fromRow > 7 ||
    toCol < 0 || toCol > 7 ||
    toRow < 0 || toRow > 7
  ) {

    Serial.println("Square outside board");
    return false;
  }

  char piece = board[fromRow][fromCol];
  char captured = board[toRow][toCol];

  if (piece == ' ') {

    Serial.println("No piece on source square");
    return false;
  }

  if (isWhite(piece) != whiteTurn) {

    Serial.println("Wrong side to move");
    return false;
  }

  if (
    (piece == 'P' || piece == 'p') &&
    captured == ' ' &&
    fromCol != toCol
  ) {

    if (toRow == 2 || toRow == 5) {

      int captureRow;

      if (piece == 'P')
        captureRow = toRow + 1;
      else
        captureRow = toRow - 1;

      if (
        captureRow >= 0 &&
        captureRow < 8 &&
        board[captureRow][toCol] != ' '
      ) {

        board[captureRow][toCol] = ' ';
      }
    }
  }

  updateCastling(piece, fromRow, fromCol);

  if (captured != ' ') {

    if (captured == 'R') {

      if (toRow == 7 && toCol == 0)
        whiteCastleQueen = false;

      if (toRow == 7 && toCol == 7)
        whiteCastleKing = false;
    }

    if (captured == 'r') {

      if (toRow == 0 && toCol == 0)
        blackCastleQueen = false;

      if (toRow == 0 && toCol == 7)
        blackCastleKing = false;
    }
  }

  board[toRow][toCol] = piece;
  board[fromRow][fromCol] = ' ';

  if (piece == 'P' && toRow == 0)
    board[toRow][toCol] = 'Q';

  if (piece == 'p' && toRow == 7)
    board[toRow][toCol] = 'q';

  enPassant = "-";

  if (piece == 'P' && fromRow == 6 && toRow == 4) {

    enPassant =
      String(char('a' + fromCol)) +
      String(3);
  }

  if (piece == 'p' && fromRow == 1 && toRow == 3) {

    enPassant =
      String(char('a' + fromCol)) +
      String(6);
  }

  if (piece == 'P' || piece == 'p' || captured != ' ')
    halfmove = 0;
  else
    halfmove++;

  if (!whiteTurn)
    fullmove++;

  whiteTurn = !whiteTurn;

  return true;
}


void updateCastling(char piece, int row, int col) {

  if (piece == 'K') {

    whiteCastleKing = false;
    whiteCastleQueen = false;

    if (row == 7 && col == 4) {

      if (board[7][7] == 'R' && board[7][6] == ' ') {

        board[7][5] = 'R';
        board[7][7] = ' ';
      }

      else if (board[7][0] == 'R' && board[7][1] == ' ') {

        board[7][3] = 'R';
        board[7][0] = ' ';
      }
    }
  }

  if (piece == 'k') {

    blackCastleKing = false;
    blackCastleQueen = false;

    if (row == 0 && col == 4) {

      if (board[0][7] == 'r' && board[0][6] == ' ') {

        board[0][5] = 'r';
        board[0][7] = ' ';
      }

      else if (board[0][0] == 'r' && board[0][1] == ' ') {

        board[0][3] = 'r';
        board[0][0] = ' ';
      }
    }
  }

  if (piece == 'R') {

    if (row == 7 && col == 0)
      whiteCastleQueen = false;

    if (row == 7 && col == 7)
      whiteCastleKing = false;
  }

  if (piece == 'r') {

    if (row == 0 && col == 0)
      blackCastleQueen = false;

    if (row == 0 && col == 7)
      blackCastleKing = false;
  }
}


String makeFEN() {

  String fen = "";

  for (int row = 0; row < 8; row++) {

    int empty = 0;

    for (int col = 0; col < 8; col++) {

      char piece = board[row][col];

      if (piece == ' ') {

        empty++;
      }
      else {

        if (empty > 0) {

          fen += String(empty);
          empty = 0;
        }

        fen += piece;
      }
    }

    if (empty > 0)
      fen += String(empty);

    if (row != 7)
      fen += "/";
  }

  fen += " ";

  if (whiteTurn)
    fen += "w";
  else
    fen += "b";

  fen += " ";

  String castle = "";

  if (whiteCastleKing)
    castle += "K";

  if (whiteCastleQueen)
    castle += "Q";

  if (blackCastleKing)
    castle += "k";

  if (blackCastleQueen)
    castle += "q";

  if (castle == "")
    castle = "-";

  fen += castle;

  fen += " ";
  fen += enPassant;

  fen += " ";
  fen += String(halfmove);

  fen += " ";
  fen += String(fullmove);

  return fen;
}


String getMove(String fen) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi lost");

    WiFi.begin(wifi, wifi_password);

    return "";
  }

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  String url =
    "https://stockfish.online/api/s/v2.php?fen=";

  url += urlencode(fen);
  url += "&depth=12";

  Serial.println("Sending to Stockfish...");

  if (!http.begin(client, url)) {

    Serial.println("HTTP begin failed");
    return "";
  }

  http.addHeader(
    "Accept",
    "application/json"
  );

  int status = http.GET();

  if (status != 200) {

    Serial.print("API error: ");
    Serial.println(status);

    http.end();

    return "";
  }

  String response = http.getString();

  http.end();

  Serial.print("API response: ");
  Serial.println(response);

  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, response);

  if (error) {

    Serial.print("JSON error: ");
    Serial.println(error.c_str());

    return "";
  }

  bool success = doc["success"];

  if (!success) {

    Serial.println("Stockfish request failed");

    return "";
  }

  const char* best =
    doc["bestmove"];

  if (best == nullptr) {

    Serial.println("No bestmove received");

    return "";
  }

  String bestMove = String(best);

  Serial.print("Raw bestmove: ");
  Serial.println(bestMove);

  int firstSpace = bestMove.indexOf(' ');

  if (firstSpace == -1) {

    if (bestMove.length() >= 4)
      return bestMove.substring(0, 4);

    return "";
  }

  int secondSpace =
    bestMove.indexOf(' ', firstSpace + 1);

  if (secondSpace == -1) {

    return bestMove.substring(firstSpace + 1);
  }

  return bestMove.substring(
    firstSpace + 1,
    secondSpace
  );
}


void makeEngineMove(String move) {

  if (move.length() < 4)
    return;

  move.toLowerCase();

  int fromCol = move[0] - 'a';
  int fromRow = 8 - (move[1] - '0');

  int toCol = move[2] - 'a';
  int toRow = 8 - (move[3] - '0');

  if (
    fromCol < 0 || fromCol > 7 ||
    fromRow < 0 || fromRow > 7 ||
    toCol < 0 || toCol > 7 ||
    toRow < 0 || toRow > 7
  )
    return;

  char piece = board[fromRow][fromCol];

  if (piece == ' ')
    return;

  char captured = board[toRow][toCol];

  updateCastling(piece, fromRow, fromCol);

  if (captured == 'R') {

    if (toRow == 7 && toCol == 0)
      whiteCastleQueen = false;

    if (toRow == 7 && toCol == 7)
      whiteCastleKing = false;
  }

  if (captured == 'r') {

    if (toRow == 0 && toCol == 0)
      blackCastleQueen = false;

    if (toRow == 0 && toCol == 7)
      blackCastleKing = false;
  }

  board[toRow][toCol] = piece;
  board[fromRow][fromCol] = ' ';

  if (move.length() >= 5) {

    char promotion = move[4];

    if (piece == 'P') {

      if (promotion == 'q')
        board[toRow][toCol] = 'Q';

      if (promotion == 'r')
        board[toRow][toCol] = 'R';

      if (promotion == 'b')
        board[toRow][toCol] = 'B';

      if (promotion == 'n')
        board[toRow][toCol] = 'N';
    }

    if (piece == 'p') {

      if (promotion == 'q')
        board[toRow][toCol] = 'q';

      if (promotion == 'r')
        board[toRow][toCol] = 'r';

      if (promotion == 'b')
        board[toRow][toCol] = 'b';

      if (promotion == 'n')
        board[toRow][toCol] = 'n';
    }
  }

  if (piece == 'P' || piece == 'p' || captured != ' ')
    halfmove = 0;
  else
    halfmove++;

  if (!whiteTurn)
    fullmove++;

  enPassant = "-";

  if (piece == 'P' && fromRow == 6 && toRow == 4) {

    enPassant =
      String(char('a' + fromCol)) +
      String(3);
  }

  if (piece == 'p' && fromRow == 1 && toRow == 3) {

    enPassant =
      String(char('a' + fromCol)) +
      String(6);
  }

  whiteTurn = !whiteTurn;
}


bool isWhite(char piece) {

  return (
    piece == 'P' ||
    piece == 'N' ||
    piece == 'B' ||
    piece == 'R' ||
    piece == 'Q' ||
    piece == 'K'
  );
}


String urlencode(String text) {

  String result = "";

  const char* hex =
    "0123456789ABCDEF";

  for (int i = 0; i < text.length(); i++) {

    char c = text[i];

    if (
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      (c >= '0' && c <= '9') ||
      c == '-' ||
      c == '_' ||
      c == '.' ||
      c == '~'
    ) {

      result += c;
    }
    else {

      result += '%';

      result +=
        hex[(c >> 4) & 0x0F];

      result +=
        hex[c & 0x0F];
    }
  }

  return result;
}