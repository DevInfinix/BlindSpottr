#pragma once
#include <Arduino.h>

/**
 * @file config.h
 * @brief Authoritative Hardware Pinouts, Timing Constants, and Network Configuration.
 * 
 * Sourced from docs/flow.md. All GPIO pinouts are fixed to the physical wiring harness.
 * Strapping pins (GPIO 0, 2, 12, 15) must NEVER be used.
 * Pins 34 and 35 are input-only (GPI) and do NOT have internal pull-up/pull-down resistors.
 */

// =============================================================================
// 1. HARDWARE GPIO PIN ALLOCATION (ESP32 DevKit V1 38-Pin)
// =============================================================================

// Radar Sensors (RCWL-0516 Microwave Doppler)
// Active-HIGH digital pulses on motion detection
#define PIN_RADAR_L             32  // Left blind-spot radar input
#define PIN_RADAR_R             33  // Right blind-spot radar input

// Infrared Sensors (TCRT5000 / LM393 Optical Presence)
// Input-only pins on ESP32 silicon (GPI)
#define PIN_IR_L                34  // Left blind-spot IR input
#define PIN_IR_R                35  // Right blind-spot IR input

// Cockpit Visual Indicators (Push-pull active-HIGH, 330Ω current-limiting resistors)
#define PIN_LED_L               25  // Left warning LED (Red)
#define PIN_LED_R               26  // Right warning LED (Red)
#define PIN_LED_SAFE            27  // Safe status LED (Green)

// Cockpit Audible Alert (Active buzzer, active-HIGH)
#define PIN_BUZZER              14  // Buzzer alert output

// OLED Display (0.96" SSD1306, 128x64 Monochrome over I2C)
#define PIN_I2C_SDA             21  // ESP32 default hardware I2C Data
#define PIN_I2C_SCL             22  // ESP32 default hardware I2C Clock
#define OLED_I2C_ADDR           0x3C
#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           64

// =============================================================================
// 2. SENSING, TIMING & FUSION CONSTANTS
// =============================================================================

// Real-Time Sensing Task Cadence (FreeRTOS Core 0)
#define SENSING_CADENCE_MS      20    // 50 Hz loop frequency

// Radar Hold Duration
// Latch active status for 3000 ms to bridge moving-to-stopped vehicle transition
#define RADAR_HOLD_MS           3000

// IR Sensor Polarity & Filtering
// Most LM393 comparator modules pull LOW when an obstacle reflects IR light
#define IR_DETECT_LEVEL         LOW   // Change to HIGH if hardware batch is active-high
#define IR_DEBOUNCE_MS          30    // Glitch suppression window

// Alert Cadence Durations (Non-Blocking Timing)
#define SINGLE_WARN_PULSE_MS    120   // Blink/beep period for Left or Right hazard
#define BOTH_WARN_PULSE_MS      60    // Rapid pulse period for Dual Hazard

// =============================================================================
// 3. FREERTOS TASK CONFIGURATION
// =============================================================================

#define SENSOR_TASK_STACK_SIZE  4096
#define SENSOR_TASK_PRIORITY    3     // Higher than idle loop
#define SENSOR_TASK_CORE        0     // Dedicated Core 0 for zero-jitter sensing

// =============================================================================
// 4. WI-FI SOFTAP & TELEMETRY CONFIGURATION
// =============================================================================

#define WIFI_AP_SSID            "BlindSpottr-AP"
#define WIFI_AP_PASS            "blindspot123"
#define WIFI_AP_CHANNEL         6
#define WIFI_MAX_CLIENTS        4

// Static Network Gateway
#define WIFI_AP_IP              192, 168, 4, 1
#define WIFI_AP_GW              192, 168, 4, 1
#define WIFI_AP_MASK            255, 255, 255, 0

// WebSocket Telemetry Heartbeat (Core 1)
#define WS_HEARTBEAT_MS         200   // Periodic telemetry broadcast interval
