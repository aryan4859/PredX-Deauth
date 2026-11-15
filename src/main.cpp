#include <WiFi.h>
#include "web_interface.h"
#include "monitor.h"
#include "definitions.h"

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif

  WiFi.mode(WIFI_MODE_AP);
  if (!WiFi.softAP(AP_SSID, AP_PASS, MONITOR_CHANNEL)) {
    Serial.println("Unable to start the monitoring access point.");
  }

  start_web_interface();
  if (!start_monitor(MONITOR_CHANNEL)) {
    Serial.println("Passive Wi-Fi monitoring could not be started.");
  }
}

void loop() {
  web_interface_handle_client();
  delay(2);
}