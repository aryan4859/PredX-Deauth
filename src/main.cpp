#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h> // LittleFS

ESP8266WebServer server(80);

bool attackRunning = false;
String targetSSID = "";

// Function prototypes
void handleScan();
void handleStartAttack();
void handleStopAttack();
void sendDeauth(String ssid);

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS!");
  } else {
    Serial.println("LittleFS mounted successfully!");
  }

  // Create Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PredX", "predx1337");

  // Web routes
  server.on("/", HTTPMethod::HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
  
    if (!file) {
      Serial.println("could not open file for read");
      server.send(500, "application/json",
                  "{\"error\":\"could not open file\"}");
    } else {
      server.streamFile(file, "text/html");
      file.close();
    }
  });

  server.on("/scan", HTTP_GET, handleScan);
  server.on("/start", HTTP_GET, handleStartAttack);
  server.on("/stop", HTTP_GET, handleStopAttack);

  server.begin();
  Serial.println("Webserver started");
}

void loop() {
  server.handleClient();

  if (attackRunning) {
    sendDeauth(targetSSID);
    delay(100); // Little delay to avoid flooding
  }
}

// Function definitions
void handleScan() {
  int n = WiFi.scanNetworks();
  String list = "";
  for (int i = 0; i < n; ++i) {
    list += WiFi.SSID(i) + "\n";
  }
  server.send(200, "text/plain", list);
}

void handleStartAttack() {
  if (server.hasArg("ssid")) {
    targetSSID = server.arg("ssid");
    attackRunning = true;
    server.send(200, "text/plain", "Started attack on " + targetSSID);
  } else {
    server.send(400, "text/plain", "SSID not provided");
  }
}

void handleStopAttack() {
  attackRunning = false;
  server.send(200, "text/plain", "Stopped attack");
}

void sendDeauth(String ssid) {
  // Very basic pseudo function
  // In reality, you need packet injection
  Serial.println("Sending deauth to: " + ssid);

  // (Real packet injection would happen here)
}