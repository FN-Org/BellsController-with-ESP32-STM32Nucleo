//#####################################################################################
// Includes//
#include <Arduino.h>

// SPIFFS
#include <SPIFFS.h>
#include <FS.h>

// Vector
#include <vector>

// Include header files
#include "setup.h"
#include "web_server.h"
#include "firebase.h"
#include "utils.h"

//########################################################################################
// Global Variables//
#define SENDING_INTERVAL 120000
// Other global variables
bool first_time = true;
bool setupCompleted = false;

// Global variables for the time
// Set time zone (UTC+1 for Italy)
const long gmtOffset_sec = 3600;      // UTC +1 (3600 seconds)
const int daylightOffset_sec = 3600;  // Ora legale


unsigned long dataMillis = 0;
unsigned long last_time_sent = 0;


//#########################################################################################################
// SETUP//
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
    File file = SPIFFS.open("/system_info.txt", "r");

    if (!connectToWifi() || !setupFirebase() || file.size() <= 0) {
      setupCompleted = false;
      Serial.println(file.size());
      startAccessPoint();
    } else {
      setupCompleted = true;
      setupNTP();   // Network Time Protocol
      setupUART();  // Universal Asynchronous Receiver-Transmitter (seriale)
    }
  } else {
    ReceiveandSaveProjectInformations();
  }
}
//#########################################################################################################################






//#########################################################################################################################
// LOOP//
void loop() {

  if (WiFi.status() == WL_CONNECTED && setupCompleted) {

    authHandler();

    app.loop();
    Docs.loop();
    storage.loop();

    if (first_time && app.ready() && app.isInitialized()) {
      Serial.println("User verified:");
      Serial.println(verified);
      first_time = !first_time;

      userUid = app.getUid();
      Serial.println("User UID in the loop: " + userUid);

      readSystemInfo();  // It reads the system information from the file in the SPIFFS (system_info.txt)

      // Usiamo systemId nella create document, che la prendiamo dal file. se è vuota ok e se invece non è vuota dovrebbe darci errore.

      // Si aggiunge come parametro delle funzioni il sistem ID.
      // Durante la prima accensione si da ID = 0, almeno crea un ID automatico.
      if (createSystemDocument()) {
        if (linkUser()) {
          Serial.println("Fatto LINKUSER");
          sendSystemInfo();
        } else {
          Serial.println("Document already existed or error occurred.");
        }
      } else {
        Serial.println("Document already existed or error occurred.");
      }

      fetchMelodies(); // Fetch melodies from firestore db
      delay(100);
      currentTimeSending();
    }

    if (app.ready() && (millis() - dataMillis > SENDING_INTERVAL || dataMillis == 0)) {
      dataMillis = millis();

      // Should run the Create_Documents.ino prior to test this example to create the documents in the collection Id at a0/b0/c0

      // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id id in the document b0.
      String collectionId = "systems/" + systemId + "/events";

      // If the collection Id path contains space e.g. "a b/c d/e f"
      // It should encode the space as %20 then the collection Id will be "a%20b/c%20d/e%20f"

      Serial.println("Searching for events to be deleted...");
      deleteOldEvents();

      Serial.println("List the documents in a collection... ");

      ListDocumentsOptions listDocsOptions;
      listDocsOptions.pageSize(100);
      listDocsOptions.orderBy("time asc");

      String payload = Docs.list(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, listDocsOptions);



      if (aClient.lastError().code() == 0) {
        Serial.println("sending events...");

        String payloadCleaned = removeSpacesAndNewlines(payload);

        Serial.println(payloadCleaned);

        Serial2.println("---");
        delay(100);
        Serial2.println("-E-");
        
        sendPackets(256, payloadCleaned);

        Serial2.println("---");
      } else
        printError(aClient.lastError().code(), aClient.lastError().message());
    }

    if (millis() - last_time_sent> SENDING_INTERVAL*10) {
      currentTimeSending();
      last_time_sent = millis();
    }
  } else server.handleClient();
}
//#############################################################################################################################################
