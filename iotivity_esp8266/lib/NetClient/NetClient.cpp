#include <Arduino.h>
#include "NetClient.h"
#include <ESP8266WiFi.h>

static const char* _ssid;
static const char* _pass;

void NetClient::begin(const char* ssid, const char* pass) {
    _ssid = ssid;
    _pass = pass;

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _pass);
    Serial.printf("[NetClient] Connecting to '%s'...\n", _ssid);
}

bool NetClient::connected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetClient::ensureConnected() {
    if (connected()) return;

    Serial.println("[NetClient] Reconnecting WiFi...");
    WiFi.disconnect();
    delay(100);
    WiFi.begin(_ssid, _pass);

    unsigned long start = millis();
    while (!connected() && millis() - start < 10000) {
        Serial.print(".");
        delay(500);
    }
    
    if (connected()) {
        Serial.println("\n[NetClient] Reconnected!");
    } else {
        Serial.println("\n[NetClient] Reconnection failed!");
    }
}