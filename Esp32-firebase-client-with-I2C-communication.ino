//#####################################################################################
//includes//
#include <Arduino.h>

//SPIFFS
#include <SPIFFS.h>
#include <FS.h>

// Wifi
#include <WiFi.h>
// Wifi udp for ntp
#include <WiFiUdp.h>
// Web server per l'access point
#include <WebServer.h>
// Firebase client
#include <FirebaseClient.h>
// Firebase json
#include <FirebaseJson.h>

#include <WiFiClientSecure.h>

// Managing time values
#include "time.h"

// JSON
#include <ArduinoJson.h>
//########################################################################################






//########################################################################################
// Global Variables//

// Global variables for connecting to the cloud database (stored in /project_info.txt)
String API_KEY = "";
String FIREBASE_PROJECT_ID = "";

// Global variables for the firebase library
WiFiClientSecure ssl_client;
DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;
AsyncResult aResult_no_callback;
FirebaseApp app;

FirebaseJson jsonParser;
FirebaseJsonData jsonData;

// Global variables for the access point
const char* ap_ssid = "ESP32_AP";
const char* ap_password = "12345678";

// Selecting the port 80 for the server (standard http port)
WebServer server(80);

// Global varaibles for wifi and autentication (stored in /credentials.txt)
String ssid = "";
String wifi_password = "";
String email = "";
String account_password = "";

// Global variables for system information and user information (stored in /systeminfo.txt)
String systemId = "";
int bellsNum = 0;
int melodiesNum = 0;
String userUid = "";

// Other global variables
bool verified = false;
unsigned long dataMillis = 0;
bool taskCompleted = false;
bool first_time = true;
bool first_start = false;

// Global variables for the time
// Set time zone (UTC+1 for Italy)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;      // UTC +1 (3600 seconds)
const int daylightOffset_sec = 3600;  // Ora legale


// JSON
#define JSON_BUFFER_SIZE 256

unsigned long last_time_sent = 0;
//###################################################################################################








//###################################################################################################
//FUNCTIONS//

/*
* @brief Function to handle the connection to the Access Point
* 
*/
void handleRoot() {
  String htmlForm = "<!DOCTYPE html><html><head><style>"
                    "body {background-color: #007BFF; font-family: Arial, sans-serif;}"
                    ".container {background-color: white; padding: 20px; margin: auto; width: 50%; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}"
                    "input[type=text], input[type=password] {width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}"
                    "input[type=submit] {width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer;}"
                    "input[type=submit]:hover {background-color: #45a049;}"
                    "label {font-weight: bold;}"
                    "</style></head><body>"
                    "<div class=\"container\">"
                    "<form action=\"/get\" method=\"post\">"
                    "<label for=\"ssid\">SSID:</label><br>"
                    "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>"
                    "<label for=\"wifi_password\">WiFi Password:</label><br>"
                    "<input type=\"text\" id=\"wifi_password\" name=\"wifi_password\"><br>"
                    "<label for=\"email\">Email:</label><br>"
                    "<input type=\"text\" id=\"email\" name=\"email\"><br>"
                    "<label for=\"account_password\">Account Password:</label><br>"
                    "<input type=\"text\" id=\"account_password\" name=\"account_password\"><br><br>"
                    "<input type=\"submit\" value=\"Submit\">"
                    "</form></div>"
                    "</body></html>";
  server.send(200, "text/html", htmlForm);
}

/*
* @brief Function to save credential in the file "/credentials.txt"
* 
*/
void saveCredentials(const String& ssid, const String& wifi_password, const String& email, const String& account_password) {
  File file = SPIFFS.open("/credentials.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.println(ssid);
  file.println(wifi_password);
  file.println(email);
  file.println(account_password);
  file.close();
}


/*
* @brief Function to read credential from the file "/credentials.txt"
* 
*/
void readCredentials() {
  Serial.println("Reading credentials...");
  File file = SPIFFS.open("/credentials.txt", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  ssid = file.readStringUntil('\n');
  wifi_password = file.readStringUntil('\n');
  email = file.readStringUntil('\n');
  account_password = file.readStringUntil('\n');
  file.close();

  ssid.trim();
  wifi_password.trim();
  email.trim();
  account_password.trim();

  Serial.println("Read credentials:");
  Serial.println("SSID: " + ssid);
  Serial.println("wifi_password: " + wifi_password);
  Serial.println("email: " + email);
  Serial.println("account password: " + account_password);
}


/*
* @brief Function to handle the submit of the form when connected to the access point
* 
*/
void handleSubmit() {
  ssid = server.arg("ssid");
  wifi_password = server.arg("wifi_password");

  email = server.arg("email");
  account_password = server.arg("account_password");

  // Save credentials on SPIFFS
  saveCredentials(ssid, wifi_password, email, account_password);

  //Print credentials for debugging
  Serial.println("SSID: " + ssid);
  Serial.println("wifi_password: " + wifi_password);

  Serial.println("email: " + email);
  Serial.println("account password: " + account_password);

  server.send(200, "text/html", "Credentials received. Rebooting...");
  delay(2000);
  ESP.restart();
}

/*
* @brief Function to start the access point
* 
*/
void startAccessPoint() {
  // Initialize Wi-Fi as access point
  Serial.println("Setting up AP...");
  bool result = WiFi.softAP(ap_ssid, ap_password);
  if (result) {
    Serial.println("AP setup successful.");
  } else {
    Serial.println("AP setup failed.");
    return;
  }

  // Get and print the IP of the access point
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Set the handlers of the web server
  server.on("/", handleRoot);
  server.on("/get", HTTP_POST, handleSubmit);
  server.begin();
  Serial.println("Web server started.");
}

/*
* @brief Function to handle the connection to the wifi
* @return true if the connection went right, false otherwise
*/
bool connectToWifi() {
  readCredentials();
  delay(100);
  WiFi.begin(ssid, wifi_password);
  // Auto reconnect is set true as default
  // To set auto connect off, use the following function
  //    WiFi.setAutoReconnect(false);

  // Will try for about 10 seconds (20 x 500ms)
  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
  while (true) {

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: Serial.println("[WiFi] SSID not found"); break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected!");
        return false;
        break;
      case WL_CONNECTION_LOST: Serial.println("[WiFi] Connection was lost"); break;
      case WL_SCAN_COMPLETED: Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED: Serial.println("[WiFi] WiFi is disconnected"); break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        return true;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.println("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      return false;
    } else {
      numberOfTries--;
    }
  }
}

/*
* @brief Function to handle the set up of the connection to the firestore cloud database service
* 
*/
bool setupFirestore() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  UserAuth user_auth(API_KEY, email, account_password);
  Serial.println("Initializing app...");

  ssl_client.setInsecure();

  authHandler();

  // Binding the FirebaseApp for authentication handler.
  // To unbind, use Docs.resetApp();
  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);

  app.getApp<Firestore::Documents>(Docs);

  // In case setting the external async result to the sync task (optional)
  // To unset, use unsetAsyncResult().
  aClient.setAsyncResult(aResult_no_callback);

  verified = verifyUser(API_KEY, email, account_password);

  return verified;
}


/*
* @brief Function to verify the existence of the user
* @return true if the user exists, false otherwise
*/
bool verifyUser(const String& apiKey, const String& email, const String& password) {
  if (ssl_client.connected())
    ssl_client.stop();

  String host = "www.googleapis.com";
  bool ret = false;

  if (ssl_client.connect(host.c_str(), 443) > 0) {
    String payload = "{\"email\":\"";
    payload += email;
    payload += "\",\"password\":\"";
    payload += password;
    payload += "\",\"returnSecureToken\":true}";

    String header = "POST /identitytoolkit/v3/relyingparty/verifyPassword?key=";
    header += apiKey;
    header += " HTTP/1.1\r\n";
    header += "Host: ";
    header += host;
    header += "\r\n";
    header += "Content-Type: application/json\r\n";
    header += "Content-Length: ";
    header += payload.length();
    header += "\r\n\r\n";

    if (ssl_client.print(header) == header.length()) {
      if (ssl_client.print(payload) == payload.length()) {
        unsigned long ms = millis();
        while (ssl_client.connected() && ssl_client.available() == 0 && millis() - ms < 5000) {
          delay(1);
        }

        ms = millis();
        while (ssl_client.connected() && ssl_client.available() && millis() - ms < 5000) {
          String line = ssl_client.readStringUntil('\n');
          if (line.length()) {
            ret = line.indexOf("HTTP/1.1 200 OK") > -1;
            break;
          }
        }
        ssl_client.stop();
      }
    }
  }

  return ret;
}


/*
* @brief Function to handle the firebase authentication
* 
*/
void authHandler() {
  // Blocking authentication handler with timeout
  unsigned long ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000) {
    // The JWT token processor required for ServiceAuth and CustomAuth authentications.
    // JWT is a static object of JWTClass and it's not thread safe.
    // In multi-threaded operations (multi-FirebaseApp), you have to define JWTClass for each FirebaseApp,
    // and set it to the FirebaseApp via FirebaseApp::setJWTProcessor(<JWTClass>), before calling initializeApp.
    JWT.loop(app.getAuth());
    printResult(aResult_no_callback);
  }
}

/*
* @brief Function to print the result of the request
*
*/
void printResult(AsyncResult& aResult) {
  if (aResult.isEvent()) {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available()) {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}

/*
* @brief Function to print the error of the request
*/
void printError(int code, const String& msg) {
  Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

/*
* @brief Function to receive and save the project informations
*/
void ReceiveandSaveProjectInformations() {
  // Read the API KEY from serial
  Serial.print("Enter API_KEY: ");
  while (API_KEY == "") {
    if (Serial.available()) {
      API_KEY = Serial.readStringUntil('\n');
      ssid.trim();
    }
  }
  Serial.print("API_KEY = " + API_KEY + "\n");

  // Read the Firebase PROJECT ID from serial
  Serial.print("Enter FIREBASE_PROJECT_ID: ");
  while (FIREBASE_PROJECT_ID == "") {
    if (Serial.available()) {
      FIREBASE_PROJECT_ID = Serial.readStringUntil('\n');
      ssid.trim();
    }
  }
  Serial.print("FIREBASE_PROJECT_ID= " + FIREBASE_PROJECT_ID);

  // Save the information read in the SPIFFS
  File file = SPIFFS.open("/project_info.txt", "w");
  if (!file) {
    Serial.println("Failed to open project_info.txt for writing");
    return;
  }
  file.println(API_KEY);
  file.println(FIREBASE_PROJECT_ID);
  file.close();

  // Reboot the ESP
  Serial.println("Project info saved successfully, reboot...");
  ESP.restart();
}

/*
* @brief Function to read the project informations from file
* @return true if the informations are read correctly, false otherwise
*/
bool readProjectInformations() {
  // Read project information from SPIFFS
  if (SPIFFS.exists("/project_info.txt")) {
    File file = SPIFFS.open("/project_info.txt", "r");
    if (!file) {
      Serial.println("Failed to open file for reading");
      return false;
    }

    API_KEY = file.readStringUntil('\n');
    API_KEY.trim();
    if (API_KEY == "") {
      Serial.println("API_KEY is empty!");
      return false;
    }
    Serial.println("API_KEY read = " + API_KEY);

    FIREBASE_PROJECT_ID = file.readStringUntil('\n');
    FIREBASE_PROJECT_ID.trim();
    if (FIREBASE_PROJECT_ID == "") {
      Serial.println("FIREBASE_PROJECT_ID is empty!");
      return false;
    }
    Serial.println("FIREBASE_PROJECT_ID read = " + FIREBASE_PROJECT_ID);

    return true;
  } else {
    Serial.println("The file project_info.txt does not exists");
    return false;
  }
}

/*
*@brief Function to setup the ntp connection
*/
void setupNTP() {
  // Configure the NTP client
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void setupUART() {
  Serial2.begin(115200);
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
/*
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Errore nel recuperare l'ora corrente");
    return;
  }
  Serial.println(&timeinfo, "Ora corrente: %A, %B %d %Y %H:%M:%S");
}
*/

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

bool checkDocumentExists()
{
    String documentPath = "systems/" + systemId;

  Serial.println("Checking document... ");
  String payload = Docs.get(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, GetDocumentOptions(DocumentMask("id")));

  if (aClient.lastError().code() == 0) {
    Serial.println("Document exists: " + payload);
    return true;
  } else {
    Serial.println("Document does not exist or error occurred");
    printError(aClient.lastError().code(), aClient.lastError().message());
    return false;
  }
}

bool isLinkedUser() {
    Serial.println("Get the linker document");
    String documentPath = "users/" + userUid + "/systems/" + systemId;
    String payload = Docs.get(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, GetDocumentOptions(DocumentMask("id")));

    if (aClient.lastError().code() == 0) {
        // Parsing the JSON string
        jsonParser.setJsonData(payload);

        // Check if the document contains the system ID
        if (jsonParser.get(jsonData, "id") && jsonData.stringValue == systemId) {
            Serial.println("User is linked to the system.");
            return true;
        } else {
            Serial.println("User is not linked to the system.");
            return false;
        }
    } else {
        printError(aClient.lastError().code(), aClient.lastError().message());
        return false;
    }
}

int setNumBells() {
  return 0;
}

int setNumMelodies() {
  return 0;
}

bool linkUser() {
  Serial.println("Linking user with UID = " + userUid);
  String doc_path = "users/" + userUid + "/systems";

  Values::StringValue nameV("");
  Values::StringValue idV(systemId);
  Values::StringValue locationV("Genova");

  Document<Values::Value> doc("location", Values::Value(locationV));
  doc.add("id", Values::Value(idV)).add("name", Values::Value(nameV));

  Serial.println("Create link system & user document... ");
  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path, systemId, DocumentMask(), doc);

  Serial.println(payload);
}

bool createSystemDocument() {
  String doc_path = "systems";

  Values::IntegerValue bellsV(bellsNum);
  Values::IntegerValue melodiesV(melodiesNum);
  Values::StringValue nameV("");
  Values::StringValue defaultV("");

  // Obtain the current time from NTP server
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return false; // For fail
  }

  // Format the timestamp
  char timestamp[21];
  sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ",
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  Values::TimestampValue timeV(timestamp);

  Document<Values::Value> doc("#bells", Values::Value(bellsV));
  doc.add("#melodies", Values::Value(melodiesV)).add("name", Values::Value(nameV)).add("time", Values::Value(timeV)).add("id", Values::Value(defaultV));

  Serial.println("Create document... ");
  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path, DocumentMask("name"), doc);

  if (aClient.lastError().code() == 0) {
    Serial.println("Document with system information created!");
    // return true; // For success
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    // return false; // For fail
  }

  jsonParser.setJsonData(payload);

  if (jsonParser.get(jsonData, "name")) {
    String nameValue = jsonData.stringValue;
    Serial.print("Name: ");
    Serial.println(nameValue);

    // Extracting the document ID from the 'name' value
    int lastSlashIndex = nameValue.lastIndexOf('/');
    if (lastSlashIndex != -1) {
      systemId = nameValue.substring(lastSlashIndex + 1);
      Serial.print("Document ID: ");
      Serial.println(systemId);

      // Update the id field in the document
      Values::StringValue idV(systemId);
      Document<Values::Value> update("id", Values::Value(idV));

      PatchDocumentOptions patchOptions(DocumentMask("id"), DocumentMask(), Precondition());

      // You can set the content of doc object directly with doc.setContent("your content")
      Serial.println(doc_path + "/" + systemId);
      String payload = Docs.patch(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path + "/" + systemId, patchOptions, update);

      if (aClient.lastError().code() == 0) {
        Serial.println("Update field id success");
        return true; // For success
      }
      else
        printError(aClient.lastError().code(), aClient.lastError().message());
        return false; // For fail
    }
  }
}
//#########################################################################################################










//#########################################################################################################
//SETUP//
void setup() {
  Serial.begin(115200);

  Serial.println("Mounting SPIFF...");
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.disconnect(true);  // This line will clear any previous WiFi configurations
  delay(1000);

  if (readProjectInformations()) {
    if (!connectToWifi()) {
      startAccessPoint();
    } else {
      setupNTP();   // Network Time Protocol
      setupUART();  // Universal Asynchronous Receiver-Transmitter (seriale)
      setupFirestore();
    }
  } else {
    ReceiveandSaveProjectInformations();
  }
  bellsNum = setNumBells();
  melodiesNum = setNumMelodies();
  // Poi queste bisogna metterle nello spiffs
}
//#########################################################################################################################






//#########################################################################################################################
//LOOP//
void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    authHandler();

    app.loop();
    Docs.loop();

    if (first_time && app.ready() && app.isInitialized()) {
      Serial.println("User verified:");
      Serial.println(verified);
      first_time = !first_time;

      userUid = app.getUid();

      File file = SPIFFS.open("/system_info.txt", "w");
      if (!file) {
        Serial.println("Failed to open project_info.txt for writing");
        return;
      }

      // Usiamo systemId nella create document, che la prendiamo dal file. se è vuota ok e se invece non è vuota dovrebbe darci errore.

      // Si aggiunge come parametro delle funzioni il sistem ID.
      // Durante la prima accensione si da ID = 0, almeno crea un ID automatico.
        if (createSystemDocument()) {
          if (linkUser()) {
            // Save the db information in the system_info.txt
            file.println(systemId);
            file.println(bellsNum);
            file.println(melodiesNum);
            file.println(userUid);
            file.close();
          }
          else {
            Serial.println("Document already existed or error occurred.");
          }
        }
        else {
          Serial.println("Document already existed or error occurred.");
        }

      // Qui invece si dà il vero systemId che si prende dallo SPIFFS 
      // (e magari si salva coem variabile globale)
      // Si vanno a creare i documenti con quell'ID, se esistono già mi aspetto
      // che il DB si arrabbi e che me lo faccia sapere in qualche modo
    }
    
    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();

        // Should run the Create_Documents.ino prior to test this example to create the documents in the collection Id at a0/b0/c0

        // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id id in the document b0.
        String collectionId = "systems/KkEsJ6nVzUb4SSZhLANG/events";

        // If the collection Id path contains space e.g. "a b/c d/e f"
        // It should encode the space as %20 then the collection Id will be "a%20b/c%20d/e%20f"

        Serial.println("List the documents in a collection... ");

        ListDocumentsOptions listDocsOptions;
        listDocsOptions.pageSize(100);

        String payload = Docs.list(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, listDocsOptions);

        if (aClient.lastError().code() == 0)
            Serial.println(payload);
        else
            printError(aClient.lastError().code(), aClient.lastError().message());
    }

    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0)) {
      dataMillis = millis();

      // Should run the Create_Documents.ino prior to test this example to create the documents in the collection Id at a0/b0/c0

      // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id id in the document b0.
      String collectionId = "systems/KkEsJ6nVzUb4SSZhLANG/events";

      // If the collection Id path contains space e.g. "a b/c d/e f"
      // It should encode the space as %20 then the collection Id will be "a%20b/c%20d/e%20f"

      Serial.println("List the documents in a collection... ");

      ListDocumentsOptions listDocsOptions;
      listDocsOptions.pageSize(100);

      String payload = Docs.list(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, listDocsOptions);

      if (aClient.lastError().code() == 0)
        Serial.println(payload);
      else
        printError(aClient.lastError().code(), aClient.lastError().message());
    }

    if (millis() - last_time_sent > 5000) {
      currentTimeSending();
      last_time_sent = millis();
    }
  } else server.handleClient();
}
//#############################################################################################################################################
