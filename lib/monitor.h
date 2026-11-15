#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include "types.h"

bool start_monitor(uint8_t channel);
void stop_monitor();
bool monitor_is_active();
uint8_t monitor_channel();
uint32_t deauth_frame_count();
size_t get_access_points(ObservedAccessPoint *out, size_t capacity);
size_t get_security_events(SecurityEvent *out, size_t capacity);
void clear_security_events();
void add_simulated_event();

#endif
