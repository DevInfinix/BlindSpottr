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
static BsmState lastLoggedState = BsmState::SAFE_CLEAR;
static int lastLoggedDistance = -999;

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

/**
 * @brief Clean, non-spamming serial output:
 *        Prints ONLY when state changes OR at a slow 2-second heartbeat
 */
static void printDiagnostics(const SensorData& data, BsmState state, uint32_t now) {
    const int currentDist = data.frontValid ? (int)data.frontDistanceCm : -1;
    const bool stateChanged = (state != lastLoggedState);
    const bool distSignificantlyChanged = (abs(currentDist - lastLoggedDistance) >= 5);
    const bool timeElapsed = (now - lastDiagnosticsMs >= 2000); // 2-second slow heartbeat

    if (!stateChanged && !distSignificantlyChanged && !timeElapsed) {
        return;
    }

    lastDiagnosticsMs = now;
    lastLoggedState = state;
    lastLoggedDistance = currentDist;

    Serial.print(F("[TELEM] L="));
    Serial.print(data.leftObstacle ? F("OBJ") : F("CLR"));
    Serial.print(F(" | R="));
    Serial.print(data.rightObstacle ? F("OBJ") : F("CLR"));
    Serial.print(F(" | US="));
    if (data.frontValid) {
        Serial.print(currentDist);
        Serial.print(F("cm"));
    } else {
        Serial.print(F("---"));
    }
    Serial.print(F(" | STATE="));

    switch (state) {
        case BsmState::SAFE_CLEAR: Serial.println(F("CLEAR")); break;
        case BsmState::WARN_LEFT:  Serial.println(F("LEFT HAZARD (2200Hz)")); break;
        case BsmState::WARN_RIGHT: Serial.println(F("RIGHT HAZARD (2900Hz)")); break;
        case BsmState::WARN_BOTH:  Serial.println(F("DUAL HAZARD (Warble)")); break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.6"));
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

    // 6. Clean, low-frequency UART Telemetry
    printDiagnostics(data, currentState, now);
}
