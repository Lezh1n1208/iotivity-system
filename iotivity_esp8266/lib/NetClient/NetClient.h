#pragma once
#include <Arduino.h>

class NetClient {
public:
    static void begin(const char* ssid, const char* pass);
    static bool connected();
    static void ensureConnected();
};
