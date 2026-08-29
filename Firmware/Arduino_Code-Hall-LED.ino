#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* wifi = "YOUR_WIFI";
const char* wifi_password = "YOUR_PASSWORD";

HardwareSerial MegaSerial(2);

String line = "";

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
}

void loop() {

  while (MegaSerial.available()) {

    char c = MegaSerial.read();

    if (c == '\n') {

      line.trim();

      if (line.startsWith("FEN:")) {

        String fen = line.substring(4);
        fen.trim();

        Serial.println("FEN:");
        Serial.println(fen);

        String move = getMove(fen);

        if (move != "") {

          MegaSerial.print("MOVE:");
          MegaSerial.println(move);

          Serial.print("Move: ");
          Serial.println(move);
        }
      }

      line = "";
    }
    else {
      line += c;
    }
  }
}

String getMove(String fen) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi lost");

    WiFi.begin(wifi, wifi_password);

    return "";
  }

  HTTPClient http;

  String url = "https://lichess.org/api/cloud-eval?fen=";
  url += urlencode(fen);

  http.begin(url);

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

  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, response);

  if (error) {

    Serial.println("JSON error");

    return "";
  }

  JsonArray pvs = doc["pvs"];

  if (pvs.isNull() || pvs.size() == 0) {
    return "";
  }

  const char* moves = pvs[0]["moves"];

  if (moves == nullptr) {
    return "";
  }

  String allMoves = String(moves);

  int space = allMoves.indexOf(' ');

  if (space == -1) {
    return allMoves;
  }

  return allMoves.substring(0, space);
}

String urlencode(String text) {

  String result = "";

  const char* hex = "0123456789ABCDEF";

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
      result += hex[(c >> 4) & 0x0F];
      result += hex[c & 0x0F];
    }
  }

  return result;
}
