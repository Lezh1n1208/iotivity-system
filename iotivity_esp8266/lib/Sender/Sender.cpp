#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Sender.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

Sender::Sender(const char* host, uint16_t port, const char* endpoint)
    : _host(host), _port(port), _endpoint(endpoint) {}

bool Sender::postReading(const SensorReading &r) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("   [Sender] WiFi not connected");
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    String url = String("http://") + _host + ":" + _port + _endpoint;
    
    Serial.printf("   [Sender] Connecting to: %s\n", url.c_str());
    
    if (!http.begin(client, url)) {
        Serial.println("   [Sender] HTTP begin failed");
        return false;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000); // 5s timeout

    String payload = "{";
    payload += "\"timestamp\":" + String(r.timestamp) + ",";
    payload += "\"temperature\":" + String(r.temperature, 2) + ",";
    payload += "\"humidity\":" + String(r.humidity, 2);
    payload += "}";

    Serial.printf("   [Sender] Payload: %s\n", payload.c_str());

    int code = http.POST(payload);
    
    Serial.printf("   [Sender] HTTP Response: %d", code);
    if (code > 0) {
        Serial.printf(" (%s)\n", http.getString().c_str());
    } else {
        Serial.printf(" (Error: %s)\n", http.errorToString(code).c_str());
    }

    bool ok = (code == 200 || code == 201);
    http.end();
    
    return ok;
}