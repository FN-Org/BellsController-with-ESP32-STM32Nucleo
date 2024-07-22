#include "web_server.h"

/********** GLOBAL VARIABLES **********/

// Global varaibles for wifi and autentication (stored in /credentials.txt)
// Credential Form
String ssid = "";
String wifi_password = "";
String email = "";
String account_password = "";

// Global varaibles for system information (stored in /system_info.txt)
// System Form
String name = "";
String location = "";
int bellsNum = 0;
int melodiesNum = 0;
int pin = 0;
String systemId = "";

// Global variables for the access point
const char* ap_ssid = "ESP32_AP";
const char* ap_password = "12345678";

// Selecting the port 80 for the server (standard http port)
WebServer server(80);

const int default_Nmelodies = 0;

/********** FUNCTIONS **********/

/*
* @brief Function to handle the connection to the Access Point
* 
*/
void handleCredentialsForm() {
  String htmlForm = "<!DOCTYPE html><html><head><style>"
                    "body {background-color: #007BFF; font-family: Arial, sans-serif;}"
                    ".container {background-color: white; padding: 20px; margin: auto; width: 50%; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}"
                    "input[type=text], input[type=password] {width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}"
                    "input[type=submit] {width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer;}"
                    "input[type=submit]:hover {background-color: #45a049;}"
                    "label {font-weight: bold;}"
                    "</style></head><body>"
                    "<div class=\"container\">"
                    "<form action=\"/first_submit\" method=\"post\">"
                    "<label for=\"ssid\">SSID:</label><br>"
                    "<input type=\"text\" id=\"ssid\" name=\"ssid\" required><br>"
                    "<label for=\"wifi_password\">WiFi Password:</label><br>"
                    "<input type=\"text\" id=\"wifi_password\" name=\"wifi_password\" required><br>"
                    "<label for=\"email\">Email:</label><br>"
                    "<input type=\"text\" id=\"email\" name=\"email\" required><br>"
                    "<label for=\"account_password\">Account Password:</label><br>"
                    "<input type=\"text\" id=\"account_password\" name=\"account_password\" required><br><br>"
                    "<input type=\"submit\" value=\"Submit\">"
                    "</form></div>"
                    "</body></html>";
  server.send(200, "text/html", htmlForm);
}

void handleSystemForm() {
  String htmlForm = "<!DOCTYPE html><html><head><style>"
                    "body {background-color: #007BFF; font-family: Arial, sans-serif;}"
                    ".container {background-color: white; padding: 20px; margin: auto; width: 50%; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}"
                    "input[type=text], input[type=password] {width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}"
                    "input[type=submit] {width: 100%; background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer;}"
                    "input[type=submit]:hover {background-color: #45a049;}"
                    "label {font-weight: bold;}"
                    "</style></head><body>"
                    "<div class=\"container\">"
                    "<form action=\"/second_submit\" method=\"post\">"
                    "<label for=\"num_campane\">Numero Campane:</label><br>"
                    "<input type=\"number\" id=\"num_campane\" name=\"num_campane\" required><br>"
                    "<label for=\"name\">Name:</label><br>"
                    "<input type=\"text\" id=\"name\" name=\"name\" required><br>"
                    "<label for=\"location\">Location:</label><br>"
                    "<input type=\"text\" id=\"location\" name=\"location\" required><br>"
                    "<label for=\"pin\">PIN:</label><br>"
                    "<input type=\"number\" id=\"pin\" name=\"pin\" required><br><br>"
                    "<input type=\"submit\" value=\"Submit\">"
                    "</form></div>"
                    "</body></html>";
  server.send(200, "text/html", htmlForm);
}

/*
* @brief Function to handle the submit of the form when connected to the access point
* 
*/
void handleCredentialsSubmit() {
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

  // Redirect to the second form
  server.sendHeader("Location", "/second_form");
  server.send(303);
}

void handleSystemSubmit() {
  bellsNum = server.arg("num_campane").toInt();
  name = server.arg("name");
  location = server.arg("location");
  pin = server.arg("pin").toInt();

  // Save details (implement saveDetails function as needed)
  saveSystemInfo(name, location, bellsNum, default_Nmelodies, pin);

  // Print details for debugging
  Serial.println("Name: " + name);
  Serial.println("Location: " + location);
  Serial.println("Numero Campane: " + String(bellsNum));
  Serial.println("PIN: " + String(pin));

  server.send(200, "text/html", "Details received. Thank you!");
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
  server.on("/", handleCredentialsForm);
  server.on("/first_submit", HTTP_POST, handleCredentialsSubmit);
  server.on("/second_form", handleSystemForm);
  server.on("/second_submit", handleSystemSubmit);
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

void saveSystemInfo(const String& name, const String& location, const int& bNum, const int& mNum, const int& pin) {
  File file = SPIFFS.open("/system_info.txt", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.println(name);
  file.println(location);
  file.println(bNum);
  file.println(mNum);
  file.println(pin);
  file.println(systemId);
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
