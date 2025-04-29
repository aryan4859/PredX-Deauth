#include <Arduino.h>
#include "web_server.h"

void setup() {
  Serial.begin(115200);
  setupWebServer();  // Initializes AP, FS, routes, etc.
}

void loop() {
  handleClient();  // Handles web requests
}
