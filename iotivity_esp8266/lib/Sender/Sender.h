#pragma once
#include <Arduino.h>
#include "DHTSensor.h"

class Sender {
public:
    Sender(const char* host, uint16_t port, const char* endpoint);
    bool postReading(const SensorReading &r);

private:
    const char* _host;
    uint16_t _port;
    const char* _endpoint;
};
