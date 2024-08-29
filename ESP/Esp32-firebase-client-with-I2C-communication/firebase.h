#ifndef FIREBASE_H
#define FIREBASE_H

#include <Arduino.h>

// Firebase client
#include <FirebaseClient.h>
// Firebase json
#include <FirebaseJson.h>

#include <WiFiClientSecure.h>

#include "web_server.h"
#include "utils.h"

// Global variables for connecting to the cloud database (stored in /project_info.txt)
extern String API_KEY;
extern String FIREBASE_PROJECT_ID;

//Global variables for connecting to the cloud storage (stored in /project_info.txt)
extern String STORAGE_BUCKET_ID;

// Global variables for the firebase library
extern WiFiClientSecure ssl_client;
extern DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection
using AsyncClient = AsyncClientClass;
extern AsyncClient aClient;
extern Firestore::Documents Docs;
extern Storage storage;
extern AsyncResult aResult_no_callback;
extern FirebaseApp app;

extern FirebaseJson jsonParser;
extern FirebaseJsonData jsonData;

extern String userUid;
extern String systemId;

extern bool verified;

extern int melodiesNum;
extern std::vector<String> melodiesNames;

/*
* @brief Function to handle the set up of the connection to the firestore cloud database service
* 
*/
bool setupFirebase();

/*
* @brief Function to verify the existence of the user
* @return true if the user exists, false otherwise
*/
bool verifyUser(const String& apiKey, const String& email, const String& password);

/*
* @brief Function to handle the firebase authentication
* 
*/
void authHandler();

/*
* @brief Function to print the result of the request
*
*/
void printResult(AsyncResult& aResult);

/*
* @brief Function to print the error of the request
*/
void printError(int code, const String& msg);

bool linkUser();

bool createSystemDocument();

void fetchMelodies();

void fileCallback(File& file, const char* filename, file_operating_mode mode);

void updateDBMelodies();

/*
void saveTitleInSPIFFS(String title);

void readMelodyTitles();
*/

void readAndSendBuffer();

/*
* @brief Function to receive and save the project informations
*/
void ReceiveandSaveProjectInformations();

/*
* @brief Function to read the project informations from file
* @return true if the informations are read correctly, false otherwise
*/
bool readProjectInformations();

void readSystemInfo();

void sendSystemInfo();

void deleteOldEvents();

#endif