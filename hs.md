# hs.md: Quick Orientation and Handoff Summary

This file provides rapid context for working on BlindSpottr (v1.0.0 Stable).

---

## 1. Quick Facts

- **Release**: **v1.0.0** (Production Release).
- **Target Microcontroller**: ESP32 DevKit V1 38-Pin (`esp32doit-devkit-v1`).
- **Framework**: Arduino on PlatformIO (`~/.platformio/penv/bin/pio` or `/opt/homebrew/bin/pio`).
- **Active Branch**: `main`.
- **Network / Telemetry**:
  - Wi-Fi SoftAP: `BlindSpottr-AP` (Password: `blindspottr123`, IP: `192.168.4.1`).
  - In-Memory Web HUD: `http://192.168.4.1` (or `http://blindspottr.local`).
  - WebSocket: `ws://192.168.4.1/ws` streaming JSON telemetry at 20 Hz.
- **Physical Pinout**:
  - Left IR Digital Out: **GPIO 35** (input-only, active-LOW).
  - Right IR Digital Out: **GPIO 32** (digital input with pull-up, active-LOW).
  - Ultrasonic Trigger: **GPIO 26** (3.3V digital output).
  - Ultrasonic Echo: **GPIO 36 (VP)** (12 ms bounded pulse).
  - Ultrasonic VCC: **VIN (5V)**.
  - Buzzer Output: **GPIO 25** (LEDC PWM Channel 0).
  - OLED SDA: **GPIO 21** (I2C data, 3.3V).
  - OLED SCL: **GPIO 22** (I2C clock, 400 kHz fast mode).

---

## 2. Working Commands

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
