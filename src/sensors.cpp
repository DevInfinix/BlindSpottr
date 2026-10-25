#include "sensors.h"

SensorManager::SensorManager()
    : _data{},
      _leftDebounced(HIGH),
      _leftDebounceStart(0),
      _rightDebounced(HIGH),
      _rightDebounceStart(0),
      _lastUsTriggerTime(0) {
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
    // 1. Configure IR pins (GPIO 34 and 35 are input-only GPI)
    pinMode(PIN_IR_LEFT, INPUT);
    pinMode(PIN_IR_RIGHT, INPUT);

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

void SensorManager::sampleUltrasonic(uint32_t now) {
    if (now - _lastUsTriggerTime < US_SAMPLE_INTERVAL_MS) {
        return;
    }
    _lastUsTriggerTime = now;
    _data.lastUsSampleMs = now;

    // Guard against floating/stuck HIGH line
    if (digitalRead(PIN_US_ECHO) == HIGH) {
        _data.frontValid = false;
        _data.frontDistanceCm = -1.0f;
        _data.frontObstacle = false;
        return;
    }

    // Send 10µs trigger pulse
    digitalWrite(PIN_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    // Measure echo pulse with bounded timeout
    const unsigned long durationUs = pulseIn(PIN_US_ECHO, HIGH, US_ECHO_TIMEOUT_US);

    if (durationUs == 0) {
        _data.frontValid = false;
        _data.frontDistanceCm = -1.0f;
        _data.frontObstacle = false;
    } else {
        const float distanceCm = static_cast<float>(durationUs) * 0.01715f;
        if (distanceCm >= US_MIN_PLAUSIBLE_CM && distanceCm <= US_MAX_PLAUSIBLE_CM) {
            _data.frontValid = true;
            _data.frontDistanceCm = distanceCm;
            _data.frontObstacle = (distanceCm <= FRONT_HAZARD_THRESHOLD_CM);
        } else {
            _data.frontValid = false;
            _data.frontDistanceCm = -1.0f;
            _data.frontObstacle = false;
        }
    }
}

const SensorData& SensorManager::getData() const {
    return _data;
}
