#include "telemetry_server.h"
#include "dashboard_html.h"
#include <ESPmDNS.h>

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
    // 1. Dual Wi-Fi Mode: AP + Station
    // This allows the ESP32 to connect to your phone's hotspot while simultaneously broadcasting its own AP
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    // Try connecting to phone hotspot if configured
    if (strlen(HOTSPOT_SSID) > 0 && strcmp(HOTSPOT_SSID, "iPhone") != 0) {
        WiFi.begin(HOTSPOT_SSID, HOTSPOT_PASSWORD);
        Serial.printf("[WIFI] Connecting to Phone Hotspot '%s'...\n", HOTSPOT_SSID);
    }

    IPAddress apIP = WiFi.softAPIP();
    Serial.print(F("[WIFI] Standalone AP active: "));
    Serial.println(WIFI_AP_SSID);
    Serial.print(F("[WIFI] AP Web URL: http://"));
    Serial.println(apIP);

    // Start mDNS responder so you can just type 'http://blindspottr.local'
    if (MDNS.begin("blindspottr")) {
        Serial.println(F("[mDNS] Responder started: http://blindspottr.local"));
    }

    // 2. Attach WebSocket event handlers
    _ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[WS] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client->text("{\"event\":\"connected\",\"system\":\"BlindSpottr\",\"version\":\"2.6\"}");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("[WS] Client #%u disconnected\n", client->id());
        }
    });

    _server.addHandler(&_ws);

    // Root route: Serves the 100% self-contained Tesla-style HUD (zero external CDN dependencies)
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", INDEX_HTML);
    });

    // Health-check HTTP route
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"online\",\"system\":\"BlindSpottr\"}");
    });

    _server.begin();
    Serial.println(F("[WS] Asynchronous WebSocket server active on /ws"));
}

uint32_t TelemetryServer::getClientCount() const {
    return _ws.count();
}

void TelemetryServer::update(const SensorData& data, BsmState state, uint32_t now) {
    _ws.cleanupClients();

    // Log Wi-Fi Station IP once connected to hotspot
    static bool staReported = false;
    if (WiFi.status() == WL_CONNECTED && !staReported) {
        staReported = true;
        Serial.print(F("[WIFI] CONNECTED TO PHONE HOTSPOT! IP Address: http://"));
        Serial.println(WiFi.localIP());
    }

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
