#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "esp_wifi.h"

// Initialize the asynchronous web server engine on standard HTTP Port 80
AsyncWebServer server(80);

// Global operational counter variables
unsigned long totalPacketsSniffed = 0;
int currentChannel = 1;

// Base configuration template structure for raw Espressif packet data
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

// Promiscuous hardware callback function for logging baseline packet metrics
void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    totalPacketsSniffed++;
}

// HTML & CSS markup structure string for the on-device web management interface
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Diagnostic Console</title>
    <style>
        body { font-family: Arial, Helvetica, sans-serif; text-align: center; background-color: #1a1a1a; color: #ffffff; margin: 0; padding: 20px; }
        .card { background-color: #2d2d2d; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.3); max-width: 400px; margin: 40px auto; }
        h1 { color: #00ffcc; font-size: 24px; }
        .stat { font-size: 48px; font-weight: bold; margin: 20px 0; color: #ffcc00; }
        .meta { color: #aaaaaa; font-size: 14px; }
    </style>
    <script>
        // Poll the server every 1 second to fetch the live package counters dynamically
        setInterval(function() {
            fetch('/stats').then(response => response.json()).then(data => {
                document.getElementById('packetCount').innerText = data.packets;
                document.getElementById('currentChan').innerText = data.channel;
            });
        }, 1000);
    </script>
</head>
<body>
    <div class="card">
        <h1>ESP32 Radio Analyzer</h1>
        <div class="meta">Diagnostic Mode Status: Active</div>
        <div class="stat" id="packetCount">0</div>
        <div class="meta">Raw IEEE 802.11 Frames Processed</div>
        <p class="meta">Monitoring Radio Channel: <span id="currentChan" style="color:#00ffcc; font-weight:bold;">1</span></p>
    </div>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[!] Initializing Asynchronous Security Dashboard...");

    // 1. Boot up an open Access Point local network zone
    WiFi.softAP("ESP32-Diagnostic-Console", "");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("[+] Access Point Live. Gateway Web Address: http://");
    Serial.println(IP);

    // 2. Initialize the background promiscuous auditing engine configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_promiscuous_rx_cb(sniffer_callback);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

    // 3. Define the web server URI interface endpoints
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/stats", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{\"packets\":" + String(totalPacketsSniffed) + ",\"channel\":" + String(currentChannel) + "}";
        request->send(200, "application/json", json);
    });

    // 4. Fire up the web service engine listeners
    server.begin();
    Serial.println("[+] Asynchronous HTTP Server Engine Started Successfully.");
}

void loop() {
    // Simple automated channel-hopping logic loop sequence (Channels 1 to 11)
    static unsigned long lastChannelSwitch = 0;
    if (millis() - lastChannelSwitch > 5000) { // Hop every 5 seconds
        lastChannelSwitch = millis();
        currentChannel++;
        if (currentChannel > 11) currentChannel = 1;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
}
