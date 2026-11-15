#ifndef TYPES_H
#define TYPES_H

struct ObservedAccessPoint {
  char ssid[33];
  char bssid[18];
  char security[8];
  int8_t rssi;
  uint8_t channel;
};

struct SecurityEvent {
  uint32_t timestamp_ms;
  char type[20];
  char severity[12];
  char ssid[33];
  char bssid[18];
  uint16_t count;
  bool simulated;
};

#endif