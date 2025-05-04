#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "wifi_scan.h"
#include "attack.h"

ESP8266WebServer webServer(80);
String scanResults;

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

void handleScan() {
  scanResults = getScanResults();
  webServer.send(200, "application/json", scanResults);
}

void handleSpectrum(){
  analyzeSpectrum();

  String jsonResponse = "{";
  for (int i = 1; i <= 13; i++) {
    jsonResponse += "\"" + String(i) + "\": " + String(channelRSSI[i]) + ",";
  }
  jsonResponse.remove(jsonResponse.length() - 1);  // Remove the trailing comma
  jsonResponse += "}"; 

  webServer.send(200, "application/json", jsonResponse);
}

void handleStop() {
  scanResults = "<p>Scanning Stopped</p>";
  webServer.send(200, "text/plain", "Stopped Scanning!");
}

void handleDeauth() {
  if (!webServer.hasArg("ap") || !webServer.hasArg("client")) {
      webServer.send(400, "text/plain", "Missing parameters: ap or client");
      return;
  }

  String targetAP = webServer.arg("ap");
  String targetClient = webServer.arg("client");

  sendDeauth(targetAP, targetClient);
  webServer.send(200, "text/plain", "Deauth attack initiated!");
}


void setupWebServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PredX", "predx1337");

  Serial.println("Access Point started with SSID: PredX and Password: predx1337");
  Serial.println("AP IP Address: " + WiFi.softAPIP().toString());

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }


  webServer.on("/", handleIndex);
  webServer.on("/scan", handleScan);
  webServer.on("/stop", handleStop);
  webServer.on("/spectrum", handleSpectrum);
  webServer.on("/deauth", handleDeauth);

  webServer.begin();
  Serial.println("Web server started!");
}

void handleClient() {
  webServer.handleClient();
}
