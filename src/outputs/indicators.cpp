#include "indicators.h"

IndicatorController::IndicatorController(uint8_t pinLeft,
                                         uint8_t pinRight,
                                         uint8_t pinSafe,
                                         uint32_t pulseMs)
    : _pinLeft(pinLeft),
      _pinRight(pinRight),
      _pinSafe(pinSafe),
      _pulseMs(pulseMs),
      _lastBlinkToggleMs(0),
      _blinkState(false) {}

void IndicatorController::begin() {
    pinMode(_pinLeft, OUTPUT);
    pinMode(_pinRight, OUTPUT);
    pinMode(_pinSafe, OUTPUT);

    allOff();
    setSafe();
}

void IndicatorController::allOff() {
    digitalWrite(_pinLeft, LOW);
    digitalWrite(_pinRight, LOW);
    digitalWrite(_pinSafe, LOW);
}

void IndicatorController::setSafe() {
    digitalWrite(_pinLeft, LOW);
    digitalWrite(_pinRight, LOW);
    digitalWrite(_pinSafe, HIGH);
}

void IndicatorController::update(uint8_t state, uint32_t now) {
    // Wrap-safe cadence timer for blinking warning indicators
    if (static_cast<uint32_t>(now - _lastBlinkToggleMs) >= _pulseMs) {
        _lastBlinkToggleMs = now;
        _blinkState = !_blinkState;
    }

    switch (state) {
        case 0: // STATE_SAFE: Clear road, green light active
            digitalWrite(_pinSafe, HIGH);
            digitalWrite(_pinLeft, LOW);
            digitalWrite(_pinRight, LOW);
            break;

        case 1: // STATE_LEFT_WARNING: Left hazard, pulsate Left Red LED
            digitalWrite(_pinSafe, LOW);
            digitalWrite(_pinLeft, _blinkState ? HIGH : LOW);
            digitalWrite(_pinRight, LOW);
            break;

        case 2: // STATE_RIGHT_WARNING: Right hazard, pulsate Right Red LED
            digitalWrite(_pinSafe, LOW);
            digitalWrite(_pinLeft, LOW);
            digitalWrite(_pinRight, _blinkState ? HIGH : LOW);
            break;

        case 3: // STATE_BOTH_WARNING: Dual hazard, both Red LEDs solid ON
            digitalWrite(_pinSafe, LOW);
            digitalWrite(_pinLeft, HIGH);
            digitalWrite(_pinRight, HIGH);
            break;

        default: // Fallback safety: default to SAFE
            setSafe();
            break;
    }
}
