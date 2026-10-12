# BlindSpottr — Firmware Implementation Reference

> **AUTHORITY:** This document defines the planned modular firmware architecture, C++ interfaces, FreeRTOS task models, and PlatformIO configuration for BlindSpottr. Refer to `AGENTS.md` for coding directives and `docs/flow.md` for authoritative hardware truth.
>
> *Note: This document describes the **planned implementation strategy**. Firmware implementation will take place during authorized development phases according to `TODO.md`.*

---

## 1. Directory Structure & Module Boundaries

The planned firmware layout organizes code into single-responsibility modules:

```text
BlindSpottr/
├── include/
│   └── config.h                  # Centralized GPIO pinout, timing, and Wi-Fi constants
├── src/
│   ├── main.cpp                  # Startup sequence, FreeRTOS task initialization, main loop
│   ├── sensors/
│   │   ├── radar.h / .cpp        # RCWL-0516 Doppler radar sampler & hold timer logic
│   │   └── ir.h / .cpp           # Short-range IR obstacle sampler with polarity normalization
│   ├── logic/
│   │   ├── fusion.h / .cpp       # Pure boolean fusion engine (IR || Radar_Hold)
│   │   └── state_machine.h / .cpp# Deterministic 4-state warning machine
│   ├── outputs/
│   │   ├── indicators.h / .cpp   # Non-blocking Left, Right, and Safe LED drivers
│   │   ├── buzzer.h / .cpp       # Active buzzer cadence generator (pulsed / solid / rapid)
│   │   └── display.h / .cpp      # SSD1306 OLED display driver and graphics layouts
│   └── network/
│       ├── wifi_manager.h / .cpp # Wi-Fi SoftAP controller (SSID: BlindSpottr-AP)
│       ├── web_server.h / .cpp   # AsyncWebServer and WebSocket (/ws) telemetry engine
│       └── dashboard_html.h      # Standalone PROGMEM HTML5/CSS3/JS phone dashboard
├── test/
│   ├── test_fusion/              # Unity unit test suite for fusion and state machine logic
│   └── test_timing/              # Unit tests for non-blocking radar hold windows
└── platformio.ini                # PlatformIO build environments and library dependencies
```

---

## 2. Configuration Module (`include/config.h`)

All hardware pin mappings and operational parameters must be centralized here. Magic numbers in module code are strictly forbidden.

```cpp
#pragma once
#include <Arduino.h>

// ==========================================
// GPIO Hardware Pin Definitions
// ==========================================
#define PIN_RADAR_L       32  // Left RCWL-0516 radar (Input, active-HIGH)
#define PIN_RADAR_R       33  // Right RCWL-0516 radar (Input, active-HIGH)
#define PIN_IR_L          34  // Left IR sensor (Input-only GPI, active-LOW)
#define PIN_IR_R          35  // Right IR sensor (Input-only GPI, active-LOW)

#define PIN_LED_L         25  // Left warning LED - Red (Output, active-HIGH)
#define PIN_LED_R         26  // Right warning LED - Red (Output, active-HIGH)
#define PIN_LED_SAFE      27  // Safe state LED - Green (Output, active-HIGH)
#define PIN_BUZZER        14  // Active buzzer (Output, active-HIGH)

#define PIN_I2C_SDA       21  // OLED I2C SDA
#define PIN_I2C_SCL       22  // OLED I2C SCL
#define OLED_I2C_ADDR     0x3C
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64

// ==========================================
// Sensor & Fusion Timing Constants
// ==========================================
#define SENSING_CADENCE_MS     20    // 50 Hz real-time sensing loop
#define RADAR_HOLD_MS          3000  // 3-second radar trigger latch window
#define IR_DETECT_LEVEL        LOW   // Active-LOW obstacle detection default
#define IR_DEBOUNCE_MS         30    // Glitch filtering window

// ==========================================
// Alert Cadence Timings (Non-Blocking)
// ==========================================
#define SINGLE_WARN_PULSE_MS   120   // Blink/beep duration for Left/Right warning
#define BOTH_WARN_PULSE_MS     60    // Rapid pulse duration for dual hazard

// ==========================================
// Wi-Fi SoftAP & Networking
// ==========================================
#define WIFI_AP_SSID      "BlindSpottr-AP"
#define WIFI_AP_PASS      "blindspot123"
#define WIFI_AP_CHANNEL   6
#define WIFI_MAX_CLIENTS  4
#define WS_HEARTBEAT_MS   200   // Periodic telemetry broadcast interval
```

---

## 3. Data Structures & Cross-Core Thread Safety

### 3.1 Shared State Definition
```cpp
#pragma once
#include <stdint.h>

enum SystemStateType : uint8_t {
    STATE_SAFE = 0,
    STATE_LEFT_WARNING = 1,
    STATE_RIGHT_WARNING = 2,
    STATE_BOTH_WARNING = 3
};

struct SystemState {
    SystemStateType state;
    bool leftOccupied;
    bool rightOccupied;
    bool leftRadarRaw;
    bool rightRadarRaw;
    bool leftRadarHold;
    bool rightRadarHold;
    bool leftIRRaw;
    bool rightIRRaw;
    uint32_t uptimeMs;
};
```

### 3.2 Thread-Safe State Manager
```cpp
class StateManager {
public:
    static void init();
    static void writeState(const SystemState &newState);
    static SystemState readState();

private:
    static SystemState _currentState;
    static portMUX_TYPE _spinlock;
};
```
*Implementation Rule:* Uses `portENTER_CRITICAL(&_spinlock)` and `portEXIT_CRITICAL(&_spinlock)` to ensure deterministic microsecond-level copy without blocking the 20 ms Core 0 sensing task.

---

## 4. Subsystem Interfaces

### 4.1 Sensor Subsystem (`src/sensors/`)
```cpp
// radar.h
class RadarSensor {
public:
    RadarSensor(uint8_t pin, uint32_t holdTimeMs);
    void begin();
    bool update(uint32_t now); // Samples pin, updates hold window, returns hold status
    bool isTriggered() const;
    bool isHoldActive() const;
private:
    uint8_t _pin;
    uint32_t _holdTimeMs;
    uint32_t _lastTriggerTimestamp;
    bool _rawState;
};

// ir.h
class IRSensor {
public:
    IRSensor(uint8_t pin, uint8_t activeLevel);
    void begin();
    bool read(); // Returns true if obstacle is detected according to configured polarity
private:
    uint8_t _pin;
    uint8_t _activeLevel;
};
```

### 4.2 Fusion & State Machine (`src/logic/`)
```cpp
// fusion.h
struct SensorInputs {
    bool leftRadarHold;
    bool rightRadarHold;
    bool leftIRObject;
    bool rightIRObject;
};

struct FusedOccupancy {
    bool leftOccupied;
    bool rightOccupied;
};

FusedOccupancy evaluateFusion(const SensorInputs &inputs);

// state_machine.h
SystemStateType evaluateFSM(bool leftOccupied, bool rightOccupied);
```

### 4.3 Output Drivers (`src/outputs/`)
```cpp
// indicators.h
class IndicatorController {
public:
    IndicatorController(uint8_t pinLeft, uint8_t pinRight, uint8_t pinSafe);
    void begin();
    void update(SystemStateType state, uint32_t now);
};

// buzzer.h
class BuzzerController {
public:
    BuzzerController(uint8_t pin);
    void begin();
    void update(SystemStateType state, uint32_t now);
};

// display.h
class DisplayController {
public:
    DisplayController(uint8_t sda, uint8_t scl);
    bool begin();
    void render(const SystemState &state, bool wifiConnected);
};
```

### 4.4 Networking & Web Dashboard (`src/network/`)
- Uses `ESPAsyncWebServer` and `AsyncTCP`.
- Route `/` serves `DASHBOARD_HTML` from PROGMEM with `text/html` header and gzip/cache options.
- Route `/ws` accepts WebSocket connections. Dispatches `textAll()` on state changes.
- Route `/api/status` provides optional fallback HTTP JSON endpoint.

---

## 5. PlatformIO Configuration (`platformio.ini`)

Dependencies planned for Phase 1 integration:

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
build_type = debug
monitor_speed = 115200

lib_deps =
    adafruit/Adafruit SSD1306 @ ^2.5.9
    adafruit/Adafruit GFX Library @ ^1.11.9
    bblanchon/ArduinoJson @ ^7.0.4
    mathieucarbou/ESPAsyncWebServer @ ^3.1.5
    mathieucarbou/AsyncTCP @ ^3.2.4

build_flags =
    -D CORE_DEBUG_LEVEL=3
    -D CONFIG_ASYNC_TCP_RUNNING_CORE=1
```
*(Note: `mathieucarbou/ESPAsyncWebServer` is the actively maintained, ESP32-S3/IDFv5/Arduino3 compatible fork of the legacy library).*

---

## 6. Unit Testing Strategy (`test/`)

1. **Host-Native Simulation:** Test `evaluateFusion()` and `evaluateFSM()` using Unity test framework compiled natively on macOS/Linux.
2. **Timing Edge Cases Tested:**
   - Single radar pulse latching for exactly $3000\text{ ms} \pm 20\text{ ms}$.
   - Radar drop-off transitioning cleanly to IR detection without state flicker.
   - Dual hazard priority override when left and right trigger simultaneously.
