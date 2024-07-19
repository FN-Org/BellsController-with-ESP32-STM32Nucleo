#include "firebase.h"

// Global variables for connecting to the cloud database (stored in /project_info.txt)
String API_KEY = "";
String FIREBASE_PROJECT_ID = "";

//Global variables for connecting to the cloud storage (stored in /project_info.txt)
String STORAGE_BUCKET_ID = "";

// Global variables for the firebase library
WiFiClientSecure ssl_client;
DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;
Storage storage;
AsyncResult aResult_no_callback;
FirebaseApp app;

FirebaseJson jsonParser;
FirebaseJsonData jsonData;

String userUid = "";
String systemId = "";

bool verified = false;

int melodiesNum = 0;
std::vector<String> melodiesNames;

/*
* @brief Function to handle the set up of the connection to the firestore cloud database service
* 
*/
bool setupFirebase() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  UserAuth user_auth(API_KEY, email, account_password);
  Serial.println("Initializing app...");

  Serial.println("Sono qui 1");

  ssl_client.setInsecure();

  Serial.println("Sono qui 2");

  authHandler();
  // Binding the FirebaseApp for authentication handler.
  // To unbind, use Docs.resetApp();
  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
  
  Serial.println("Sono qui 3");

  app.getApp<Firestore::Documents>(Docs);
  app.getApp<Storage>(storage);

  Serial.println("Sono qui 4");

  // In case setting the external async result to the sync task (optional)
  // To unset, use unsetAsyncResult().
  aClient.setAsyncResult(aResult_no_callback);

  Serial.println("Sono qui 5");

  verified = verifyUser(API_KEY, email, account_password);

  Serial.println("Sono qui 6: " + String(verified));

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

bool linkUser() {
  Serial.println("Linking user with UID = " + userUid);
  String doc_path = "users/" + userUid + "/systems/" + systemId;

  Values::StringValue nameV(name);
  Values::StringValue idV(systemId);
  Values::StringValue locationV(location);

  Document<Values::Value> doc("location", Values::Value(locationV));
  doc.add("id", Values::Value(idV)).add("name", Values::Value(nameV));

  Serial.println("Create link system & user document... ");
  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path, DocumentMask(), doc);

  if (aClient.lastError().code() == 0) {
    Serial.println("System & user linked");
    return true;  // For success
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return false;  // For fail
  }
}

bool createSystemDocument() {
  String doc_path = "systems/" + systemId;
  Serial.println("Percorso del documento: " + doc_path);

  Values::IntegerValue bellsV(bellsNum);
  Values::IntegerValue melodiesV(default_Nmelodies);
  Values::StringValue nameV(name);
  Values::StringValue locationV(location);
  Values::IntegerValue pinV(pin);
  Values::StringValue defaultV(systemId);

  // Obtain the current time from NTP server
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return false;
  }

  // Format the timestamp
  char timestamp[25];  // Buffer for the formatted timestamp (adjust size as needed)
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

  Values::TimestampValue timeV(timestamp);

  Document<Values::Value> doc("nBells", Values::Value(bellsV));
  doc.add("nMelodies", Values::Value(melodiesV)).add("name", Values::Value(nameV)).add("time", Values::Value(timeV)).add("id", Values::Value(defaultV));
  doc.add("location", Values::Value(locationV)).add("pin", Values::Value(pinV));

  try {
    Serial.println("Create document... ");
    String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path, DocumentMask("name"), doc);

    if (aClient.lastError().code() == 0) {
      Serial.println("Document with system information created!");

      jsonParser.setJsonData(payload);

      if (jsonParser.get(jsonData, "name")) {
        String nameValue = jsonData.stringValue;
        Serial.print("Name: ");
        Serial.println(nameValue);

        // Extracting the document ID from the 'name' value
        int lastSlashIndex = nameValue.lastIndexOf('/');
        if (lastSlashIndex != -1) {
          systemId = nameValue.substring(lastSlashIndex + 1);
          Serial.print("Document ID: " + systemId);
          String doc_path = "systems/" + systemId;
          Serial.println("Nuovo percorso del documento: " + doc_path);

          // Update the id field in the document
          Values::StringValue idV(systemId);
          Document<Values::Value> update("id", Values::Value(idV));

          PatchDocumentOptions patchOptions(DocumentMask("id"), DocumentMask(), Precondition());

          // You can set the content of doc object directly with doc.setContent("your content")
          Serial.println(doc_path + systemId);
          String payload = Docs.patch(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), doc_path, patchOptions, update);

          if (aClient.lastError().code() == 0) {
            Serial.println("Update field id success");
            return true;  // For success
          } else {
            printError(aClient.lastError().code(), aClient.lastError().message());
            return false;  // For fail
          }
        }
      } else {
        printError(aClient.lastError().code(), aClient.lastError().message());
        return false;  // For fail
      }
    }
  } catch (std::exception& e) {
    Serial.print("Exception caught: ");
    Serial.println(e.what());
    // Handle the exception as needed
    return false;  // Indicate failure
  }
}

void fetchMelodies() {
  Serial.println("Get all files...");

  String startingPath = "melodies/" + String(bellsNum) + "/";

  int cnt = 1;
  bool result = true;

  String path = startingPath + String(cnt) + ".txt";

  String buff = "buf.txt";

  FileConfig media_file(buff, fileCallback);

  while (result) {
    Serial.print("Count");
    Serial.println(cnt);
    path = startingPath + String(cnt) + ".txt";
    result = storage.download(aClient, FirebaseStorage::Parent(STORAGE_BUCKET_ID, path), getFile(media_file));

    cnt++;
    if (result) {
      Serial.println("Object downloaded.");
      readAndSendBuffer(cnt);
    } else
      printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

void fileCallback(File& file, const char* filename, file_operating_mode mode) {
  // FILE_OPEN_MODE_READ, FILE_OPEN_MODE_WRITE and FILE_OPEN_MODE_APPEND are defined in this library
  // MY_FS is defined in this example

  Serial.print("Mode:");
  Serial.println(mode);
  Serial.print("Name:");
  Serial.println(filename);
  switch (mode) {
    case file_mode_open_read:
      file = SPIFFS.open(filename, FILE_OPEN_MODE_READ);
      break;
    case file_mode_open_write:
      file = SPIFFS.open(filename, FILE_OPEN_MODE_WRITE);
      break;
    case file_mode_open_append:
      file = SPIFFS.open(filename, FILE_OPEN_MODE_APPEND);
      break;
    case file_mode_remove:
      SPIFFS.remove(filename);
      break;
    default:
      break;
  }
}

void updateMelodies() {

  String starting_path = "systems/" + systemId + "/melodies/";

  String path = starting_path;

  for (int i = 0; i < melodiesNames.size(); i++) {

    path = starting_path + melodiesNames[i];
    Serial.println("Create document: " + path);
    Values::IntegerValue num(i + 1);
    Values::StringValue name(melodiesNames[i]);

    Document<Values::Value> doc("name", Values::Value(name));
    doc.add("number", Values::Value(num));

    String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), path, DocumentMask(), doc);

    if (aClient.lastError().code() == 0) {
      Serial.println("Created doc " + path + "with success");
      //send to the stm32
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }
  }

  //update the SPIFFS
  saveSystemInfo(name, location, bellsNum, melodiesNum, pin);
  File file = SPIFFS.open("/system_info.txt", "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  file.println(systemId);
  file.close();

  //update the firestore db
  path = "systems/" + systemId;
  Values::IntegerValue nMelodies(melodiesNum);
  Document<Values::Value> update("nMelodies", Values::Value(nMelodies));

  PatchDocumentOptions patchOptions(DocumentMask("nMelodies"), DocumentMask(), Precondition());

  // You can set the content of doc object directly with doc.setContent("your content")
  String payload = Docs.patch(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), path, patchOptions, update);

  if (aClient.lastError().code() == 0) {
    Serial.println("Update of the melodies number on the db successful");
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

void readAndSendBuffer(int cnt) {
  File testFile = SPIFFS.open("/buf.txt", "r");
  if (!testFile) {
    Serial.println("File /buf.txt doesn't exists!");
    return;
  }
  Serial2.println("---");
  delay(100);
  Serial2.println("-M-");
  delay(700);

  String melodyTitle = testFile.readStringUntil('\n');
  melodyTitle.trim();
  Serial2.println(melodyTitle);
  Serial.println(melodyTitle);
  melodiesNames.push_back(melodyTitle);
  delay(100);
  String line;
  while (testFile.available()) {
    line = testFile.readStringUntil('\n');
    line.trim();
    Serial2.println(line);
    delay(100);
  }
  Serial2.println("---");
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

  // Read the Firebase BUCKET ID from serial
  Serial.print("Enter STORAGE_BUCKET_ID: ");
  while (STORAGE_BUCKET_ID == "") {
    if (Serial.available()) {
      STORAGE_BUCKET_ID = Serial.readStringUntil('\n');
      ssid.trim();
    }
  }
  Serial.print("STORAGE_BUCKET_ID= " + STORAGE_BUCKET_ID);

  // Save the information read in the SPIFFS
  File file = SPIFFS.open("/project_info.txt", "w");
  if (!file) {
    Serial.println("Failed to open project_info.txt for writing");
    return;
  }
  file.println(API_KEY);
  file.println(FIREBASE_PROJECT_ID);
  file.println(STORAGE_BUCKET_ID);
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

    STORAGE_BUCKET_ID = file.readStringUntil('\n');
    STORAGE_BUCKET_ID.trim();
    if (STORAGE_BUCKET_ID == "") {
      Serial.println("STORAGE_BUCKET_ID is empty!");
      return false;
    }
    Serial.println("STORAGE_BUCKET_ID read = " + STORAGE_BUCKET_ID);

    return true;
  } else {
    Serial.println("The file project_info.txt does not exists");
    return false;
  }
}

void readSystemInfo() {
  File file = SPIFFS.open("/system_info.txt", "r");
  if (!file) {
    Serial.println("Failed to open system_info.txt for reading");
    return;
  }

  Serial.println("system_info.txt document:");
  int lineNumber = 0;
  String line;
  while (file.available()) {
    line = file.readStringUntil('\n');
    line.trim();  // Rimuove spazi bianchi iniziali e finali
    lineNumber++;
    switch (lineNumber) {
      case 1:
        name = line;
        Serial.println("name: " + line);
        break;
      case 2:
        location = line;
        Serial.println("location: " + line);
        break;
      case 3:
        bellsNum = line.toInt();
        Serial.println("bells number: " + line);
        break;
      case 4:
        melodiesNum = line.toInt();
        Serial.println("melodies number: " + line);
        break;
      case 5:
        pin = line.toInt();
        Serial.println("pin: " + line);
        break;
      case 6:
        systemId = line;
        Serial.println("system id: " + line);
        break;
      default:
        break;
    }
  }
  file.close();
}