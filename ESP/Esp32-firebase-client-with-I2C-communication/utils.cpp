#include "utils.h"

// JSON
const int JSON_BUFFER_SIZE = 256;

String removeSpacesAndNewlines(String str) {
  String result = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c != ' ' && c != '\n' && c != '\r') {
      result += c;
    }
  }
  return result;
}

void sendPackets(int size, String string) {
  int packetSize = size - 1;  // 1023 caratteri + 1 per \n
  Serial.println(string.length());
  for (int i = 0; i < string.length(); i += packetSize) {
    String packet = string.substring(i, i + packetSize);
    packet += "\n";  // Aggiungi un nuovo fine linea alla fine del pacchetto
    delay(100);
    Serial.println(packet);
    Serial.println(i);
    Serial2.print(packet);
  }
  delay(100);
}

void currentTimeSending() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error when trying to get the time.");
    return;
  }

  String JSON_time = currentTimetoJSOn(&timeinfo);
  Serial.println("Requested time, sending:" + JSON_time);

  Serial2.println(JSON_time);
}

String currentTimetoJSOn(struct tm* timeData) {
  StaticJsonDocument<JSON_BUFFER_SIZE> jsonBuffer;
  jsonBuffer["s"] = timeData->tm_sec;
  jsonBuffer["mi"] = timeData->tm_min;
  jsonBuffer["h"] = timeData->tm_hour;
  jsonBuffer["md"] = timeData->tm_mday;
  jsonBuffer["mo"] = timeData->tm_mon;
  jsonBuffer["y"] = timeData->tm_year;
  /*jsonBuffer["tm_wday"] = timeData->tm_wday;
  jsonBuffer["tm_yday"] = timeData->tm_yday;*/
  jsonBuffer["isdst"] = timeData->tm_isdst;

  String output;
  serializeJson(jsonBuffer, output);

  return output;
}