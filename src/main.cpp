#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "display.h"
#include "buzzer.h"

// Hardware Singletons
static SensorManager sensors;
static DisplayController display;
static BuzzerController buzzer;

static BsmState currentState = BsmState::SAFE_CLEAR;
static uint32_t lastDiagnosticsMs = 0;

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

    Serial.print(F("LEFT_RAW="));
    Serial.print(data.leftRaw);
    Serial.print(F(" ["));
    Serial.print(data.leftObstacle ? F("OBJECT") : F("CLEAR"));
    Serial.print(F("] | RIGHT_RAW="));
    Serial.print(data.rightRaw);
    Serial.print(F(" ["));
    Serial.print(data.rightObstacle ? F("OBJECT") : F("CLEAR"));
    Serial.print(F("] | STATE="));

    switch (state) {
        case BsmState::SAFE_CLEAR: Serial.println(F("ROAD_CLEAR (Buzzer Silent)")); break;
        case BsmState::WARN_LEFT:  Serial.println(F("LEFT_HAZARD (Buzzer 2200Hz Beep)")); break;
        case BsmState::WARN_RIGHT: Serial.println(F("RIGHT_HAZARD (Buzzer 2900Hz Beep)")); break;
        case BsmState::WARN_BOTH:  Serial.println(F("DUAL_HAZARD (Buzzer 3400/2400Hz Rapid Chirp)")); break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.0"));
    Serial.println(F("=================================================="));

    // 1. Initialize Buzzer Controller on GPIO 25 with tone generation
    buzzer.begin();
    buzzer.silence();

    // 2. Initialize IR sensors on verified GPIO 34 & 35
    sensors.begin();

    // 3. Initialize OLED Display on GPIO 21 & 22 with Graphical Dashboard
    display.begin();

    Serial.println(F("[INFO] System online. Dashboard and audio alerts active."));
}

void loop() {
    const uint32_t now = millis();

    // 1. Non-blocking sensor update
    sensors.update(now);
    const SensorData& data = sensors.getData();

    // 2. Deterministic FSM State Resolution
    currentState = evaluateState(data);

    // 3. Audio Alert Engine with Tone Variations
    buzzer.update(currentState, now);

    // 4. Graphical OLED Dashboard Rendering
    display.update(data, currentState, now);

    // 5. Periodic UART Telemetry
    printDiagnostics(data, currentState, now);
}
