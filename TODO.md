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
| **Phase 6** | **Ultrasonic Front Headway Sensing (HC-SR04)** | COMPLETED |
| **Phase 7** | **Documentation, Schematics & Prompt Library** | COMPLETED |
| **Phase 8** | **ESP32 FreeRTOS WebSocket Telemetry Server** | COMPLETED |
| **Phase 9** | **Cross-Platform Flutter Mobile App (Tesla-Style 3D HUD)** | COMPLETED |
| **Phase 10** | **Interactive Settings & Proximity Calibration** | COMPLETED |

---

## Phase Details

### Phase 1: PlatformIO Foundation & Pin Configuration (COMPLETED)
- [x] Configure `platformio.ini` with `esp32doit-devkit-v1` board, Arduino framework, and libraries (`Adafruit SSD1306`, `Adafruit GFX`, `ArduinoJson`, `ESPAsyncWebServer`, `AsyncTCP`).
- [x] Create `include/config.h` with centralized GPIO constants, eliminating magic numbers from source files.

### Phase 2: IR Sensor Bring-Up & Debounce (COMPLETED)
- [x] Audit physical sensor wiring: Left IR = GPIO 35, Right IR = GPIO 32.
- [x] Handle ESP32 input-only silicon rules (GPI, external pull-up required).
- [x] Implement 15 ms non-blocking software debounce filter in `src/sensors.cpp`.

### Phase 3: Multi-Tone Buzzer Controller (COMPLETED)
- [x] Configure ESP32 hardware LEDC PWM on GPIO 25.
- [x] Assign distinct pitches: Left = 2200 Hz, Right = 2900 Hz, Multi = 3400/2400 Hz alternating chirp.
- [x] Decouple buzzer from front ultrasonic measurements.

### Phase 4: Graphic OLED HUD Dashboard (COMPLETED)
- [x] Implement top-down pixel-art truck model on SSD1306 (128x64).
- [x] Add animated side radar wave pulses, top chevrons, and inverted warning badges.
- [x] Add continuous forward scanning radar beams and flowing road guidelines.
- [x] Protect I2C bus with `Wire.setTimeOut(50)` and 400 kHz clock.

### Phase 5: Perspective Calibration & Visual Tuning (COMPLETED)
- [x] Calibrate visual animation and chevron mapping to match overhead viewing perspective.

### Phase 6: Ultrasonic Front Headway Sensing (COMPLETED)
- [x] Connect HC-SR04 ultrasonic sensor (TRIG: GPIO 26, ECHO: GPIO 36).
- [x] Measure pulse duration with non-blocking bounded timeout (12 ms / ~200 cm).
- [x] Display live distance in centimeters above the cab on the OLED HUD.

### Phase 7: Documentation, Schematics & Prompt Library (COMPLETED)
- [x] Merge implementation specs into `docs/flow.md` as unified reference.
- [x] Build 2D visual layout and schematics in `docs/architecture.md`.
- [x] Humanize `README.md` and clean up project documentation.
- [x] Transform `docs/design.md` into an AI visual reference library.

### Phase 8: ESP32 FreeRTOS WebSocket Telemetry Server (COMPLETED)
- [x] Spin up Wi-Fi SoftAP (`BlindSpottr-AP`, WPA2, IP `192.168.4.1`).
- [x] Host asynchronous WebSocket server on `/ws` with `ESPAsyncWebServer` and `AsyncTCP`.
- [x] Stream JSON telemetry packets at 20 Hz (`{ id, left, right, front_valid, front_cm, state, uptime_ms }`).

### Phase 9: Cross-Platform Mobile App with Tesla-Style 3D HUD (COMPLETED)
- [x] Scaffold Flutter mobile application (`mobile/` workspace).
- [x] Build Three.js 3D WebGL viewport (`mobile/web/threejs/index.html`):
  - 3D truck model in the center of an infinite dynamic grid/road.
  - Live animated curved radar wave bands pulsing on Left and Right flanks when obstacles are detected.
  - Dynamically moving 3D lead vehicle or obstacle proxy appearing ahead of the truck at the exact measured centimeter distance.
  - Color-coded safety zones (Green = Clear, Yellow = Caution, Red = Hazard).

### Phase 10: Interactive Settings & Calibration (COMPLETED)
- [x] Mobile Settings Screen (`mobile/lib/views/settings/settings_screen.dart`):
  - Front Hazard Threshold slider (10 cm to 100 cm).
  - Cockpit Multi-Tone Buzzer toggle.
  - WebSocket host configuration (`192.168.4.1`) and auto-reconnect controller.
