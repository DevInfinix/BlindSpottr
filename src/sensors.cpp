#include "sensors.h"

SensorManager::SensorManager()
    : _data{},
      _leftDebounced(HIGH),
      _leftDebounceStart(0),
      _rightDebounced(HIGH),
      _rightDebounceStart(0),
      _lastUsTriggerTime(0),
      _usConsecutiveHits(0),
      _recentDistances{-1.0f, -1.0f, -1.0f},
      _recentDistIndex(0) {
    _data.leftRaw = HIGH;
    _data.leftObstacle = false;
    _data.rightRaw = HIGH;
    _data.rightObstacle = false;
    _data.frontValid = false;
    _data.frontDistanceCm = -1.0f;
    _data.frontObstacle = false;
    _data.lastSampleMs = 0;
    _data.lastUsSampleMs = 0;
}

void SensorManager::begin() {
    // 1. Configure IR pins
    pinMode(PIN_IR_LEFT, INPUT);
    pinMode(PIN_IR_RIGHT, INPUT_PULLUP); // GPIO 32 supports internal pullup

    _data.leftRaw = digitalRead(PIN_IR_LEFT);
    _leftDebounced = _data.leftRaw;
    _data.leftObstacle = (_leftDebounced == IR_LEFT_ACTIVE_LEVEL);

    _data.rightRaw = digitalRead(PIN_IR_RIGHT);
    _rightDebounced = _data.rightRaw;
    _data.rightObstacle = (_rightDebounced == IR_RIGHT_ACTIVE_LEVEL);

    // 2. Configure Ultrasonic pins if enabled
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
    if (now - _lastUsTriggerTime < US_SAMPLE_INTERVAL_MS) {
        return;
    }
    _lastUsTriggerTime = now;
    _data.lastUsSampleMs = now;

    // Guard against floating or stuck HIGH echo line
    if (digitalRead(PIN_US_ECHO) == HIGH) {
        _data.frontValid = false;
        _data.frontDistanceCm = -1.0f;
        _data.frontObstacle = false;
        _usConsecutiveHits = 0;
        return;
    }

    // Emit 10µs trigger pulse
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(4);
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    // Read echo duration with bounded timeout (18 ms = ~300 cm)
    const unsigned long durationUs = pulseIn(PIN_US_ECHO, HIGH, US_ECHO_TIMEOUT_US);

    float measuredCm = -1.0f;
    bool validMeasurement = false;

    // Filter out 0 (timeout) and unrealistic ultra-short transients (< 120µs = < 2cm)
    if (durationUs >= 120 && durationUs <= US_ECHO_TIMEOUT_US) {
        float rawCm = durationUs * 0.01715f;
        if (rawCm >= US_MIN_PLAUSIBLE_CM && rawCm <= US_MAX_PLAUSIBLE_CM) {
            measuredCm = rawCm;
            validMeasurement = true;
        }
    }

    if (validMeasurement) {
        _recentDistances[_recentDistIndex] = measuredCm;
        _recentDistIndex = (_recentDistIndex + 1) % 3;

        // Running median filter of last 3 samples to eliminate single-sample spikes
        float sorted[3];
        for (int i = 0; i < 3; i++) sorted[i] = _recentDistances[i];
        if (sorted[0] > sorted[1]) { float t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }
        if (sorted[1] > sorted[2]) { float t = sorted[1]; sorted[1] = sorted[2]; sorted[2] = t; }
        if (sorted[0] > sorted[1]) { float t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }

        const float medianDist = (sorted[1] > 0) ? sorted[1] : measuredCm;

        _data.frontValid = true;
        _data.frontDistanceCm = medianDist;

        // Require 2 consecutive close readings before raising front hazard alarm
        if (medianDist <= FRONT_HAZARD_THRESHOLD_CM) {
            if (_usConsecutiveHits < 2) {
                _usConsecutiveHits++;
            }
        } else {
            _usConsecutiveHits = 0;
        }
        _data.frontObstacle = (_usConsecutiveHits >= 2);
    } else {
        // Measurement timed out or no echo
        _data.frontValid = false;
        _data.frontDistanceCm = -1.0f;
        _data.frontObstacle = false;
        _usConsecutiveHits = 0;
    }
}

const SensorData& SensorManager::getData() const {
    return _data;
}
