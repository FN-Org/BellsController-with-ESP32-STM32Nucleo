#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
// Wifi
#include <WiFi.h>
// Wifi udp for ntp
#include <WiFiUdp.h>
// Web server per l'access point
#include <WebServer.h>

//SPIFFS
#include <SPIFFS.h>

// Global varaibles for wifi and autentication (stored in /credentials.txt)
// Credential Form
extern String ssid;
extern String wifi_password;
extern String email;
extern String account_password;

// Global varaibles for system information (stored in /system_info.txt)
// System Form
extern String name;
extern String location;
extern int bellsNum;
extern int pin;

// Global variables for the access point
extern const char* ap_ssid;
extern const char* ap_password;

// Selecting the port 80 for the server (standard http port)
extern WebServer server;

extern const int default_Nmelodies;

/*
* @brief Function to handle the submit of the form when connected to the access point
* 
*/
void handleCredentialsSubmit();

void handleSystemSubmit();

/*
* @brief Function to start the access point
* 
*/
void startAccessPoint();

/*
* @brief Function to handle the connection to the wifi
* @return true if the connection went right, false otherwise
*/
bool connectToWifi();

/*
* @brief Function to handle the connection to the Access Point
* 
*/
void handleCredentialsForm();

void handleSystemForm();

/*
* @brief Function to save credential in the file "/credentials.txt"
* 
*/
void saveCredentials(const String& ssid, const String& wifi_password, const String& email, const String& account_password);

void saveSystemInfo(const String& name, const String& location, const int& bNum, const int& mNum, const int& pin);

/*
* @brief Function to read credential from the file "/credentials.txt"
* 
*/
void readCredentials();

#endif