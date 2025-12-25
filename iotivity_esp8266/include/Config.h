#pragma once

// === WiFi ===
#define WIFI_SSID     "PTIT.HCM_SV"
#define WIFI_PASS     ""

// === API Server === (PC IP + Flask port)
#define SERVER_HOST   "10.251.5.213" 
#define SERVER_PORT   5000
#define SERVER_ENDPOINT "/sensor"

// === DHT configuration ===
#define DHT_PIN   5        // NodeMCU D1 = GPIO5
#define DHT_TYPE  11       // DHT11

// === Read interval ===
#define READ_INTERVAL_MS 5000
