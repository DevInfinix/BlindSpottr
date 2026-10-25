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
    const bool sideBoth = (data.leftObstacle && data.rightObstacle);
    const bool front = (data.frontValid && data.frontObstacle);

    // Front hazard combined with side obstacles creates multi-hazard
    if (front && (data.leftObstacle || data.rightObstacle)) {
        return BsmState::WARN_MULTI;
    } else if (sideBoth) {
        return BsmState::WARN_BOTH;
    } else if (data.leftObstacle) {
        return BsmState::WARN_LEFT;
    } else if (data.rightObstacle) {
        return BsmState::WARN_RIGHT;
    } else if (front) {
        return BsmState::WARN_FRONT;
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
    Serial.print(F(" | FRONT="));
    if (data.frontValid) {
        Serial.print((int)data.frontDistanceCm);
        Serial.print(F("cm"));
        if (data.frontObstacle) Serial.print(F(" [WARN]"));
    } else {
        Serial.print(F("---"));
    }
    Serial.print(F(" | STATE="));

    switch (state) {
        case BsmState::SAFE_CLEAR: Serial.println(F("ROAD_CLEAR")); break;
        case BsmState::WARN_LEFT:  Serial.println(F("LEFT_HAZARD (2200Hz)")); break;
        case BsmState::WARN_RIGHT: Serial.println(F("RIGHT_HAZARD (2900Hz)")); break;
        case BsmState::WARN_FRONT: Serial.println(F("FRONT_HAZARD (1800Hz)")); break;
        case BsmState::WARN_BOTH:  Serial.println(F("DUAL_HAZARD (Warble)")); break;
        case BsmState::WARN_MULTI: Serial.println(F("MULTI_HAZARD (Warble)")); break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.3"));
    Serial.println(F("=================================================="));
    Serial.println(F("[CONFIG] Left IR = GPIO 35, Right IR = GPIO 32"));
    Serial.println(F("[CONFIG] Ultrasonic TRIG = GPIO 26, ECHO = GPIO 36"));

    // 1. Initialize Buzzer Controller on GPIO 25 with tone generation
    buzzer.begin();
    buzzer.silence();

    // 2. Initialize IR sensors and Ultrasonic
    sensors.begin();

    // 3. Initialize OLED Display on GPIO 21 & 22 with Graphical Dashboard
    display.begin();

    Serial.println(F("[INFO] System online. Dashboard, ultrasonic & audio alerts active."));
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
