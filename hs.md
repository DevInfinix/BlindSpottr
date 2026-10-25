# hs.md: Quick Orientation and Handoff Summary

This file provides rapid context for working on BlindSpottr.

---

## 1. Quick Facts

- **Project**: BlindSpottr (Toy-Truck Blind-Spot Monitoring, Headway Sensing, and Live 3D Mobile Telemetry).
- **Target Microcontroller**: ESP32 DevKit V1 38-Pin (`esp32doit-devkit-v1`).
- **Framework**: Arduino on PlatformIO (`~/.platformio/penv/bin/pio` or `/opt/homebrew/bin/pio`).
- **Mobile Stack**: Flutter + Three.js WebGL 3D Canvas (`mobile/`).
- **Network / Telemetry**:
  - Wi-Fi SoftAP: `BlindSpottr-AP` (Password: `blindspottr123`, IP: `192.168.4.1`).
  - WebSocket: `ws://192.168.4.1/ws` streaming JSON at 20 Hz.
- **Physical Pin Mapping**:
  - Left IR Digital Out: **GPIO 35** (input-only, active-LOW).
  - Right IR Digital Out: **GPIO 32** (input-only, active-LOW).
  - Ultrasonic Trigger: **GPIO 26** (3.3V digital output).
  - Ultrasonic Echo: **GPIO 36 (VP)** (12 ms bounded pulse).
  - Ultrasonic VCC: **VIN (5V)**.
  - Buzzer Output: **GPIO 25** (LEDC PWM Channel 0).
  - OLED SDA: **GPIO 21** (I2C data, 3.3V).
  - OLED SCL: **GPIO 22** (I2C clock, 400 kHz fast mode).

---

## 2. Reading Order for New Sessions

```text
1. AGENTS.md            -> Engineering rules, non-blocking guidelines, Git workflow
2. hs.md                -> This file (quick state and handoff summary)
3. docs/flow.md         -> Authoritative project specification and layman guide
4. docs/architecture.md -> 2D component wiring illustration and subsystem data flow
5. docs/design.md       -> Industrial design and AI visual prompt library
6. TODO.md              -> Phased roadmap and task prerequisites
7. PROGRESS.md          -> Verified milestones, commit audit log, and hardware facts
8. Inspect codebase     -> 'git status', check active branch, run 'pio run'
```

---

## 3. Working Commands

- Compile firmware:
  ```bash
  pio run
  ```
- Upload to ESP32:
  ```bash
  pio run --target upload
  ```
- Monitor serial console:
  ```bash
  pio device monitor --baud 115200
  ```
- Run Flutter Mobile App:
  ```bash
  cd mobile && flutter run
  ```
