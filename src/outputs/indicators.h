#pragma once
#include <Arduino.h>
#include "config.h"

/**
 * @brief Non-blocking controller for Cockpit Warning & Safe LEDs.
 * 
 * Manages Left Red LED (GPIO 25), Right Red LED (GPIO 26), and Safe Green LED (GPIO 27).
 * All timing is 100% non-blocking using wrap-safe millis() delta arithmetic.
 */
class IndicatorController {
public:
    /**
     * @param pinLeft GPIO for Left Red warning LED (default PIN_LED_L)
     * @param pinRight GPIO for Right Red warning LED (default PIN_LED_R)
     * @param pinSafe GPIO for Safe Green status LED (default PIN_LED_SAFE)
     * @param pulseMs Blink half-period in milliseconds (default SINGLE_WARN_PULSE_MS)
     */
    IndicatorController(uint8_t pinLeft = PIN_LED_L,
                        uint8_t pinRight = PIN_LED_R,
                        uint8_t pinSafe = PIN_LED_SAFE,
                        uint32_t pulseMs = SINGLE_WARN_PULSE_MS);

    /**
     * @brief Configures GPIO pins as outputs and ensures safe initial state.
     */
    void begin();

    /**
     * @brief Evaluates current system state and drives LEDs non-blockingly.
     * @param state System state code (0: SAFE, 1: LEFT, 2: RIGHT, 3: BOTH)
     * @param now Current timestamp from millis()
     */
    void update(uint8_t state, uint32_t now);

    /**
     * @brief Immediately forces all LEDs off.
     */
    void allOff();

    /**
     * @brief Turns on Safe LED and turns off all warning LEDs.
     */
    void setSafe();

private:
    uint8_t _pinLeft;
    uint8_t _pinRight;
    uint8_t _pinSafe;
    uint32_t _pulseMs;

    uint32_t _lastBlinkToggleMs;
    bool _blinkState;
};
