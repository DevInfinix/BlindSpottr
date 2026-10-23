#pragma once
#include <Arduino.h>
#include "config.h"

struct SensorData {
    uint8_t leftRaw;
    bool leftObstacle;

    uint8_t rightRaw;
    bool rightObstacle;

    uint32_t lastSampleMs;
};

class SensorManager {
public:
    SensorManager();
    void begin();
    void update(uint32_t now);
    const SensorData& getData() const;

private:
    SensorData _data;
    uint8_t _leftDebounced;
    uint32_t _leftDebounceStart;

    uint8_t _rightDebounced;
    uint32_t _rightDebounceStart;
};
