#ifndef WIFI_SCAN_H
#define WIFI_SCAN_H

#include <ESP8266WiFi.h>

String getScanResults();
void analyzeSpectrum();

extern int channelRSSI[14];

#endif
