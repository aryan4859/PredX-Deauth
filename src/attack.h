#ifndef ATTACK_H
#define ATTACK_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

// Function to send deauthentication packets
void sendDeauth(const String& targetAP, const String& targetClient);

#endif