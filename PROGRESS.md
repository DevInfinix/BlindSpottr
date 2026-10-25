# PROGRESS.md: Implementation Tracking and Verification Log

This document tracks verified implementation milestones, physical hardware audit results, and active progress on BlindSpottr.

---

## 1. Verified Hardware State

- **Target MCU**: ESP32 DevKit V1 38-Pin (`esp32doit-devkit-v1`).
- **Physical IR Sensor Pins**:
  - Left IR Digital Out: **GPIO 34** (Verified on physical chassis).
  - Right IR Digital Out: **GPIO 35** (Verified on physical chassis).
  - Sensor Polarity: Active-LOW (`LOW = Obstacle present`).
  - Signal Type: Digital comparator outputs with onboard pull-ups.
- **Physical Ultrasonic Sensor Pins**:
  - Trigger: **GPIO 26** (10µs start pulse).
  - Echo: **GPIO 36 (VP)** (Stepped down from 5V via 1kΩ / 2.7kΩ voltage divider).
  - Power: **VIN (5V)**.
- **Physical Buzzer Pin**: **GPIO 25**.
  - Acoustic Driver: ESP32 hardware LEDC PWM (Channel 0, 8-bit).
  - Frequencies: Left = 2200 Hz, Right = 2900 Hz, Front = 1800 Hz, Multi = 3400/2400 Hz alternating chirp.
- **Physical OLED Pinout**:
  - I2C Data (SDA): **GPIO 21**.
  - I2C Clock (SCL): **GPIO 22**.
  - I2C Address: Automatically scanned (`0x3C` confirmed).
  - Bus Speed: 400 kHz fast mode with 50 ms timeout protection.

---

## 2. Commit Audit Trail

Recent commits on integration branch `dev` and release branch `main`:

| Commit SHA | Type | Summary |
| :--- | :--- | :--- |
| `83ece63` | `DOCS` | Transform design.md into AI visual prompt library and remove live.md (fixes #4) |
| `1bdaedb` | `DOCS` | Humanize README.md removing AI writing patterns, em dashes, and decorative emojis |
| `6defc09` | `FIX` | Invert animation wave and chevron mapping in OLED HUD to match top-down perspective |
| `2c34c2f` | `FEAT` | Align left/right animations and add forward scanning radar beams to OLED HUD |
| `d36636d` | `FIX` | Invert IR sensor pin assignments (GPIO 34 = Left, GPIO 35 = Right) |
| `aefa924` | `FEAT` | Complete dynamic OLED HUD, multi-tone buzzer alert engine, and verified IR sensor acquisition |
| `7906436` | `DOCS` | Document macOS serial port exclusive lock resolution |

---

## 3. Milestone Completion Status

- [x] **Milestone 1: Project Reconnaissance & Toolchain**: PlatformIO environment, Git branches (`main`, `dev`), library dependencies.
- [x] **Milestone 2: Centralized Hardware Configuration**: Pin mappings, timing, and frequencies in `include/config.h`.
- [x] **Milestone 3: Optical Sensor Bring-Up**: Verified GPIO 34/35 pin assignments, 15 ms debounce filtering, active-LOW normalization.
- [x] **Milestone 4: Multi-Tone Buzzer Controller**: ESP32 LEDC PWM generator on GPIO 25, independent pitch variations for Left, Right, Front, and Multi hazards.
- [x] **Milestone 5: Dynamic 16 FPS Graphic OLED HUD**: 128x64 top-down truck model, side radar pulses, live centimeter readout, flowing road markers.
- [x] **Milestone 6: Perspective Calibration**: Inverted wave and chevron animations to match overhead line of sight.
- [x] **Milestone 7: HC-SR04 Ultrasonic Distance Sensor**: Wired via 1kΩ / 2.7kΩ voltage divider into GPIO 36 (VP), non-blocking acquisition, headway alerts.
- [x] **Milestone 8: Documentation Unification & Prompt Library**: Unified specifications into `docs/flow.md`, added 2D visual layouts to `docs/architecture.md`, transformed `docs/design.md` into AI prompt library.

---

## 4. Active Work and Next Steps

- **Active State**: Dual-side blind-spot monitoring and front ultrasonic distance tracking operational.
- **Upcoming Milestone**: Directional turn signal input escalation.
