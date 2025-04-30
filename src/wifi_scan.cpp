#include "wifi_scan.h"

int channelRSSI[14] = {0};

String getScanResults() {
  int n = WiFi.scanNetworks();
  String result = "<ul>";
  for (int i = 0; i < n; ++i) {
    result += "<li>SSID: " + WiFi.SSID(i) + " | Signal: " + String(WiFi.RSSI(i)) + " dBm</li>";
  }
  result += "</ul>";
  return result;
}

void analyzeSpectrum(){
  for (int ch = 1; ch <= 13; ch++){
    WiFi.disconnect();   
    delay(10);
    wifi_set_channel(ch);  
    delay(200);  

    int maxRSSI = -100;  // Start with the minimum possible RSSI value

    int n = WiFi.scanNetworks();  // Scan for available networks
    for (int i = 0; i < n; i++) {
      int rssi = WiFi.RSSI(i);  // Get the RSSI value for each network
      if (rssi > maxRSSI) {
        maxRSSI = rssi;  // Update the maxRSSI if the current one is higher
      }

  }

  channelRSSI[ch] = maxRSSI;
  WiFi.scanDelete();
}
}