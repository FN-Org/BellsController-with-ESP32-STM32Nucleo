#include "firebase.h"

// Global variables for connecting to the cloud database (stored in /project_info.txt)
String API_KEY = "";
String FIREBASE_PROJECT_ID = "";

String CLIENT_EMAIL = "";
String PRIVATE_KEY = "";

//Global variables for connecting to the cloud storage (stored in /project_info.txt)
String STORAGE_BUCKET_ID = "";

// Global variables for the firebase library
WiFiClientSecure ssl_client;
DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;
Storage storage;
Messaging messaging;

AsyncResult aResult_no_callback;
FirebaseApp app;
FirebaseApp appService;

FirebaseJson jsonParser;
FirebaseJsonData jsonData;
FirebaseJsonArray jsonArr;

JWTClass jwtProcessorUser;
JWTClass jwtProcessorService;

String userUid = "";

bool verified = false;

std::vector<String> melodiesNames;
std::vector<String> melodiesList;  // list to memorize all the melodies

/*
* @brief Function to handle the set up of the connection to the firestore cloud database service
* 
*/
bool setupFirebase() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  UserAuth user_auth(API_KEY, email, account_password);
  ServiceAuth sa_auth(timeStatusCB, CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);
  Serial.println("Initializing app...");

  Serial.println("Sono qui 1");

  ssl_client.setInsecure();

  Serial.println("Sono qui 2");

  app.setJWTProcessor(jwtProcessorUser);
  appService.setJWTProcessor(jwtProcessorService);

  authHandler();
  // Binding the FirebaseApp for authentication handler.
  // To unbind, use Docs.resetApp();
  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
  initializeApp(aClient, appService, getAuth(sa_auth), aResult_no_callback);

  Serial.println("Sono qui 3");

  app.getApp<Firestore::Documents>(Docs);
  app.getApp<Storage>(storage);
  appService.getApp<Messaging>(messaging);

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
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000 && appService.isInitialized() && !appService.ready()) {
    // The JWT token processor required for ServiceAuth and CustomAuth authentications.
    // JWT is a static object of JWTClass and it's not thread safe.
    // In multi-threaded operations (multi-FirebaseApp), you have to define JWTClass for each FirebaseApp,
    // and set it to the FirebaseApp via FirebaseApp::setJWTProcessor(<JWTClass>), before calling initializeApp.
    jwtProcessorUser.loop(app.getAuth());
    printResult(aResult_no_callback);
    jwtProcessorService.loop(appService.getAuth());
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
    delay(100);

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
        Serial.println("Failed to extract document ID");
        return false;
      }  // For fail
      jsonParser.clear();
      jsonData.clear();  // clear the parser
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
      return false;
    }  // For fail
  } catch (std::exception& e) {
    Serial.print("Exception caught: ");
    Serial.println(e.what());
    // Handle the exception as needed
    return false;  // Indicate failure
  }
}

void fetchMelodies() {
  Serial.println("Get all files...");

  melodiesList.clear();
  melodiesNames.clear();
  // int cnt = 1;

  // Fetch melodies based on bells number
  for (int i = 3; i <= bellsNum; i++) {
    // The index starts from 3 because it is the minimum number of bells
    // in a bell tower to reproduce a melody
    String startingPath = "melodies/" + String(i) + "/";

    int title = 1;
    bool result = true;

    while (result) {
      Serial.print("db Title: ");
      Serial.println(title);
      String path = startingPath + String(title) + ".txt";
      Serial.print("Downloading from path: ");
      Serial.println(path);
      String buff = "buf.txt";

      FileConfig media_file(buff, fileCallback);

      result = storage.download(aClient, FirebaseStorage::Parent(STORAGE_BUCKET_ID, path), getFile(media_file));

      if (result) {
        Serial.println("Object downloaded.");
        File testFile = SPIFFS.open("/buf.txt", "r");
        if (testFile) {
          String fileContent = testFile.readString();  // Leggi tutto il contenuto del file
          melodiesList.push_back(fileContent);         // Aggiungi il contenuto alla lista
          testFile.close();
        } else {
          Serial.println("Failed to open the downloaded file.");
        }
        title++;
        // cnt++;
      } else {
        printError(aClient.lastError().code(), aClient.lastError().message());
      }
    }
  }

  // Fetch personal melodies
  String startingPath = "melodies/" + systemId + "/";
  int title = 1;
  bool result = true;
  while (result) {
    Serial.print("db Title: ");
    Serial.println(title);
    String path = startingPath + String(title) + ".txt";
    Serial.print("Downloading from path: ");
    Serial.println(path);
    String buff = "buf.txt";

    FileConfig media_file(buff, fileCallback);

    result = storage.download(aClient, FirebaseStorage::Parent(STORAGE_BUCKET_ID, path), getFile(media_file));

    if (result) {
      Serial.println("Object downloaded.");
      File testFile = SPIFFS.open("/buf.txt", "r");
      if (testFile) {
        String fileContent = testFile.readString();  // Leggi tutto il contenuto del file
        melodiesList.push_back(fileContent);         // Aggiungi il contenuto alla lista
        testFile.close();
      } else {
        Serial.println("Failed to open the downloaded file.");
      }
      title++;
      // cnt++;
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }
  }

  // Manda sempre le melodie
  for (const String& melody : melodiesList) {
    // Salva il contenuto in "buf.txt"
    File file = SPIFFS.open("/buf.txt", "w");
    if (file) {
      file.print(melody);
      file.close();
      delay(1000);
      readAndSendBuffer();  // Invia la melodia
    } else {
      Serial.println("Failed to open /buf.txt for writing.");
    }
  }

  updateDBMelodies();
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

void updateDBMelodies() {

  String starting_path = "systems/" + systemId + "/melodies/";

  String path = starting_path;

  deleteMelodies(starting_path, melodiesNames);

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
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }
  }

  melodiesNum = melodiesNames.size();

  // Update the SPIFFS
  saveSystemInfo(name, location, bellsNum, melodiesNum, pin);
  File file = SPIFFS.open("/system_info.txt", "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  file.println(systemId);
  file.close();

  // Update the firestore db for the number of melodies
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

void deleteMelodies(String documentPath, std::vector<String> melodiesName) {

  for (size_t i = 0; i < melodiesName.size(); i++) {
    // Costruisce il percorso completo del documento da eliminare
    String fullPath = documentPath + melodiesName[i];

    // Esegue l'eliminazione del documento
    auto payload = Docs.deleteDoc(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), fullPath, Precondition());

    // Controlla se l'eliminazione è avvenuta con successo
    if (aClient.lastError().code() == 0) {
      Serial.println("Deleted: " + fullPath);
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }
  }
}

void syncOnDB() {
  // Definisci il percorso del documento
  String documentPath = "systems/" + systemId;

  Values::BooleanValue boolV(true);

  // Crea un oggetto Document che rappresenta il contenuto da aggiornare
  Document<Values::Value> doc;
  doc.add("sync", Values::Value(boolV));  // Imposta il campo "sync" su true

  // Crea l'oggetto PatchDocumentOptions
  PatchDocumentOptions patchOptions(DocumentMask("sync"), DocumentMask(), Precondition());

  // Esegui l'aggiornamento del documento
  Serial.println("Updating the sync field in the document... ");
  String payload = Docs.patch(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, patchOptions, doc);

  // Verifica se l'aggiornamento è avvenuto con successo
  if (aClient.lastError().code() == 0) {
    Serial.println("Sync field updated successfully.");
    Serial.println(payload);
  } else {
    printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

/*
void saveTitleInSPIFFS(String title) {
  File file = SPIFFS.open("/melody_titles.txt", "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  file.println(title);
  file.close();
}

void readMelodyTitles() {
  File testFile = SPIFFS.open("/melody_titles.txt", "r");
  if (!testFile) {
    Serial.println("File /buf.txt doesn't exists!");
    return;
  }
  while (testFile.available()) {
    String melodyTitle = testFile.readStringUntil('\n');
    melodiesNames.push_back(melodyTitle);
  }
  testFile.close();
}
*/

void readAndSendBuffer() {
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
  // saveTitleInSPIFFS(melodyTitle);
  delay(100);
  String line;
  while (testFile.available()) {
    line = testFile.readStringUntil('\n');
    line.trim();
    Serial2.println(line);
    delay(100);
  }
  testFile.close();
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
      API_KEY.trim();
    }
  }
  Serial.println("API_KEY = " + API_KEY);

  // Read the Firebase PROJECT ID from serial
  Serial.print("Enter FIREBASE_PROJECT_ID: ");
  while (FIREBASE_PROJECT_ID == "") {
    if (Serial.available()) {
      FIREBASE_PROJECT_ID = Serial.readStringUntil('\n');
      FIREBASE_PROJECT_ID.trim();
    }
  }
  Serial.println("FIREBASE_PROJECT_ID= " + FIREBASE_PROJECT_ID);

  // Read the Firebase BUCKET ID from serial
  Serial.print("Enter STORAGE_BUCKET_ID: ");
  while (STORAGE_BUCKET_ID == "") {
    if (Serial.available()) {
      STORAGE_BUCKET_ID = Serial.readStringUntil('\n');
      STORAGE_BUCKET_ID.trim();
    }
  }
  Serial.println("STORAGE_BUCKET_ID= " + STORAGE_BUCKET_ID);

  // Read the Firebase CLIENT_EMAIL from serial
  Serial.print("Enter CLIENT_EMAIL: ");
  while (CLIENT_EMAIL == "") {
    if (Serial.available()) {
      CLIENT_EMAIL = Serial.readStringUntil('\n');
      CLIENT_EMAIL.trim();
    }
  }
  Serial.println("CLIENT_EMAIL= " + CLIENT_EMAIL);

  // Read the Firebase CLIENT_EMAIL from serial
  Serial.print("Enter PRIVATE_KEY: ");
  while (PRIVATE_KEY == "") {
    if (Serial.available()) {
      PRIVATE_KEY = Serial.readStringUntil('\n');
      PRIVATE_KEY.trim();
    }
  }
  Serial.println("PRIVATE_KEY= " + PRIVATE_KEY);

  // Save the information read in the SPIFFS
  File file = SPIFFS.open("/project_info.txt", "w");
  if (!file) {
    Serial.println("Failed to open project_info.txt for writing");
    return;
  }
  file.println(API_KEY);
  file.println(FIREBASE_PROJECT_ID);
  file.println(STORAGE_BUCKET_ID);
  file.println(CLIENT_EMAIL);
  file.println(PRIVATE_KEY);
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

    CLIENT_EMAIL = file.readStringUntil('\n');
    CLIENT_EMAIL.trim();
    if (CLIENT_EMAIL == "") {
      Serial.println("CLIENT_EMAIL is empty!");
      return false;
    }
    Serial.println("CLIENT_EMAIL read = " + CLIENT_EMAIL);

    PRIVATE_KEY = file.readStringUntil('\n');
    PRIVATE_KEY.trim();
    if (PRIVATE_KEY == "") {
      Serial.println("PRIVATE_KEY is empty!");
      return false;
    }
    Serial.println("PRIVATE_KEY read = " + PRIVATE_KEY);

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

void sendSystemInfo() {

  Serial2.println("---");
  delay(100);
  Serial2.println("-S-");
  Serial.println("-S-");
  delay(1000);

  Serial2.println(systemId);
  Serial.println(systemId);
  delay(100);
  Serial2.println(pin);
  Serial.println(pin);
  delay(100);
  Serial2.println("---");
}

void moveOldEvents(String payload) {
  String oldEventsDocPath = "systems/" + systemId + "/oldEvents";
  String eventsDocPath = "systems/" + systemId + "/events";
  //Serial.println(payload); //Debug

  bool problems = false;

  jsonArr.setJsonArrayData(payload);

  for (int i = 0; i < jsonArr.size(); i++) {
    jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/time/timestampValue");

    if (jsonData.success) {
      String timestamp = jsonData.stringValue;
      struct tm timeinfo;
      jsonData.clear();
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Error when trying to get the time.");
        return;
      }

      // Convert `struct tm` to ISO 8601 string
      char currentTime[30];
      strftime(currentTime, sizeof(currentTime), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
      Serial.println("Current Time: " + String(currentTime));
      Serial.println("Timestamp: " + timestamp);
      int res = strcmp(currentTime, timestamp.c_str());
      if (res > 0) {
        Values::TimestampValue timestampV(timestamp);

        Document<Values::Value> eventDoc("time", Values::Value(timestampV));

        // Getting and adding the number
        jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/color/integerValue");
        if (jsonData.success) {
          int color = jsonData.intValue;
          jsonData.clear();
          Values::IntegerValue colorV(color);

          eventDoc.add("color", Values::Value(colorV));

        } else {
          Serial.println("Failed to get the color.");
          problems = true;
        }

        // Getting and adding the melodyname
        jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/melodyName/stringValue");
        if (jsonData.success) {
          String melodyName = jsonData.stringValue;
          jsonData.clear();
          Values::StringValue melodyNameV(melodyName);
          eventDoc.add("melodyName", Values::Value(melodyNameV));

        } else {
          Serial.println("Failed to get the melodyName.");
          problems = true;
        }

        // Getting and adding the melodyNumber
        jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/melodyNumber/integerValue");
        if (jsonData.success) {
          int melodyNumber = jsonData.intValue;
          jsonData.clear();
          Values::IntegerValue melodyNumberV(melodyNumber);
          eventDoc.add("melodyNumber", Values::Value(melodyNumberV));

        } else {
          Serial.println("Failed to get the melodyNumber.");
          problems = true;
        }

        // Getting and adding the id and creating the new document
        jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/id/stringValue");
        if (jsonData.success) {
          String id = jsonData.stringValue;
          Serial.println("Event with id: " + id + String("Should be deleted!"));
          jsonData.clear();
          Values::StringValue idV(id);
          eventDoc.add("id", Values::Value(idV));
          if (!problems) {
            payload = Docs.deleteDoc(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), eventsDocPath + String("/") + id, Precondition());

            if (aClient.lastError().code() == 0)
              Serial.println("Deleted with success!");
            else
              printError(aClient.lastError().code(), aClient.lastError().message());

            String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), oldEventsDocPath + String("/") + id, DocumentMask(), eventDoc);
            if (aClient.lastError().code() == 0)
              Serial.println("Created with success!");
            else
              printError(aClient.lastError().code(), aClient.lastError().message());
          } else {
            Serial.println("Problems fetching document fields");
            continue;
          }
        } else {
          Serial.println("Failed to get the id.");
        }

      } else Serial.println("No events to be deleted.");
    } else {
      Serial.println("Failed to get the timestamp.");
    }
  }
  jsonArr.clear();
}

void timeStatusCB(uint32_t& ts) {
  if (time(nullptr) < FIREBASE_DEFAULT_TS) {

    configTime(3 * 3600, 0, "pool.ntp.org");
    while (time(nullptr) < FIREBASE_DEFAULT_TS) {
      delay(100);
    }
  }
  ts = time(nullptr);
}

void notifyFCM(String melodyName, std::vector<String> TokensFCM) {
  Messages::Message msg;
  Messages::Notification notification;
  Messages::AndroidConfig androidConfig;
  Messages::AndroidNotification androidNotification;

  for (int i = 0; i < TokensFCM.size(); i++) {
    msg.topic("event notification");
    msg.token(TokensFCM[i]);  // Registration token to send a message to

    // Basic notification
    notification.body("Event with melody: " + melodyName + "is now playing").title("Event executing");

    // Priority of a message to send to Android devices.
    // https://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages#androidmessagepriority
    androidConfig.priority(Messages::AndroidMessagePriority::_HIGH);

    // Set the relative priority for this notification.
    // Priority is an indication of how much of the user's attention should be consumed by this notification.
    // Low-priority notifications may be hidden from the user in certain situations,
    // while the user might be interrupted for a higher-priority notification.
    // https://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages#NotificationPriority
    androidNotification.notification_priority(Messages::NotificationPriority::PRIORITY_HIGH);

   androidConfig.notification(androidNotification);

    msg.android(androidConfig);

    msg.notification(notification);

    String payload = messaging.send(aClient, Messages::Parent(FIREBASE_PROJECT_ID), msg);

    if (aClient.lastError().code() == 0) {
      Serial.println("Message delivered to " + TokensFCM[i]);
      Serial.println(payload);
    } else
      printError(aClient.lastError().code(), aClient.lastError().message());
      Serial.println(payload);
  }
}

std::vector<String> getSystemTokensFCM() {
  String collectionId = "systems/" + systemId + "/tokensFCM";
  String payload = Docs.list(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), collectionId, ListDocumentsOptions());

  std::vector<String> Tokens;

  if (aClient.lastError().code() == 0) {
    jsonArr.setJsonArrayData(payload);
    for (int i = 0; i < jsonArr.size(); i++) {
      jsonArr.get(jsonData, "documents/[" + String(i) + "]/fields/token/stringValue");

      if (jsonData.success) {
        Tokens.push_back(jsonData.stringValue);
      } else Serial.println("Error when obtaining token");
    }


  } else
    printError(aClient.lastError().code(), aClient.lastError().message());
  Serial.println("Tokens obtained: " + String(Tokens.size()));

  return Tokens;
}
