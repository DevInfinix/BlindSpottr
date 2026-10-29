#include "telemetry_server.h"
#include "dashboard_html.h"
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_wifi.h>

static Preferences prefs;
static char storedSsid[33] = "";
static char storedPass[65] = "";
static uint32_t lastStaAttemptMs = 0;
static uint32_t lastIpPrintMs = 0;
static bool staConnected = false;

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
    // 1. Load stored Wi-Fi credentials from NVS flash
    // If not set, defaults to "Andronix" / "samsingh"
    prefs.begin("wifi", false);
    String s = prefs.getString("ssid", HOTSPOT_SSID);
    String p = prefs.getString("pass", HOTSPOT_PASSWORD);
    strncpy(storedSsid, s.c_str(), sizeof(storedSsid) - 1);
    strncpy(storedPass, p.c_str(), sizeof(storedPass) - 1);
    prefs.end();

    // 2. Set Wi-Fi Mode to AP + STA
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);

    // Disable Wi-Fi sleep for rock-solid zero-latency responsiveness
    WiFi.setSleep(false);

    // Initial connection attempt to Andronix hotspot
    if (strlen(storedSsid) > 0) {
        WiFi.begin(storedSsid, storedPass);
    }

    IPAddress apIP = WiFi.softAPIP();
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("           BLINDSPOTTR WI-FI & WEB HUD            "));
    Serial.println(F("=================================================="));
    Serial.print(F(" [HOTSPOT TARGET] : "));
    Serial.println(storedSsid);
    Serial.print(F(" [STANDALONE AP]  : "));
    Serial.print(WIFI_AP_SSID);
    Serial.print(F(" (Password: "));
    Serial.print(WIFI_AP_PASSWORD);
    Serial.println(F(")"));
    Serial.print(F(" [AP DIRECT IP]   : http://"));
    Serial.println(apIP);
    Serial.println(F("=================================================="));

    // Register mDNS
    if (MDNS.begin("blindspottr")) {
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 80);
    }

    // 3. Attach WebSocket event handlers
    _ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            client->text("{\"event\":\"connected\",\"system\":\"BlindSpottr\",\"version\":\"2.8\"}");
        }
    });

    _server.addHandler(&_ws);

    // Root route: Serves the 100% self-contained Tesla-style HUD
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", INDEX_HTML);
    });

    // In-browser Wi-Fi configuration endpoint
    _server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
            String newSsid = request->getParam("ssid", true)->value();
            String newPass = request->getParam("pass", true)->value();
            prefs.begin("wifi", false);
            prefs.putString("ssid", newSsid);
            prefs.putString("pass", newPass);
            prefs.end();

            strncpy(storedSsid, newSsid.c_str(), sizeof(storedSsid) - 1);
            strncpy(storedPass, newPass.c_str(), sizeof(storedPass) - 1);

            WiFi.disconnect();
            WiFi.begin(storedSsid, storedPass);

            request->send(200, "application/json", "{\"status\":\"saved\",\"connecting_to\":\"" + newSsid + "\"}");
            Serial.printf("[WIFI] New credentials saved! Connecting to '%s'...\n", storedSsid);
        } else {
            request->send(400, "application/json", "{\"error\":\"missing ssid or pass\"}");
        }
    });

    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"online\",\"system\":\"BlindSpottr\"}");
    });

    _server.begin();
}

uint32_t TelemetryServer::getClientCount() const {
    return _ws.count();
}

void TelemetryServer::update(const SensorData& data, BsmState state, uint32_t now) {
    _ws.cleanupClients();

    // 1. Wi-Fi Station connection status and continuous IP printing
    if (WiFi.status() == WL_CONNECTED) {
        if (!staConnected) {
            staConnected = true;
            Serial.println();
            Serial.println(F("**************************************************"));
            Serial.print(F(" >>> CONNECTED TO HOTSPOT! IP: http://"));
            Serial.println(WiFi.localIP());
            Serial.println(F("**************************************************"));
            lastIpPrintMs = now;
        } else {
            // Periodically print the IP address every 5 seconds so it is never lost!
            if (now - lastIpPrintMs >= 5000) {
                lastIpPrintMs = now;
                Serial.print(F("[WIFI IP] http://"));
                Serial.print(WiFi.localIP());
                Serial.print(F(" (RSSI: "));
                Serial.print(WiFi.RSSI());
                Serial.println(F(" dBm)"));
            }
        }
    } else {
        if (staConnected) {
            staConnected = false;
            Serial.println(F("[WIFI] Disconnected from hotspot. Auto-scanning..."));
        }

        // Active background auto-scan and reconnection every 4 seconds
        if (now - lastStaAttemptMs >= 4000) {
            lastStaAttemptMs = now;
            if (strlen(storedSsid) > 0) {
                Serial.printf("[WIFI] Scanning and connecting to '%s'...\n", storedSsid);
                WiFi.disconnect();
                WiFi.begin(storedSsid, storedPass);
            }
        }
    }

    // 2. Stream telemetry to active clients at 20 Hz
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
