#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "DHTSensor.h"
#include "NetClient.h"
#include "Sender.h"

DHTSensor sensor(DHT_PIN, DHT_TYPE);
Sender sender(SERVER_HOST, SERVER_PORT, SERVER_ENDPOINT);

unsigned long lastRead = 0;

void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println("\n=== ESP8266 IoTivity Sensor ===");
    Serial.printf("Connecting WiFi: %s\n", WIFI_SSID);

    NetClient::begin(WIFI_SSID, WIFI_PASS);

    unsigned long t0 = millis();
    while (!WiFi.isConnected() && millis() - t0 < 10000) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("ESP8266 IP Address: ");
    Serial.println(WiFi.localIP());

    sensor.begin();
}

void loop() {
    NetClient::ensureConnected();

    unsigned long now = millis();
    if (now - lastRead >= READ_INTERVAL_MS) {
        lastRead = now;

        SensorReading r = sensor.read();
        if (!r.ok) {
            Serial.println("DHT read failed");
            return;
        }

        Serial.printf("T=%.2f°C  H=%.2f%%\n", r.temperature, r.humidity);
        bool sent = sender.postReading(r);
        Serial.printf("Sent: %s\n", sent ? "OK" : "FAIL");
    }

    delay(10);
}
