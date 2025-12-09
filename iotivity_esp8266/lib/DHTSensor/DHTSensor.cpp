#include <Arduino.h>
#include "DHTSensor.h"
#include <DHT.h>

static DHT* dht = nullptr;

DHTSensor::DHTSensor(uint8_t pin, uint8_t type) : _pin(pin), _type(type) {}

void DHTSensor::begin() {
    if (!dht) {
        dht = new DHT(_pin, _type);
        dht->begin();
        delay(500);
    }
}

SensorReading DHTSensor::read() {
    SensorReading r;
    r.timestamp = millis();
    r.ok = false;

    float h = dht->readHumidity();
    float t = dht->readTemperature();

    if (isnan(h) || isnan(t)) {
        r.ok = false;
        r.temperature = 0;
        r.humidity = 0;
    } else {
        r.ok = true;
        r.temperature = t;
        r.humidity = h;
    }

    return r;
}
