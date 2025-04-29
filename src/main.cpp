#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h> // LittleFS

ESP8266WebServer server(80);

bool attackRunning = false;
String targetSSID = "";
int currentChannel = 1; // Channel for Wi-Fi operations

// Access point structure
struct AccessPoint {
  String essid;
  int channel;
  uint8_t deauthPacket[26]; // Placeholder for deauth packet
  bool found;
};

AccessPoint access_points[50]; // Array to store discovered APs
int current = -1;

// Function prototypes
void handleScan();
void handleStartAttack();
void handleStopAttack();
void sendDeauth(String ssid);
void scan();
void promisc_cb(uint8_t* buf, uint16_t len);
void clean_ap_list();

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
      Serial.println("Could not open file for read");
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
  scan(); // Perform a scan for access points
  String list = "";
  for (int i = 0; i <= current; ++i) {
    list += "SSID: " + access_points[i].essid + ", Channel: " + String(access_points[i].channel) + "\n";
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
  Serial.println("Sending deauth to: " + ssid);

  // Find the target AP in the list
  for (int i = 0; i <= current; i++) {
    if (access_points[i].essid == ssid) {
      // Send deauth packet (placeholder functionality)
      Serial.print("Deauthenticating clients from AP: ");
      Serial.print(access_points[i].essid);
      Serial.print(" on channel ");
      Serial.println(access_points[i].channel);
      // Real deauth logic (e.g., packet injection) goes here
    }
  }
}

void scan() {
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  Serial.println("[!] Scanning for APs...");

  for (int i = 0; i <= current; i++)
    access_points[i].found = false;
  
  for (int i = 0; i < 2; i++) {
    for (int p = 0; p < 13; p++) { // Wi-Fi channels 1-13
      currentChannel = p + 1;
      wifi_set_channel(currentChannel);
      
      Serial.print("Scanning channel: ");
      Serial.println(currentChannel);
      
      delay(500); // Allow time for packet capture
    }
  }

  Serial.println("[!] Done scanning");
  clean_ap_list();
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(0);
}

// Promiscuous mode callback to process captured packets
void promisc_cb(uint8_t* buf, uint16_t len) {
  // Process packets to identify access points
  // This function should parse beacon frames and extract SSIDs, channels, etc.
  // Placeholder implementation
  Serial.println("Packet received in promiscuous mode");
}

// Clean up AP list after scan
void clean_ap_list() {
  int index = 0;
  for (int i = 0; i <= current; i++) {
    if (access_points[i].found) {
      access_points[index] = access_points[i];
      index++;
    }
  }
  current = index - 1;
  Serial.println("Cleaned up AP list");
}