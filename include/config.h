#pragma once
#include <Arduino.h>

/**
 * @file config.h
 * @brief Centralized Hardware Configuration for BlindSpottr Toy-Truck BSM.
 */

// =============================================================================
// 1. HARDWARE GPIO PIN ALLOCATION (ESP32 DevKit V1 38-Pin)
// =============================================================================

// Left & Right Digital IR Obstacle Sensors
// Verified physical pin assignments on chassis:
// (GPIO 35 corresponds to physical Left side, GPIO 34 corresponds to physical Right side)
constexpr uint8_t PIN_IR_LEFT             = 35; // Physical Left IR input
constexpr uint8_t PIN_IR_RIGHT            = 34; // Physical Right IR input

// HC-SR04 Front Ultrasonic Sensor (Optional / physically disabled when not plugged)
constexpr uint8_t PIN_US_TRIG             = 26;
constexpr uint8_t PIN_US_ECHO             = 36; // Reassigned away from 34 to avoid conflict
constexpr bool ENABLE_ULTRASONIC          = false; // Set to true when HC-SR04 is plugged in

// I2C OLED Display (0.96" SSD1306, 128x64 Monochrome)
constexpr uint8_t PIN_OLED_SDA            = 21;
constexpr uint8_t PIN_OLED_SCL            = 22;
constexpr uint8_t OLED_I2C_ADDR           = 0x3C;
constexpr uint8_t SCREEN_WIDTH            = 128;
constexpr uint8_t SCREEN_HEIGHT           = 64;

// Cockpit Audible Alert (Buzzer output on GPIO 25 via ESP32 LEDC PWM)
constexpr uint8_t PIN_BUZZER              = 25;
constexpr uint8_t BUZZER_LEDC_CHANNEL     = 0;
constexpr uint8_t BUZZER_LEDC_RESOLUTION  = 8;     // 8-bit resolution (0..255)

// Cockpit Optional Indicator LEDs (legacy/header references)
constexpr uint8_t PIN_LED_L               = 18;
constexpr uint8_t PIN_LED_R               = 19;
constexpr uint8_t PIN_LED_SAFE            = 23;
constexpr uint32_t BUZZER_SIDE_ON_MS      = 100;

// =============================================================================
// 2. SENSOR TIMING & POLARITY
// =============================================================================

// Standard LM393 IR modules output LOW on object detection
constexpr uint8_t IR_LEFT_ACTIVE_LEVEL    = LOW;
constexpr uint8_t IR_RIGHT_ACTIVE_LEVEL   = LOW;

constexpr uint32_t IR_SAMPLE_INTERVAL_MS  = 10;   // 100 Hz sampling rate
constexpr uint32_t IR_DEBOUNCE_MS         = 15;   // Glitch filter window

// =============================================================================
// 3. BUZZER FREQUENCY VARIATIONS & PATTERNS
// =============================================================================

// Distinct acoustic pitch for each warning state:
// - Left Hazard: 2200 Hz tone (Mid-high tone, rhythmic pulsed cadence)
// - Right Hazard: 2900 Hz tone (Sharper high tone, alternating cadence)
// - Dual Hazard (Both Sides): Rapid dual-chirp switching between 2400 Hz and 3400 Hz!
constexpr uint32_t BUZZER_FREQ_LEFT_HZ    = 2200;
constexpr uint32_t BUZZER_FREQ_RIGHT_HZ   = 2900;
constexpr uint32_t BUZZER_FREQ_DUAL_LO_HZ = 2400;
constexpr uint32_t BUZZER_FREQ_DUAL_HI_HZ = 3400;

// Cadence timings in milliseconds:
constexpr uint32_t BUZZER_SINGLE_ON_MS    = 100;
constexpr uint32_t BUZZER_SINGLE_OFF_MS   = 120;

constexpr uint32_t BUZZER_DUAL_CHIRP_MS   = 50;  // Ultra-rapid alert burst

// =============================================================================
// 4. DISPLAY REFRESH & SERIAL DIAGNOSTICS
// =============================================================================

constexpr uint32_t OLED_REFRESH_INTERVAL_MS = 60;    // ~16 FPS smooth animation
constexpr uint32_t SERIAL_BAUD              = 115200;
constexpr uint32_t DIAGNOSTICS_INTERVAL_MS  = 250;
