#include "buzzer.h"

BuzzerController::BuzzerController()
    : _currentState(BsmState::SAFE_CLEAR),
      _isToneActive(false),
      _dualToggle(false),
      _lastToggleMs(0) {}

void BuzzerController::begin() {
    ledcSetup(BUZZER_LEDC_CHANNEL, 2700, BUZZER_LEDC_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
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
    if (state != _currentState) {
        _currentState = state;
        _lastToggleMs = now;
        _dualToggle = false;

        if (state == BsmState::SAFE_CLEAR) {
            silence();
            return;
        }

        if (state == BsmState::WARN_LEFT) {
            toneOn(BUZZER_FREQ_LEFT_HZ);
        } else if (state == BsmState::WARN_RIGHT) {
            toneOn(BUZZER_FREQ_RIGHT_HZ);
        } else if (state == BsmState::WARN_FRONT) {
            toneOn(BUZZER_FREQ_FRONT_HZ);
        } else if (state == BsmState::WARN_BOTH || state == BsmState::WARN_MULTI) {
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

    // Pattern 1: DUAL OR MULTI HAZARD (Rapid alternating chirp)
    if (state == BsmState::WARN_BOTH || state == BsmState::WARN_MULTI) {
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

    // Pattern 2: FRONT HAZARD (Urgent pulsing 1800 Hz)
    if (state == BsmState::WARN_FRONT) {
        const uint32_t interval = _isToneActive ? BUZZER_FRONT_ON_MS : BUZZER_FRONT_OFF_MS;
        if (now - _lastToggleMs >= interval) {
            _lastToggleMs = now;
            if (_isToneActive) {
                toneOff();
            } else {
                toneOn(BUZZER_FREQ_FRONT_HZ);
            }
        }
        return;
    }

    // Pattern 3: SINGLE SIDE HAZARD (Left or Right)
    const uint32_t currentInterval = _isToneActive ? BUZZER_SINGLE_ON_MS : BUZZER_SINGLE_OFF_MS;
    if (now - _lastToggleMs >= currentInterval) {
        _lastToggleMs = now;
        if (_isToneActive) {
            toneOff();
        } else {
            if (state == BsmState::WARN_LEFT) {
                toneOn(BUZZER_FREQ_LEFT_HZ);
            } else {
                toneOn(BUZZER_FREQ_RIGHT_HZ);
            }
        }
    }
}
