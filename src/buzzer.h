#pragma once
#include <Arduino.h>
#include "config.h"

enum class BsmState : uint8_t {
    SAFE_CLEAR,
    WARN_LEFT,
    WARN_RIGHT,
    WARN_FRONT,
    WARN_BOTH,
    WARN_MULTI
};

class BuzzerController {
public:
    BuzzerController();
    void begin();
    void update(BsmState state, uint32_t now);
    void silence();

private:
    void toneOn(uint32_t freqHz);
    void toneOff();

    BsmState _currentState;
    bool _isToneActive;
    bool _dualToggle;
    uint32_t _lastToggleMs;
};
