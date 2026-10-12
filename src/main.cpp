#include <Arduino.h>
#include "config.h"
#include "outputs/indicators.h"

// Instantiate LED Indicator Controller with pins from config.h
static IndicatorController indicators(PIN_LED_L, PIN_LED_R, PIN_LED_SAFE, SINGLE_WARN_PULSE_MS);

// Demo / Test State Management
static uint8_t currentState = 0;
static uint32_t lastStateChangeMs = 0;
static const uint32_t STATE_DWELL_MS = 3000; // Hold each mode for 3 seconds
static bool autoCycle = true;

const char* getStateName(uint8_t state) {
    switch (state) {
        case 0: return "STATE_SAFE (Green Solid ON)";
        case 1: return "STATE_LEFT_WARNING (Left Red Blinking)";
        case 2: return "STATE_RIGHT_WARNING (Right Red Blinking)";
        case 3: return "STATE_BOTH_WARNING (Both Red Solid ON)";
        default: return "UNKNOWN";
    }
}

void printStatus(uint8_t state) {
    Serial.printf("[TEST STEP] Active State %d: %s\n", state, getStateName(state));
}

void setup() {
    Serial.begin(115200);
    delay(500); // Allow USB Serial to stabilize on host

    Serial.println();
    Serial.println("==================================================");
    Serial.println("  BLINDSPOTTR — HARDWARE BENCH TEST: COMPONENT 1  ");
    Serial.println("          Cockpit LED Indicator Cluster           ");
    Serial.println("==================================================");
    Serial.printf("  Safe LED (Green)  : GPIO %d\n", PIN_LED_SAFE);
    Serial.printf("  Left LED (Red)    : GPIO %d\n", PIN_LED_L);
    Serial.printf("  Right LED (Red)   : GPIO %d\n", PIN_LED_R);
    Serial.println("--------------------------------------------------");
    Serial.println("  Auto-cycling states every 3 seconds:");
    Serial.println("    0: SAFE        -> Green ON");
    Serial.println("    1: LEFT HAZARD -> Left Red Pulsing (120ms)");
    Serial.println("    2: RIGHT HAZARD-> Right Red Pulsing (120ms)");
    Serial.println("    3: BOTH HAZARD -> Both Red Solid ON");
    Serial.println("--------------------------------------------------");
    Serial.println("  Interactive Serial Commands:");
    Serial.println("    Type '0', '1', '2', or '3' to lock state.");
    Serial.println("    Type 'a' to resume auto-cycle.");
    Serial.println("==================================================");

    indicators.begin();
    printStatus(currentState);
    lastStateChangeMs = millis();
}

void loop() {
    uint32_t now = millis();

    // Check for interactive user input over Serial
    if (Serial.available() > 0) {
        char ch = Serial.read();
        if (ch >= '0' && ch <= '3') {
            currentState = ch - '0';
            autoCycle = false;
            Serial.printf(">> Locked to %s (Manual Mode)\n", getStateName(currentState));
        } else if (ch == 'a' || ch == 'A') {
            autoCycle = true;
            lastStateChangeMs = now;
            Serial.println(">> Resumed Auto-Cycle Mode");
        }
    }

    // Auto-advance through demo states every 3 seconds if enabled
    if (autoCycle && (static_cast<uint32_t>(now - lastStateChangeMs) >= STATE_DWELL_MS)) {
        lastStateChangeMs = now;
        currentState = (currentState + 1) % 4; // Cycle 0 -> 1 -> 2 -> 3 -> 0
        printStatus(currentState);
    }

    // Continuously update indicators without any blocking delays
    indicators.update(currentState, now);
}
