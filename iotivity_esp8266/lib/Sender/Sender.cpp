#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Sender.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

Sender::Sender(const char* host, uint16_t port, const char* endpoint)
    : _host(host), _port(port), _endpoint(endpoint) {}

bool Sender::postReading(const SensorReading &r) {
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClient client;
    HTTPClient http;

    String url = String("http://") + _host + ":" + _port + _endpoint;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"timestamp\":" + String(r.timestamp) + ",";
    payload += "\"temperature\":" + String(r.temperature, 2) + ",";
    payload += "\"humidity\":" + String(r.humidity, 2);
    payload += "}";

    int code = http.POST(payload);
    bool ok = (code == 200 || code == 201);

    http.end();
    return ok;
}
