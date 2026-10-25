#include "sensors.h"

SensorManager::SensorManager()
    : _data{},
      _leftDebounced(HIGH),
      _leftDebounceStart(0),
      _rightDebounced(HIGH),
      _rightDebounceStart(0),
      _usPhase(UsPhase::IDLE),
      _usPhaseTime(0),
      _lastUsTriggerTime(0),
      _recentDistances{-1.0f, -1.0f, -1.0f},
      _recentDistIndex(0) {
    _data.leftRaw = HIGH;
    _data.leftObstacle = false;
    _data.rightRaw = HIGH;
    _data.rightObstacle = false;
    _data.frontValid = false;
    _data.frontDistanceCm = -1.0f;
    _data.lastSampleMs = 0;
    _data.lastUsSampleMs = 0;
}

void SensorManager::begin() {
    // 1. Configure IR pins
    pinMode(PIN_IR_LEFT, INPUT);
    pinMode(PIN_IR_RIGHT, INPUT_PULLUP);

    _data.leftRaw = digitalRead(PIN_IR_LEFT);
    _leftDebounced = _data.leftRaw;
    _data.leftObstacle = (_leftDebounced == IR_LEFT_ACTIVE_LEVEL);

    _data.rightRaw = digitalRead(PIN_IR_RIGHT);
    _rightDebounced = _data.rightRaw;
    _data.rightObstacle = (_rightDebounced == IR_RIGHT_ACTIVE_LEVEL);

    // 2. Configure Ultrasonic pins
    if (ENABLE_ULTRASONIC) {
        pinMode(PIN_US_TRIG, OUTPUT);
        digitalWrite(PIN_US_TRIG, LOW);
        pinMode(PIN_US_ECHO, INPUT); // GPIO 36 (VP) input-only
    }
}

void SensorManager::update(uint32_t now) {
    sampleIrSensors(now);
    if (ENABLE_ULTRASONIC) {
        sampleUltrasonic(now);
    }
}

void SensorManager::sampleIrSensors(uint32_t now) {
    if (now - _data.lastSampleMs < IR_SAMPLE_INTERVAL_MS) {
        return;
    }
    _data.lastSampleMs = now;

    // 1. Left IR Sensor (GPIO 35)
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

    // 2. Right IR Sensor (GPIO 32)
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

void SensorManager::sampleUltrasonic(uint32_t now) {
    // Schedule acoustic pings every 90 ms (~11 Hz) to prevent acoustic reverberation
    if (now - _lastUsTriggerTime < 90) {
        return;
    }
    _lastUsTriggerTime = now;
    _data.lastUsSampleMs = now;

    // Check if ECHO line is already stuck HIGH before pulse
    if (digitalRead(PIN_US_ECHO) == HIGH) {
        _data.frontValid = false;
        _data.frontDistanceCm = -1.0f;
        return;
    }

    // Send 10µs trigger pulse
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    // Timing with bounded 12ms timeout (~200 cm range)
    const unsigned long durationUs = pulseIn(PIN_US_ECHO, HIGH, 12000);

    // Filter out:
    // 1. durationUs == 0 (no echo received / timeout)
    // 2. durationUs < 400 µs (transducer acoustic ringdown & electrical spike: 400 * 0.01715 = ~6.8 cm)
    // 3. durationUs > 11600 µs (> 200 cm out of range)
    if (durationUs >= 400 && durationUs <= 11600) {
        const float rawCm = durationUs * 0.01715f;
        if (rawCm >= 7.0f && rawCm <= 180.0f) {
            _recentDistances[_recentDistIndex] = rawCm;
            _recentDistIndex = (_recentDistIndex + 1) % 3;

            // 3-sample median filter
            float sorted[3];
            for (int i = 0; i < 3; i++) sorted[i] = _recentDistances[i];
            if (sorted[0] > sorted[1]) { float t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }
            if (sorted[1] > sorted[2]) { float t = sorted[1]; sorted[1] = sorted[2]; sorted[2] = t; }
            if (sorted[0] > sorted[1]) { float t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }

            if (sorted[1] > 0) {
                _data.frontValid = true;
                _data.frontDistanceCm = sorted[1];
                return;
            }
        }
    }

    // No valid echo received: smoothly clear without 0-glitching
    _data.frontValid = false;
    _data.frontDistanceCm = -1.0f;
}

const SensorData& SensorManager::getData() const {
    return _data;
}
