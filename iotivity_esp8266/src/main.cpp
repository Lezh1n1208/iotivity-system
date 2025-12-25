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
    Serial.printf("Target Server: http://%s:%d%s\n", SERVER_HOST, SERVER_PORT, SERVER_ENDPOINT);
    Serial.printf("WiFi SSID: %s\n", WIFI_SSID);

    NetClient::begin(WIFI_SSID, WIFI_PASS);

    unsigned long t0 = millis();
    while (!WiFi.isConnected() && millis() - t0 < 15000) {
        Serial.print(".");
        delay(500);
    }
    
    Serial.println();
    if (WiFi.isConnected()) {
        Serial.println("✅ WiFi connected!");
        Serial.print("   ESP8266 IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("   Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("   Signal: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("❌ WiFi connection FAILED!");
        Serial.println("   Check SSID/Password in Config.h");
    }

    Serial.println("\nInitializing DHT sensor...");
    sensor.begin();
    Serial.println("✅ DHT ready\n");
}

void loop() {
    // Kiểm tra WiFi
    if (!WiFi.isConnected()) {
        Serial.println("⚠️  WiFi disconnected! Reconnecting...");
        NetClient::ensureConnected();
        delay(1000);
        return;
    }

    unsigned long now = millis();
    if (now - lastRead >= READ_INTERVAL_MS) {
        lastRead = now;

        Serial.println("─────────────────────────────");
        Serial.printf("[%lu] Reading sensor...\n", now);
        
        SensorReading r = sensor.read();
        if (!r.ok) {
            Serial.println("❌ DHT read FAILED (check wiring)");
            Serial.println("   Data pin: GPIO5 (D1)");
            Serial.println("   Power: 3.3V");
            return;
        }

        Serial.printf("✅ Sensor OK: T=%.2f°C  H=%.2f%%\n", r.temperature, r.humidity);
        Serial.printf("Posting to http://%s:%d%s\n", SERVER_HOST, SERVER_PORT, SERVER_ENDPOINT);
        
        bool sent = sender.postReading(r);
        
        if (sent) {
            Serial.println("✅ HTTP POST SUCCESS");
        } else {
            Serial.println("❌ HTTP POST FAILED");
            Serial.println("   Possible causes:");
            Serial.println("   - Server not reachable");
            Serial.println("   - Firewall blocking port 5000");
            Serial.println("   - Wrong IP in Config.h");
        }
        Serial.println();
    }

    delay(10);
}