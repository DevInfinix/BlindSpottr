#pragma once
#include <Arduino.h>
#include "config.h"

struct SensorData {
    // Left IR Sensor
    uint8_t leftRaw;
    bool leftObstacle;

    // Right IR Sensor
    uint8_t rightRaw;
    bool rightObstacle;

    // Front Ultrasonic Sensor
    bool frontValid;
    float frontDistanceCm;
    bool frontObstacle;

    uint32_t lastSampleMs;
    uint32_t lastUsSampleMs;
};

class SensorManager {
public:
    SensorManager();
    void begin();
    void update(uint32_t now);
    const SensorData& getData() const;

private:
    void sampleIrSensors(uint32_t now);
    void sampleUltrasonic(uint32_t now);

    SensorData _data;
    uint8_t _leftDebounced;
    uint32_t _leftDebounceStart;

    uint8_t _rightDebounced;
    uint32_t _rightDebounceStart;

    uint32_t _lastUsTriggerTime;
};
