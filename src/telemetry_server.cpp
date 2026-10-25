#include "telemetry_server.h"

static const char* stateToString(BsmState state) {
    switch (state) {
        case BsmState::SAFE_CLEAR: return "CLEAR";
        case BsmState::WARN_LEFT:  return "LEFT_HAZARD";
        case BsmState::WARN_RIGHT: return "RIGHT_HAZARD";
        case BsmState::WARN_BOTH:  return "DUAL_HAZARD";
        default:                   return "UNKNOWN";
    }
}

TelemetryServer::TelemetryServer()
    : _server(HTTP_PORT),
      _ws("/ws"),
      _lastBroadcastMs(0),
      _packetCounter(0) {}

void TelemetryServer::begin() {
    // 1. Initialize Wi-Fi SoftAP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    delay(100);

    IPAddress ip = WiFi.softAPIP();
    Serial.print(F("[WIFI] Access Point started: "));
    Serial.println(WIFI_SSID);
    Serial.print(F("[WIFI] IP Address: "));
    Serial.println(ip);

    // 2. Attach WebSocket event handlers
    _ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client->text("{\"event\":\"connected\",\"system\":\"BlindSpottr\",\"version\":\"2.5\"}");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("[WS] Client #%u disconnected\n", client->id());
        }
    });

    _server.addHandler(&_ws);

    // Health-check HTTP route
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"online\",\"system\":\"BlindSpottr\"}");
    });

    _server.begin();
    Serial.println(F("[WS] Asynchronous WebSocket server running on ws://192.168.4.1/ws"));
}

uint32_t TelemetryServer::getClientCount() const {
    return _ws.count();
}

void TelemetryServer::update(const SensorData& data, BsmState state, uint32_t now) {
    _ws.cleanupClients();

    // Stream telemetry to active clients at 20 Hz
    if (now - _lastBroadcastMs < WS_BROADCAST_INTERVAL_MS) {
        return;
    }
    _lastBroadcastMs = now;

    if (_ws.count() > 0) {
        broadcastTelemetry(data, state, now);
    }
}

void TelemetryServer::broadcastTelemetry(const SensorData& data, BsmState state, uint32_t now) {
    _packetCounter++;

    JsonDocument doc;
    doc["id"] = _packetCounter;
    doc["left"] = data.leftObstacle;
    doc["right"] = data.rightObstacle;
    doc["front_valid"] = data.frontValid;
    doc["front_cm"] = data.frontValid ? round(data.frontDistanceCm * 10) / 10.0 : -1.0;
    doc["state"] = stateToString(state);
    doc["uptime_ms"] = now;

    char buffer[256];
    size_t len = serializeJson(doc, buffer);
    _ws.textAll(buffer, len);
}
