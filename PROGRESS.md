# PROGRESS.md: Implementation Tracking and Verification Log

This document tracks verified implementation milestones, physical hardware audit results, and active progress on BlindSpottr.

---

## 1. Verified Hardware State

- **Target MCU**: ESP32 DevKit V1 38-Pin (`esp32doit-devkit-v1`).
- **Physical IR Sensor Pins**:
  - Left IR Digital Out: **GPIO 35** (Verified on physical chassis).
  - Right IR Digital Out: **GPIO 32** (Verified on physical chassis).
  - Sensor Polarity: Active-LOW (`LOW = Obstacle present`).
  - Signal Type: Digital comparator outputs with onboard pull-ups.
- **Physical Ultrasonic Sensor Pins**:
  - Trigger: **GPIO 26** (10µs start pulse).
  - Echo: **GPIO 36 (VP)** (12 ms non-blocking bounded timeout).
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
  - Stream: Asynchronous WebSockets (`ws://192.168.4.1/ws`) broadcasting JSON packets at 20 Hz.

---

## 2. Commit Audit Trail

Recent commits on integration branch `dev` and release branch `main`:

| Commit SHA | Type | Summary |
| :--- | :--- | :--- |
| `c36b52e` | `FIX` | Flip OLED overhead perspective, decouple ultrasonic from buzzer, and non-blocking telemetry (fixes #12) |
| `e1ae96b` | `FIX` | Ultrasonic median filter, direct 1:1 badge alignment, and state resolution (fixes #10) |
| `9121a21` | `FIX` | Remap physical pin assignments (Left IR=GPIO 35, Right IR=GPIO 32) and update schematics (fixes #8) |
| `b102d3f` | `FEAT` | Integrate HC-SR04 ultrasonic distance sensor, update 2D wiring and sync docs (fixes #6) |
| `83ece63` | `DOCS` | Transform design.md into AI visual prompt library and remove live.md (fixes #4) |
| `1bdaedb` | `DOCS` | Humanize README.md removing AI writing patterns, em dashes, and decorative emojis |

---

## 3. Milestone Completion Status

- [x] **Milestone 1: Project Reconnaissance & Toolchain**: PlatformIO environment, Git branches (`main`, `dev`), library dependencies.
- [x] **Milestone 2: Centralized Hardware Configuration**: Pin mappings, timing, and frequencies in `include/config.h`.
- [x] **Milestone 3: Optical Sensor Bring-Up**: Verified GPIO 35/32 pin assignments, 15 ms debounce filtering, active-LOW normalization.
- [x] **Milestone 4: Multi-Tone Buzzer Controller**: ESP32 LEDC PWM generator on GPIO 25, independent pitch variations for Left, Right, and Dual hazards.
- [x] **Milestone 5: Dynamic 16 FPS Graphic OLED HUD**: 128x64 top-down truck model, side radar pulses, live centimeter readout, flowing road markers.
- [x] **Milestone 6: Perspective Calibration**: Inverted wave and chevron animations to match overhead line of sight.
- [x] **Milestone 7: HC-SR04 Ultrasonic Distance Sensor**: Non-blocking pulse acquisition, median filter, decoupled from buzzer.
- [x] **Milestone 8: Documentation Unification & Prompt Library**: Unified specifications into `docs/flow.md`, added 2D visual layouts to `docs/architecture.md`, transformed `docs/design.md` into AI prompt library.
- [x] **Milestone 9: Asynchronous FreeRTOS WebSocket Telemetry Server**: Core 1 SoftAP streaming JSON packets at 20 Hz (`ws://192.168.4.1/ws`).
- [x] **Milestone 10: Cross-Platform Mobile App (Tesla-Style 3D HUD)**: Flutter cross-platform architecture with embedded Three.js WebGL rendering, dynamic 3D vehicle proxy, and settings suite.

---

## 4. Active Work and Next Steps

- **Hardware Status**: Hardware freeze established as requested. No further physical wiring required.
- **Active State**: Dual-side blind-spot monitoring, front distance tracking, local OLED HUD, and live 3D mobile telemetry operational.
