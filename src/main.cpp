#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

// Define a raw packet structure matching the Espressif driver output
struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned :1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssmatch0:1;
    unsigned bssmatch1:1;
    unsigned mcs:7;
    unsigned cwb:1;
    unsigned HT_length:16;
    unsigned smoothing:1;
    unsigned notch_cap:1;
    unsigned :2;
    unsigned agc_gain:6;
    unsigned lna_gain:3;
    unsigned noise_floor:8;
    unsigned sig_len:12;
    unsigned rx_state:8;
    unsigned dump_len:12;
};

// This callback function runs every time a raw Wi-Fi packet is caught in the air
void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    RxControl* control = (RxControl*)&pkt->rx_ctrl;

    // Check if the packet contains payload data
    if (pkt->rx_ctrl.sig_len > 0) {
        uint8_t* payload = pkt->payload;
        
        // Byte 0 of an IEEE 802.11 frame is the Frame Control field
        uint8_t frame_type = payload[0];

        Serial.print("[Raw Packet] Type: 0x");
        Serial.print(frame_type, HEX);
        Serial.print(" | Size: ");
        Serial.print(pkt->rx_ctrl.sig_len);
        Serial.print(" bytes | Signal (RSSI): ");
        Serial.println(control->rssi);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[!] Custom ESP32 Wireless Auditing Module Initializing...");

    // 1. Initialize the Wi-Fi storage configuration baseline
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // 2. Turn off normal link-state logic to prepare for raw operations
    esp_wifi_stop();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_start();

    // 3. Register our sniffer callback function
    esp_wifi_set_promiscuous_rx_cb(sniffer_callback);
    
    // 4. Enable Promiscuous Mode
    esp_wifi_set_promiscuous(true);
    
    // 5. Lock the hardware radio to Channel 1
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

    Serial.println("[+] Promiscuous Sniffer Active on Channel 1. Listening...");
}

void loop() {
    // The packet capture runs via background interrupts automatically.
    // Channel hopping or UI refreshes can be placed here later.
    delay(1000);
}
