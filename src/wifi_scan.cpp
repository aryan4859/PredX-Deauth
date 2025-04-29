#include "wifi_scan.h"

String getScanResults() {
  int n = WiFi.scanNetworks();
  String result = "<ul>";
  for (int i = 0; i < n; ++i) {
    result += "<li>SSID: " + WiFi.SSID(i) + " | Signal: " + String(WiFi.RSSI(i)) + " dBm</li>";
  }
  result += "</ul>";
  return result;
}
