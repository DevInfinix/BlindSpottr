# BlindSpottr: Master Implementation Roadmap

This roadmap tracks active phases, prerequisites, and tasks across the development lifecycle of BlindSpottr.

---

## Roadmap Overview

| Phase | Description | Status |
| :--- | :--- | :--- |
| **Phase 1** | **PlatformIO Foundation & Pin Map** | COMPLETED |
| **Phase 2** | **IR Sensor Bring-Up & Debounce** | COMPLETED |
| **Phase 3** | **Multi-Tone Buzzer Controller** | COMPLETED |
| **Phase 4** | **Graphic OLED HUD Dashboard** | COMPLETED |
| **Phase 5** | **Perspective Calibration & Visual Tuning** | COMPLETED |
| **Phase 6** | **Documentation & Architectural Schematics** | COMPLETED |
| **Phase 7** | **Ultrasonic Front Collision Sensing** | PLANNED |
| **Phase 8** | **Turn-Signal Alert Escalation** | PLANNED |

---

## Phase Details

### Phase 1: PlatformIO Foundation & Pin Configuration (COMPLETED)
- [x] Configure `platformio.ini` with `esp32doit-devkit-v1` board, Arduino framework, and libraries (`Adafruit SSD1306`, `Adafruit GFX`).
- [x] Create `include/config.h` with centralized GPIO constants, eliminating magic numbers from source files.

### Phase 2: IR Sensor Bring-Up & Debounce (COMPLETED)
- [x] Audit physical sensor wiring: Left IR = GPIO 34, Right IR = GPIO 35.
- [x] Handle ESP32 input-only silicon rules (GPI, external pull-up required).
- [x] Implement 15 ms non-blocking software debounce filter in `src/sensors.cpp`.

### Phase 3: Multi-Tone Buzzer Controller (COMPLETED)
- [x] Configure ESP32 hardware LEDC PWM on GPIO 25.
- [x] Assign distinct pitches: Left = 2200 Hz, Right = 2900 Hz, Dual = 3400/2400 Hz alternating chirp.
- [x] Ensure immediate silence upon return to `SAFE_CLEAR`.

### Phase 4: Graphic OLED HUD Dashboard (COMPLETED)
- [x] Implement top-down pixel-art truck model on SSD1306 (128x64).
- [x] Add animated side radar wave pulses, top chevrons, and inverted warning badges.
- [x] Add continuous forward scanning radar beams and flowing road guidelines.
- [x] Protect I2C bus with `Wire.setTimeOut(50)` and 400 kHz clock.

### Phase 5: Perspective Calibration & Visual Tuning (COMPLETED)
- [x] Calibrate visual animation and chevron mapping to match overhead viewing perspective.
- [x] Verify that left obstacle triggers left radar waves and right obstacle triggers right radar waves.

### Phase 6: Documentation & Schematics (COMPLETED)
- [x] Merge `docs/implementation.md` into `docs/flow.md` as the unified layman-friendly reference.
- [x] Create 2D visual layout and pinout matrix in `docs/architecture.md`.
- [x] Humanize `README.md` and clean up project documentation.

### Phase 7: Ultrasonic Front Collision Sensing (PLANNED)
- [ ] Connect HC-SR04 ultrasonic sensor (TRIG: GPIO 26, ECHO: GPIO 36 via resistor divider).
- [ ] Add forward distance bar and emergency brake warning on OLED HUD.

### Phase 8: Turn-Signal Alert Escalation (PLANNED)
- [ ] Wire physical left and right turn-signal switches.
- [ ] Keep alerts visual-only when cruising; escalate to loud audio alerts when signaling toward an occupied lane.
