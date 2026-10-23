#include "buzzer.h"

BuzzerController::BuzzerController()
    : _currentState(BsmState::SAFE_CLEAR),
      _isToneActive(false),
      _dualToggle(false),
      _lastToggleMs(0) {}

void BuzzerController::begin() {
    ledcSetup(BUZZER_LEDC_CHANNEL, 2700, BUZZER_LEDC_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0); // Silence initially
    _isToneActive = false;
    _lastToggleMs = millis();
}

void BuzzerController::silence() {
    toneOff();
    _isToneActive = false;
}

void BuzzerController::toneOn(uint32_t freqHz) {
    ledcWriteTone(BUZZER_LEDC_CHANNEL, freqHz);
    _isToneActive = true;
}

void BuzzerController::toneOff() {
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
    _isToneActive = false;
}

void BuzzerController::update(BsmState state, uint32_t now) {
    // Immediate state transition handling
    if (state != _currentState) {
        _currentState = state;
        _lastToggleMs = now;
        _dualToggle = false;

        if (state == BsmState::SAFE_CLEAR) {
            silence();
            return;
        }

        // Start burst immediately upon hazard detection
        if (state == BsmState::WARN_LEFT) {
            toneOn(BUZZER_FREQ_LEFT_HZ);
        } else if (state == BsmState::WARN_RIGHT) {
            toneOn(BUZZER_FREQ_RIGHT_HZ);
        } else if (state == BsmState::WARN_BOTH) {
            toneOn(BUZZER_FREQ_DUAL_HI_HZ);
        }
        return;
    }

    if (state == BsmState::SAFE_CLEAR) {
        if (_isToneActive) {
            silence();
        }
        return;
    }

    // Pattern 1: DUAL HAZARD (Both sides blocked)
    // Rapid warble alarm switching between high pitch (3400 Hz) and low pitch (2400 Hz)
    if (state == BsmState::WARN_BOTH) {
        if (now - _lastToggleMs >= BUZZER_DUAL_CHIRP_MS) {
            _lastToggleMs = now;
            _dualToggle = !_dualToggle;
            if (_dualToggle) {
                toneOn(BUZZER_FREQ_DUAL_HI_HZ); // 3400 Hz
            } else {
                toneOn(BUZZER_FREQ_DUAL_LO_HZ); // 2400 Hz
            }
        }
        return;
    }

    // Pattern 2: SINGLE HAZARD (Left or Right)
    // Rhythmic warning beep: 100ms ON / 120ms OFF
    const uint32_t currentInterval = _isToneActive ? BUZZER_SINGLE_ON_MS : BUZZER_SINGLE_OFF_MS;
    if (now - _lastToggleMs >= currentInterval) {
        _lastToggleMs = now;
        if (_isToneActive) {
            toneOff();
        } else {
            if (state == BsmState::WARN_LEFT) {
                toneOn(BUZZER_FREQ_LEFT_HZ);  // 2200 Hz
            } else {
                toneOn(BUZZER_FREQ_RIGHT_HZ); // 2900 Hz
            }
        }
    }
}
