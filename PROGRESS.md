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
- **Physical Buzzer Pin**: **GPIO 25**.
  - Acoustic Driver: ESP32 hardware LEDC PWM (Channel 0, 8-bit).
  - Frequencies: Left = 2200 Hz, Right = 2900 Hz, Dual = 3400/2400 Hz alternating chirp.
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
| `1bdaedb` | `DOCS` | Humanize README.md removing AI writing patterns, em dashes, and decorative emojis |
| `6defc09` | `FIX` | Invert animation wave and chevron mapping in OLED HUD to match top-down perspective |
| `2c34c2f` | `FEAT` | Align left/right animations and add forward scanning radar beams to OLED HUD |
| `d36636d` | `FIX` | Invert IR sensor pin assignments (GPIO 34 = Left, GPIO 35 = Right) |
| `aefa924` | `FEAT` | Complete dynamic OLED HUD, multi-tone buzzer alert engine, and verified IR sensor acquisition |
| `7906436` | `DOCS` | Document macOS serial port exclusive lock resolution |
| `a0563d1` | `FEAT` | Implement non-blocking LED indicator controller |
| `c1889e7` | `CONFIG`| Define centralized GPIO pinout and operational constants |
| `dafaada` | `BUILD` | Configure PlatformIO dependencies and build flags in platformio.ini |

---

## 3. Milestone Completion Status

- [x] **Milestone 1: Project Reconnaissance & Toolchain**: PlatformIO environment, Git branches (`main`, `dev`), library dependencies.
- [x] **Milestone 2: Centralized Hardware Configuration**: Pin mappings, timing, and frequencies in `include/config.h`.
- [x] **Milestone 3: Sensor Hardware Bring-Up**: Verified GPIO 34/35 pin assignments, 15 ms debounce filtering, active-LOW normalization.
- [x] **Milestone 4: Multi-Tone Buzzer Controller**: ESP32 LEDC PWM generator on GPIO 25, independent pitch variations for Left, Right, and Dual hazards.
- [x] **Milestone 5: Dynamic 16 FPS Graphic OLED HUD**: 128x64 top-down truck model, side radar pulses, forward scanner beams, flowing road markers.
- [x] **Milestone 6: Perspective Calibration**: Inverted wave and chevron animations to match the user's overhead line of sight.
- [x] **Milestone 7: Documentation Unification**: Merged implementation specifications into `docs/flow.md`, rebuilt `docs/architecture.md` with 2D visual layouts, and humanized project prose.

---

## 4. Active Work and Next Steps

- **Active State**: Base dual-side blind-spot monitoring prototype is operational and verified on breadboard hardware.
- **Upcoming Milestone**: Optional front collision avoidance (integrating HC-SR04 on GPIO 26 TRIG, GPIO 36 ECHO via voltage divider).
- **Upcoming Milestone**: Directional turn signal input escalation.
