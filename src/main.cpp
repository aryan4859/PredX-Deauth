#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

ESP8266WebServer webServer(80);

String scanResults;

// Function to scan available Wi-Fi networks
void scanNetworks() {
  int n = WiFi.scanNetworks();
  scanResults = "<ul>";
  for (int i = 0; i < n; ++i) {
    scanResults += "<li>SSID: " + WiFi.SSID(i) + " | Signal: " + String(WiFi.RSSI(i)) + " dBm</li>";
  }
  scanResults += "</ul>";
}

// Function to serve the main page
void handleIndex() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    webServer.send(500, "text/plain", "Failed to load index.html");
    return;
  }
  String html = file.readString();
  file.close();
  html.replace("{SCAN_RESULTS}", scanResults);
  webServer.send(200, "text/html", html);
}

// Function to handle the scan request
void handleScan() {
  scanNetworks();
  webServer.send(200, "text/plain", "Scan Complete!");
}

// Function to handle the stop request
void handleStop() {
  scanResults = "<p>Scanning Stopped</p>";
  webServer.send(200, "text/plain", "Stopped Scanning!");
}

void setup() {
  Serial.begin(115200); 
// Configure and start the ap
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PredX", "predx1337");
  Serial.println("Access Point started with SSID: PredX and Password: predx1337");
  Serial.println("AP IP Address: " + WiFi.softAPIP().toString());

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Define HTTP routes
  webServer.on("/", handleIndex);
  webServer.on("/scan", handleScan);
  webServer.on("/stop", handleStop);

  webServer.begin();
  Serial.println("Web server started!");
}

void loop() {
  webServer.handleClient();
}