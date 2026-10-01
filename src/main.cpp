#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

void scan_networks() {
    Serial.println("[*] Scanning for Wi-Fi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("[-] No networks found.");
    } else {
        Serial.printf("[+] %d networks found:\n", n);
        for (int i = 0; i < n; ++i) {
            Serial.printf("  %d: %s (RSSI: %d, Ch: %d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("[!] Custom ESP32 Firmware Starting...");

    // Initialize Wi-Fi in Station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    Serial.println("[+] Wi-Fi initialized. Ready for module injection.");
    scan_networks();
}

void loop() {
    // Keep running diagnostic loops or wait for user inputs
    delay(10000);
}
