#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"
#include "buzzer.h"

class TelemetryServer {
public:
    TelemetryServer();
    void begin();
    void update(const SensorData& data, BsmState state, uint32_t now);
    uint32_t getClientCount() const;

private:
    void broadcastTelemetry(const SensorData& data, BsmState state, uint32_t now);

    AsyncWebServer _server;
    AsyncWebSocket _ws;
    uint32_t _lastBroadcastMs;
    uint32_t _packetCounter;
};
