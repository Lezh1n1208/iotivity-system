#pragma once
#include <Arduino.h>

struct SensorReading {
    bool ok;
    float temperature;
    float humidity;
    unsigned long timestamp;
};

class DHTSensor {
public:
    DHTSensor(uint8_t pin, uint8_t type);
    void begin();
    SensorReading read();

private:
    uint8_t _pin;
    uint8_t _type;
};
