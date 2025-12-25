#pragma once

// === WiFi ===
#define WIFI_SSID     "Your Wifi"
#define WIFI_PASS     "Password"

// === API Server === (PC IP + Flask port)
#define SERVER_HOST   "192.168.1.3" 
#define SERVER_PORT   5000
#define SERVER_ENDPOINT "/sensor"

// === DHT configuration ===
#define DHT_PIN   5        // NodeMCU D1 = GPIO5
#define DHT_TYPE  11       // DHT11

// === Read interval ===
#define READ_INTERVAL_MS 5000
