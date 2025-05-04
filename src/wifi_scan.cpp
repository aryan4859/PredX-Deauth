#include "wifi_scan.h"

int channelRSSI[14] = {0};

String getScanResults() {
  int n = WiFi.scanNetworks();
  String json = "[";

  for (int i = 0; i < n; ++i) {
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"bssid\":\"" + WiFi.BSSIDstr(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"channel\":" + String(WiFi.channel(i)) + ",";
    json += "\"encryption\":" + String(WiFi.encryptionType(i));
    json += "}";
    if (i != n - 1) json += ",";
  }

  json += "]";
  return json;
}

void analyzeSpectrum() {
  for (int ch = 1; ch <= 13; ch++) {
    WiFi.disconnect();
    delay(10);
    wifi_set_channel(ch);
    delay(200);

    int maxRSSI = -100;

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      int rssi = WiFi.RSSI(i);
      if (rssi > maxRSSI) {
        maxRSSI = rssi;
      }
    }

    channelRSSI[ch] = maxRSSI;
    WiFi.scanDelete();
  }
}
