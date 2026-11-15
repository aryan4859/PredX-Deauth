#include <esp_wifi.h>
#include <stdio.h>
#include <string.h>
#include "monitor.h"
#include "definitions.h"

namespace {
const size_t EVENT_CAPACITY = 32;
const size_t AP_CAPACITY = 32;
const uint8_t SECURITY_OPEN = 0;
const uint8_t SECURITY_WEP = 1;
const uint8_t SECURITY_WPA = 2;

struct AccessPointRecord {
  bool used;
  uint8_t security;
  ObservedAccessPoint ap;
};

portMUX_TYPE monitor_mux = portMUX_INITIALIZER_UNLOCKED;
SecurityEvent events[EVENT_CAPACITY] = {};
AccessPointRecord access_points[AP_CAPACITY] = {};
size_t event_head = 0;
size_t event_count = 0;
size_t ap_replacement = 0;
uint32_t deauth_count = 0;
uint8_t active_channel = MONITOR_CHANNEL;
bool active = false;

void copy_text(char *destination, size_t capacity, const char *source) {
  if (capacity == 0) return;
  snprintf(destination, capacity, "%s", source ? source : "");
}

void format_mac(const uint8_t *mac, char *out, size_t capacity) {
  snprintf(out, capacity, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void record_event_locked(const char *type, const char *severity, const char *ssid,
                         const char *bssid, bool simulated) {
  const uint32_t now = millis();
  if (!simulated && (strcmp(type, "DEAUTH") == 0 || strcmp(type, "DISASSOC") == 0)) {
    if (deauth_count != UINT32_MAX) ++deauth_count;
  }

  if (!simulated && event_count > 0) {
    const size_t last = (event_head + EVENT_CAPACITY - 1) % EVENT_CAPACITY;
    SecurityEvent &previous = events[last];
    if (!previous.simulated && strcmp(previous.type, type) == 0 &&
        strcmp(previous.bssid, bssid) == 0 &&
        static_cast<uint32_t>(now - previous.timestamp_ms) <= 3000) {
      previous.timestamp_ms = now;
      if (previous.count != UINT16_MAX) ++previous.count;
      return;
    }
  }

  SecurityEvent &event = events[event_head];
  memset(&event, 0, sizeof(event));
  event.timestamp_ms = now;
  copy_text(event.type, sizeof(event.type), type);
  copy_text(event.severity, sizeof(event.severity), severity);
  copy_text(event.ssid, sizeof(event.ssid), ssid);
  copy_text(event.bssid, sizeof(event.bssid), bssid);
  event.count = 1;
  event.simulated = simulated;
  event_head = (event_head + 1) % EVENT_CAPACITY;
  if (event_count < EVENT_CAPACITY) ++event_count;
}

const char *security_name(uint8_t security) {
  switch (security) {
    case SECURITY_OPEN: return "Open";
    case SECURITY_WEP: return "WEP";
    default: return "WPA/RSN";
  }
}

void observe_beacon(const uint8_t *frame, size_t length, int8_t rssi, uint8_t channel) {
  if (length < 36) return;

  const uint16_t capabilities = frame[34] | (static_cast<uint16_t>(frame[35]) << 8);
  uint8_t security = (capabilities & 0x0010) ? SECURITY_WEP : SECURITY_OPEN;
  char ssid[33] = {};
  bool has_ssid = false;

  size_t offset = 36;
  while (offset + 2 <= length) {
    const uint8_t id = frame[offset];
    const uint8_t element_length = frame[offset + 1];
    offset += 2;
    if (offset + element_length > length) break;

    if (id == 0 && !has_ssid && element_length <= 32) {
      memcpy(ssid, frame + offset, element_length);
      ssid[element_length] = '\0';
      has_ssid = element_length > 0;
    } else if (id == 48 ||
               (id == 221 && element_length >= 4 &&
                frame[offset] == 0x00 && frame[offset + 1] == 0x50 &&
                frame[offset + 2] == 0xF2 && frame[offset + 3] == 0x01)) {
      security = SECURITY_WPA;
    }
    offset += element_length;
  }

  char bssid[18];
  format_mac(frame + 16, bssid, sizeof(bssid));

  portENTER_CRITICAL(&monitor_mux);
  int existing = -1;
  int available = -1;
  bool security_mismatch = false;
  for (size_t i = 0; i < AP_CAPACITY; ++i) {
    if (access_points[i].used && strcmp(access_points[i].ap.bssid, bssid) == 0) {
      existing = static_cast<int>(i);
      break;
    }
    if (!access_points[i].used && available < 0) available = static_cast<int>(i);
    if (has_ssid && access_points[i].used &&
        strcmp(access_points[i].ap.ssid, ssid) == 0 &&
        access_points[i].security != security) {
      security_mismatch = true;
    }
  }

  if (existing >= 0) {
    AccessPointRecord &record = access_points[existing];
    if (record.security != security && has_ssid) {
      record_event_locked("SECURITY CHANGE", "warning", ssid, bssid, false);
    }
    record.security = security;
    if (has_ssid) copy_text(record.ap.ssid, sizeof(record.ap.ssid), ssid);
    record.ap.rssi = rssi;
    record.ap.channel = channel;
    copy_text(record.ap.security, sizeof(record.ap.security), security_name(security));
  } else {
    const size_t slot = available >= 0 ? static_cast<size_t>(available) : ap_replacement;
    ap_replacement = (slot + 1) % AP_CAPACITY;
    AccessPointRecord &record = access_points[slot];
    memset(&record, 0, sizeof(record));
    record.used = true;
    record.security = security;
    copy_text(record.ap.ssid, sizeof(record.ap.ssid), has_ssid ? ssid : "<hidden>");
    copy_text(record.ap.bssid, sizeof(record.ap.bssid), bssid);
    copy_text(record.ap.security, sizeof(record.ap.security), security_name(security));
    record.ap.rssi = rssi;
    record.ap.channel = channel;

    if (security_mismatch) {
      record_event_locked("SSID SECURITY MISMATCH", "warning", ssid, bssid, false);
    }
  }
  portEXIT_CRITICAL(&monitor_mux);
}

void packet_callback(void *buffer, wifi_promiscuous_pkt_type_t type) {
  if (!buffer || type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t *packet = static_cast<wifi_promiscuous_pkt_t *>(buffer);
  const uint8_t *frame = packet->payload;
  const size_t length = packet->rx_ctrl.sig_len;
  if (length < 24) return;

  const uint16_t frame_control = frame[0] | (static_cast<uint16_t>(frame[1]) << 8);
  if ((frame_control & 0x000C) != 0) return;
  const uint16_t subtype = frame_control & 0x00FC;

  if (subtype == 0x00C0 || subtype == 0x00A0) {
    char bssid[18];
    format_mac(frame + 16, bssid, sizeof(bssid));
    portENTER_CRITICAL(&monitor_mux);
    record_event_locked(subtype == 0x00C0 ? "DEAUTH" : "DISASSOC",
                        "warning", "", bssid, false);
    portEXIT_CRITICAL(&monitor_mux);
  } else if ((subtype == 0x0080 || subtype == 0x0050) && length >= 36) {
    observe_beacon(frame, length, packet->rx_ctrl.rssi, packet->rx_ctrl.channel);
  }
}
}

bool start_monitor(uint8_t channel) {
  if (channel < 1 || channel > 13) return false;

  esp_wifi_set_promiscuous(false);
  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;

  esp_err_t result = esp_wifi_set_promiscuous_filter(&filter);
  if (result != ESP_OK) {
    Serial.printf("Unable to configure management-frame filter: %d\n", result);
    return false;
  }
  result = esp_wifi_set_promiscuous_rx_cb(&packet_callback);
  if (result != ESP_OK) {
    Serial.printf("Unable to register monitor callback: %d\n", result);
    return false;
  }
  result = esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  if (result != ESP_OK) {
    Serial.printf("Unable to select monitoring channel: %d\n", result);
    return false;
  }
  result = esp_wifi_set_promiscuous(true);
  if (result != ESP_OK) {
    Serial.printf("Unable to enable passive monitor: %d\n", result);
    return false;
  }

  portENTER_CRITICAL(&monitor_mux);
  active_channel = channel;
  active = true;
  portEXIT_CRITICAL(&monitor_mux);
  return true;
}

void stop_monitor() {
  const esp_err_t result = esp_wifi_set_promiscuous(false);
  if (result != ESP_OK) {
    Serial.printf("Unable to stop passive monitor: %d\n", result);
  }
  portENTER_CRITICAL(&monitor_mux);
  active = false;
  portEXIT_CRITICAL(&monitor_mux);
}

bool monitor_is_active() {
  portENTER_CRITICAL(&monitor_mux);
  const bool result = active;
  portEXIT_CRITICAL(&monitor_mux);
  return result;
}

uint8_t monitor_channel() {
  portENTER_CRITICAL(&monitor_mux);
  const uint8_t result = active_channel;
  portEXIT_CRITICAL(&monitor_mux);
  return result;
}

uint32_t deauth_frame_count() {
  portENTER_CRITICAL(&monitor_mux);
  const uint32_t result = deauth_count;
  portEXIT_CRITICAL(&monitor_mux);
  return result;
}

size_t get_access_points(ObservedAccessPoint *out, size_t capacity) {
  if (!out || capacity == 0) return 0;
  size_t copied = 0;
  portENTER_CRITICAL(&monitor_mux);
  for (size_t i = 0; i < AP_CAPACITY && copied < capacity; ++i) {
    if (access_points[i].used) out[copied++] = access_points[i].ap;
  }
  portEXIT_CRITICAL(&monitor_mux);
  return copied;
}

size_t get_security_events(SecurityEvent *out, size_t capacity) {
  if (!out || capacity == 0) return 0;
  size_t copied = 0;
  portENTER_CRITICAL(&monitor_mux);
  const size_t count = event_count < capacity ? event_count : capacity;
  const size_t oldest = (event_head + EVENT_CAPACITY - event_count) % EVENT_CAPACITY;
  const size_t skip = event_count - count;
  for (size_t i = 0; i < count; ++i) {
    out[copied++] = events[(oldest + skip + i) % EVENT_CAPACITY];
  }
  portEXIT_CRITICAL(&monitor_mux);
  return copied;
}

void clear_security_events() {
  portENTER_CRITICAL(&monitor_mux);
  memset(events, 0, sizeof(events));
  event_head = 0;
  event_count = 0;
  portEXIT_CRITICAL(&monitor_mux);
}

void add_simulated_event() {
  portENTER_CRITICAL(&monitor_mux);
  record_event_locked("SIMULATION", "info", "Deauth burst example",
                      "TRAINING ONLY", true);
  portEXIT_CRITICAL(&monitor_mux);
}