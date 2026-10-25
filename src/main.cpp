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

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.7"));
    Serial.println(F("=================================================="));

    // 1. Initialize Buzzer Controller and ensure silence
    buzzer.begin();
    buzzer.silence();

    // 2. Initialize IR sensors and Ultrasonic
    sensors.begin();

    // 3. Initialize OLED Display
    display.begin();

    // 4. Initialize Wi-Fi SoftAP, mDNS & WebSocket Telemetry Server
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

    // Serial telemetry prints completely disabled as requested!
    // Wi-Fi connection info and IP addresses remain permanently clean in terminal.
}
