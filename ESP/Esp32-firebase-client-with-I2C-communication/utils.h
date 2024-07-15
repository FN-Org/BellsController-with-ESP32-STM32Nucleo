#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// JSON
#include <ArduinoJson.h>

//SPIFFS
#include <SPIFFS.h>

extern const int JSON_BUFFER_SIZE;

String removeSpacesAndNewlines(String str);

void sendPackets(int size, String string);

void currentTimeSending();

String currentTimetoJSOn(struct tm* timeData);

#endif