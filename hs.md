# hs.md — BlindSpottr Rapid Context & Handoff Summary

> **QUICK START:** This file is the primary quick-orientation document for any AI coding agent starting a session on **BlindSpottr**. For full engineering rules, refer to `AGENTS.md`. For authoritative hardware specifications, refer to `docs/flow.md`.

---

## 1. Project Identity & Objective

- **Project:** BlindSpottr
- **System Type:** ESP32-based intelligent automotive blind-spot monitoring prototype for a model vehicle.
- **Target Microcontroller:** ESP32 38-pin DevKit V1 (`esp32doit-devkit-v1`).
- **Firmware Environment:** PlatformIO with Arduino Framework in VS Code.
- **Core Concept:** Fuses short-range moving-object detection (**2× RCWL-0516 microwave radar**) and stationary-object presence (**2× IR sensors**) to eliminate blind-spot false negatives when vehicles stop. Outputs warnings locally (left/right red LEDs, green safe LED, active buzzer, 0.96" I2C OLED) and over an onboard Wi-Fi SoftAP to an interactive mobile phone dashboard via WebSockets.

---

## 2. Mandatory Reading Order

When beginning a new session or resuming development, read documents in this exact order:

```text
1. AGENTS.md                → Master engineering rules, safety, and Git conventions
2. hs.md                    → This file (high-level state and handoff)
3. docs/flow.md             → Authoritative technical specification & hardware pinouts
4. docs/architecture.md     → Subsystems, concurrency, and Mermaid data flows
5. docs/design.md           → State transitions, UI layouts, and design trade-offs
6. docs/implementation.md   → Module interfaces, headers, and FreeRTOS task models
7. TODO.md                  → Phased implementation roadmap & task acceptance criteria
8. PROGRESS.md              → Current verified state, audit logs, and known issues
```

---

## 3. Authoritative Hardware Pinout Summary

Centralized pinout from `docs/flow.md`:

| Signal Name | ESP32 GPIO | Direction | Function / Hardware Notes |
| :--- | :--- | :--- | :--- |
| `PIN_RADAR_L` | **GPIO 32** | Input | Left RCWL-0516 radar Doppler pulse (Active-HIGH) |
| `PIN_RADAR_R` | **GPIO 33** | Input | Right RCWL-0516 radar Doppler pulse (Active-HIGH) |
| `PIN_IR_L` | **GPIO 34** | Input | Left IR obstacle sensor (Input-only GPI, active-LOW) |
| `PIN_IR_R` | **GPIO 35** | Input | Right IR obstacle sensor (Input-only GPI, active-LOW) |
| `PIN_LED_L` | **GPIO 25** | Output | Left red alert LED (Active-HIGH) |
| `PIN_LED_R` | **GPIO 26** | Output | Right red alert LED (Active-HIGH) |
| `PIN_LED_SAFE`| **GPIO 27** | Output | Green safe LED (Active-HIGH, ON during `STATE_SAFE`) |
| `PIN_BUZZER` | **GPIO 14** | Output | Active buzzer (Active-HIGH alert cadences) |
| `PIN_I2C_SDA` | **GPIO 21** | I/O | SSD1306 OLED I2C Data (Address `0x3C`, 3.3V) |
| `PIN_I2C_SCL` | **GPIO 22** | Output | SSD1306 OLED I2C Clock (Address `0x3C`, 3.3V) |

*Safety Reminder:* GPIO 34 & 35 are input-only pins without internal pull-ups; strapping pins (GPIO 0, 2, 12, 15) must remain unused.

---

## 4. Key Architectural Constants & Rules

1. **Sensor Fusion Formula:**
   $$\text{side\_occupied} = \text{IR\_detected} \lor \text{radar\_hold\_active}$$
2. **Radar Hold Duration:** `3000 ms` (`RADAR_HOLD_MS`). Radar triggers latch for 3 seconds to bridge vehicle movement to stationary stopping.
3. **Sensing Cadence:** `20 ms` (50 Hz) on FreeRTOS Core 0.
4. **Deterministic States:** Exactly 4 states: `STATE_SAFE (0)`, `STATE_LEFT_WARNING (1)`, `STATE_RIGHT_WARNING (2)`, `STATE_BOTH_WARNING (3)`.
5. **Non-Blocking Rule:** `delay()` is strictly forbidden in runtime code. Use `millis()` software timers and FreeRTOS yields.
6. **Web Dashboard Delivery:** Embedded in flash memory via **PROGMEM HTML/CSS/JS** (`src/network/dashboard_html.h`). No SPIFFS/LittleFS image uploads required.
7. **Telemetry Tick:** WebSocket `/ws` updates pushed immediately on state change, plus a 200 ms periodic telemetry packet.

---

## 5. Current Development State

- **Current Phase:** Phase 0 (Initialization & Engineering Reconnaissance) complete.
- **Firmware Status:** **NO firmware implementation has been performed.** Source directory contains only the initial template (`src/main.cpp`).
- **Repository Status:** Documentation architecture fully established. Git repository and GitHub integration ready for Phase 1.
- **Next Recommended Action:** **Phase 1, Task 1.1** — Configure `platformio.ini` dependencies (`lib_deps`), verify compile pipeline with `pio run`, and establish `include/config.h` pinout definitions.
