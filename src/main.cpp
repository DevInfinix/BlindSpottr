#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "display.h"
#include "buzzer.h"
#include "telemetry_server.h"

// Hardware & Server Singletons
static SensorManager sensors;
static DisplayController display;
static BuzzerController buzzer;
static TelemetryServer telemetry;

static BsmState currentState = BsmState::SAFE_CLEAR;
static uint32_t lastDiagnosticsMs = 0;

/**
 * @brief Evaluates state strictly based on Blind-Spot IR inputs.
 */
static BsmState evaluateState(const SensorData& data) {
    if (data.leftObstacle && data.rightObstacle) {
        return BsmState::WARN_BOTH;
    } else if (data.leftObstacle) {
        return BsmState::WARN_LEFT;
    } else if (data.rightObstacle) {
        return BsmState::WARN_RIGHT;
    }
    return BsmState::SAFE_CLEAR;
}

static void printDiagnostics(const SensorData& data, BsmState state, uint32_t now) {
    if (now - lastDiagnosticsMs < DIAGNOSTICS_INTERVAL_MS) {
        return;
    }
    lastDiagnosticsMs = now;

    Serial.print(F("LEFT (P35)="));
    Serial.print(data.leftObstacle ? F("OBJ") : F("CLR"));
    Serial.print(F(" | RIGHT (P32)="));
    Serial.print(data.rightObstacle ? F("OBJ") : F("CLR"));
    Serial.print(F(" | US_DIST="));
    if (data.frontValid) {
        Serial.print((int)data.frontDistanceCm);
        Serial.print(F("cm"));
    } else {
        Serial.print(F("---"));
    }
    Serial.print(F(" | WS_CLIENTS="));
    Serial.print(telemetry.getClientCount());
    Serial.print(F(" | BUZZER="));

    switch (state) {
        case BsmState::SAFE_CLEAR: Serial.println(F("SILENT (Clear)")); break;
        case BsmState::WARN_LEFT:  Serial.println(F("2200Hz Beep (Left Hazard)")); break;
        case BsmState::WARN_RIGHT: Serial.println(F("2900Hz Beep (Right Hazard)")); break;
        case BsmState::WARN_BOTH:  Serial.println(F("3400/2400Hz Chirp (Dual Hazard)")); break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.5"));
    Serial.println(F("=================================================="));

    // 1. Initialize Buzzer Controller and ensure silence
    buzzer.begin();
    buzzer.silence();

    // 2. Initialize IR sensors and Ultrasonic
    sensors.begin();

    // 3. Initialize OLED Display
    display.begin();

    // 4. Initialize Wi-Fi SoftAP & WebSocket Telemetry Server
    telemetry.begin();

    Serial.println(F("[INFO] System online. Telemetry streaming on ws://192.168.4.1/ws"));
}

void loop() {
    const uint32_t now = millis();

    // 1. Non-blocking sensor update
    sensors.update(now);
    const SensorData& data = sensors.getData();

    // 2. Deterministic FSM State Resolution (Strictly IR blind spot warnings)
    currentState = evaluateState(data);

    // 3. Audio Alert Engine (Beeps ONLY for left/right/both IR hazards)
    buzzer.update(currentState, now);

    // 4. Graphical OLED Dashboard Rendering
    display.update(data, currentState, now);

    // 5. High-speed WebSocket Telemetry Stream (20 Hz)
    telemetry.update(data, currentState, now);

    // 6. Periodic UART Telemetry
    printDiagnostics(data, currentState, now);
}
