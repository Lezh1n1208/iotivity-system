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
}

bool NetClient::connected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetClient::ensureConnected() {
    if (connected()) return;

    WiFi.begin(_ssid, _pass);

    unsigned long start = millis();
    while (!connected() && millis() - start < 8000) {
        delay(300);
    }
}
