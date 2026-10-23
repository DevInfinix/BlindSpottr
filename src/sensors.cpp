#include "sensors.h"

SensorManager::SensorManager()
    : _data{},
      _leftDebounced(HIGH),
      _leftDebounceStart(0),
      _rightDebounced(HIGH),
      _rightDebounceStart(0) {
    _data.leftRaw = HIGH;
    _data.leftObstacle = false;
    _data.rightRaw = HIGH;
    _data.rightObstacle = false;
    _data.lastSampleMs = 0;
}

void SensorManager::begin() {
    // GPIO 34 and 35 are input-only pins (GPI)
    pinMode(PIN_IR_LEFT, INPUT);
    pinMode(PIN_IR_RIGHT, INPUT);

    _data.leftRaw = digitalRead(PIN_IR_LEFT);
    _leftDebounced = _data.leftRaw;
    _data.leftObstacle = (_leftDebounced == IR_LEFT_ACTIVE_LEVEL);

    _data.rightRaw = digitalRead(PIN_IR_RIGHT);
    _rightDebounced = _data.rightRaw;
    _data.rightObstacle = (_rightDebounced == IR_RIGHT_ACTIVE_LEVEL);
}

void SensorManager::update(uint32_t now) {
    if (now - _data.lastSampleMs < IR_SAMPLE_INTERVAL_MS) {
        return;
    }
    _data.lastSampleMs = now;

    // 1. Left IR Sensor
    const uint8_t rawL = digitalRead(PIN_IR_LEFT);
    _data.leftRaw = rawL;
    if (rawL != _leftDebounced) {
        if (_leftDebounceStart == 0) {
            _leftDebounceStart = now;
        } else if (now - _leftDebounceStart >= IR_DEBOUNCE_MS) {
            _leftDebounced = rawL;
            _leftDebounceStart = 0;
        }
    } else {
        _leftDebounceStart = 0;
    }
    _data.leftObstacle = (_leftDebounced == IR_LEFT_ACTIVE_LEVEL);

    // 2. Right IR Sensor
    const uint8_t rawR = digitalRead(PIN_IR_RIGHT);
    _data.rightRaw = rawR;
    if (rawR != _rightDebounced) {
        if (_rightDebounceStart == 0) {
            _rightDebounceStart = now;
        } else if (now - _rightDebounceStart >= IR_DEBOUNCE_MS) {
            _rightDebounced = rawR;
            _rightDebounceStart = 0;
        }
    } else {
        _rightDebounceStart = 0;
    }
    _data.rightObstacle = (_rightDebounced == IR_RIGHT_ACTIVE_LEVEL);
}

const SensorData& SensorManager::getData() const {
    return _data;
}
