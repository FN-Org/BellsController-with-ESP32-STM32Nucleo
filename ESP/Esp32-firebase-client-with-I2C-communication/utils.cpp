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

  Serial2.println("---");
  delay(100);
  Serial2.println("-T-");
  delay(100);
  String JSON_time = currentTimetoJSOn(&timeinfo);
  Serial2.println(JSON_time);
  delay(300);
  Serial2.println("---");
  Serial.println(JSON_time);
}

String currentTimetoJSOn(struct tm* timeData) {
  StaticJsonDocument<JSON_BUFFER_SIZE> jsonBuffer;
  jsonBuffer["s"] = timeData->tm_sec; //Seconds (0 to 59)
  jsonBuffer["mi"] = timeData->tm_min; //Minutes (0 to 59)
  jsonBuffer["h"] = timeData->tm_hour; //Hour (0 to 23)
  jsonBuffer["md"] = timeData->tm_mday; //Day of the month (1 to 31)
  jsonBuffer["mo"] = timeData->tm_mon; //Month of the year (0 to 11)
  jsonBuffer["y"] = timeData->tm_year; //Year from 1900 (2024 - 1900 = 124)
  jsonBuffer["wd"] = timeData->tm_wday; // Day of the week (0 to 6) 6 is saturday
  //jsonBuffer["tm_yday"] = timeData->tm_yday;*/ //Day of the year (0 to 365) 365 is 31 december
  jsonBuffer["l"] = timeData->tm_isdst; //Legal hour, if in use is >0, ==0 not in use, <0 not known

  String output;
  serializeJson(jsonBuffer, output);

  return output;
}