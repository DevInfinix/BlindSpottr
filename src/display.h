#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "sensors.h"
#include "buzzer.h"

class DisplayController {
public:
    DisplayController();
    bool begin();
    void update(const SensorData& data, BsmState state, uint32_t now);
    bool isAvailable() const;

private:
    void renderDashboard(const SensorData& data, BsmState state, uint32_t now);
    void drawTruckChassis(int centerX, int centerY);
    void drawBlindSpotWaves(bool leftHazard, bool rightHazard, uint8_t animTick);
    void drawWarningChevrons(bool leftHazard, bool rightHazard, uint8_t animTick);

    Adafruit_SSD1306 _display;
    bool _isInitialized;
    uint32_t _lastRefreshMs;
    uint8_t _animPhase;
};
