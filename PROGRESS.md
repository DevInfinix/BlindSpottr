# PROGRESS.md: Implementation Tracking and Verification Log

This document tracks verified implementation milestones, physical hardware audit results, and active progress on BlindSpottr.

---

## 1. Verified Hardware State (Release v1.0.0)

- **Target MCU**: ESP32 DevKit V1 38-Pin (`esp32doit-devkit-v1`).
- **Physical IR Sensor Pins**:
  - Left IR Digital Out: **GPIO 35** (Verified on physical chassis).
  - Right IR Digital Out: **GPIO 32** (Verified on physical chassis).
  - Sensor Polarity: Active-LOW (`LOW = Obstacle present`).
  - Signal Type: Digital comparator outputs with onboard pull-ups.
- **Physical Ultrasonic Sensor Pins**:
  - Trigger: **GPIO 26** (10µs start pulse).
  - Echo: **GPIO 36 (VP)** (Non-blocking bounded pulse reading, ringdown filtered > 400µs).
  - Power: **VIN (5V)**.
- **Physical Buzzer Pin**: **GPIO 25**.
  - Acoustic Driver: ESP32 hardware LEDC PWM (Channel 0, 8-bit).
  - Audio Policy: Sounds **strictly** for Left, Right, or Dual IR blind-spot hazards. Decoupled from ultrasonic distance.
- **Physical OLED Pinout**:
  - I2C Data (SDA): **GPIO 21**.
  - I2C Clock (SCL): **GPIO 22**.
  - Display: SSD1306 128x64 @ 400 kHz Fast I2C, 50 ms timeout protection.
- **Network / Telemetry Interface**:
  - Wi-Fi SoftAP: `BlindSpottr-AP` (`192.168.4.1`, WPA2).
  - Auto-Connecting Hotspot Client: Supports any personal hotspot saved via web UI.
  - Stream: Asynchronous WebSockets (`ws://192.168.4.1/ws`) broadcasting JSON packets at 20 Hz.
  - In-Memory Web Perception HUD: Served directly from flash on `http://192.168.4.1/`.

---

## 2. Milestone Completion Status (v1.0.0 Stable)

- [x] **Milestone 1: PlatformIO Toolchain & Baseline**: Clean build configuration, library dependencies.
- [x] **Milestone 2: Centralized Hardware Pinout**: `include/config.h` configuration structure.
- [x] **Milestone 3: Optical Sensor Bring-Up**: GPIO 35/32 pin assignments, 15 ms debounce filtering.
- [x] **Milestone 4: Multi-Tone Buzzer Controller**: ESP32 LEDC PWM generator on GPIO 25, independent pitch variations for Left, Right, and Dual hazards.
- [x] **Milestone 5: Dynamic Cockpit OLED HUD**: 128x64 top-down truck model, side radar pulses, live centimeter readout.
- [x] **Milestone 6: Perspective Calibration**: Inverted wave and chevron animations to match overhead line of sight.
- [x] **Milestone 7: HC-SR04 Ultrasonic Distance Sensor**: Non-blocking pulse acquisition, acoustic ringdown rejection, decoupled from buzzer.
- [x] **Milestone 8: Documentation Unification & Schematics**: Merged master guide in `docs/flow.md`, 2D schematics in `docs/architecture.md`, and visual prompt library in `docs/design.md`.
- [x] **Milestone 9: Asynchronous FreeRTOS WebSocket Telemetry Server**: Core 1 SoftAP streaming JSON packets at 20 Hz.
- [x] **Milestone 10: Tesla-Style 3D Web Perception HUD**: Pure in-memory WebGL canvas, gear indicators, dynamic 3D obstacle proxy, and interactive Wi-Fi manager.

---

## 3. Active Release

- **Current Release**: **v1.0.0** (Production Ready).
- **Active Branch**: `main`.
