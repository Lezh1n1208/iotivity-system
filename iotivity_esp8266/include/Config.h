#pragma once

// === WiFi ===
#define WIFI_SSID     "An Binh Lau4 -1"
#define WIFI_PASS     "anbinh123"

// === API Server === (PC IP + Flask port)
#define SERVER_HOST   "192.168.1.2" 
#define SERVER_PORT   5000
#define SERVER_ENDPOINT "/sensor"

// === DHT configuration ===
#define DHT_PIN   5        // NodeMCU D1 = GPIO5
#define DHT_TYPE  11       // DHT11

// === Read interval ===
#define READ_INTERVAL_MS 5000
