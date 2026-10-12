# BlindSpottr — Master Phased Implementation Roadmap

> **AUTHORITY:** This roadmap is the comprehensive development guide for the BlindSpottr firmware and system integration. Every task includes technical rationale, prerequisites, implementation steps, failure modes, and verifiable acceptance criteria.
>
> *Operational Rule:* Do NOT execute tasks out of order. Verify completion criteria before marking any task as complete.

---

## Roadmap Overview & Phase Status

| Phase | Title | Focus Area | Status |
| :--- | :--- | :--- | :--- |
| **Phase 0** | **Reconnaissance & Engineering Setup** | Specs, documentation, skills, and Git protocol | **COMPLETED** |
| **Phase 1** | **PlatformIO Foundation & Pin Configuration** | `platformio.ini`, `lib_deps`, `config.h` | **PENDING (ACTIVE)** |
| **Phase 2** | **Sensor Abstractions & Non-Blocking Hold** | Radar hold timer, IR polarity, debouncing | **PENDING** |
| **Phase 3** | **Sensor Fusion & 4-State Machine** | Boolean fusion, FSM transitions, unit tests | **PENDING** |
| **Phase 4** | **Local Actuation & Cockpit UI** | LEDs, active buzzer cadences, SSD1306 OLED | **PENDING** |
| **Phase 5** | **FreeRTOS Concurrency & Dual-Core Task** | Core 0 50 Hz task, spinlocks, state manager | **PENDING** |
| **Phase 6** | **Wi-Fi SoftAP & PROGMEM Dashboard** | SoftAP, AsyncWebServer, WebSockets, HTML5 UI | **PENDING** |
| **Phase 7** | **System Integration & Hardware Bring-Up** | Bench validation, noise immunity, fail-safes | **PENDING** |
| **Phase 8** | **CI Automation & Release Preparation** | GitHub Actions, build checks, demo guide | **PENDING** |

---

## Phase 0: Project Reconnaissance & Engineering Setup (COMPLETED)

### Task 0.1 — Repository Audit & Specification Preservation
- **Status:** **COMPLETED**
- **Objective:** Ingest `docs.md`, preserve specification in `docs/flow.md`, and audit hardware constraints.
- **Verification:** `docs/flow.md` created with all 1,727 lines preserved.

### Task 0.2 — Documentation Architecture Establishment
- **Status:** **COMPLETED**
- **Objective:** Create authoritative engineering guides: `AGENTS.md`, `hs.md`, `docs/architecture.md`, `docs/design.md`, and `docs/implementation.md`.
- **Verification:** All 5 documentation files created and cross-verified.

---

## Phase 1: PlatformIO Foundation & Pin Configuration

### Task 1.1 — PlatformIO Environment & Dependency Configuration
- **Status:** Pending
- **Objective:** Configure `platformio.ini` with verified libraries, build flags, and monitor baud rate.
- **Why this is necessary:** BlindSpottr requires Adafruit graphics, SSD1306 drivers, ArduinoJson, and asynchronous web serving. Setting this up first guarantees a stable compilation pipeline.
- **Prerequisites:** Task 0.2.
- **Specification references:** `docs/flow.md` §3, `docs/implementation.md` §5.
- **Implementation plan:**
  1. Update `platformio.ini` with `lib_deps`: `adafruit/Adafruit SSD1306`, `adafruit/Adafruit GFX Library`, `bblanchon/ArduinoJson`, `mathieucarbou/ESPAsyncWebServer`, and `mathieucarbou/AsyncTCP`.
  2. Set monitor speed to `115200`.
  3. Add build flags for debug levels and Core 1 network binding.
  4. Run `/opt/homebrew/bin/pio run` to verify clean library resolution and compilation.
- **Expected files:** `platformio.ini`.
- **Integration considerations:** Modern ESPAsyncWebServer fork (`mathieucarbou`) must be used for ESP-IDF / Arduino 2.x/3.x compatibility.
- **Testing & validation:** Run `pio run` locally; zero compilation warnings or unresolved include headers.
- **Potential failure cases:** Incompatible AsyncTCP versions on ESP32 dual-core.
- **Acceptance criteria:** `platformio.ini` resolves all libraries and compiles an empty sketch cleanly.
- **GitHub tracking:** Issue #1.

### Task 1.2 — Centralized Hardware Pinout & Configuration Header
- **Status:** Pending
- **Objective:** Implement `include/config.h` containing all pin definitions, timing parameters, and Wi-Fi credentials.
- **Why this is necessary:** Eliminates magic numbers across modules and enforces single-source-of-truth pinout rules.
- **Prerequisites:** Task 1.1.
- **Specification references:** `docs/flow.md` §5, §6; `AGENTS.md` §5.2.
- **Implementation plan:**
  1. Define radar pins: `PIN_RADAR_L (32)`, `PIN_RADAR_R (33)`.
  2. Define IR pins: `PIN_IR_L (34)`, `PIN_IR_R (35)`.
  3. Define LED pins: `PIN_LED_L (25)`, `PIN_LED_R (26)`, `PIN_LED_SAFE (27)`.
  4. Define Buzzer: `PIN_BUZZER (14)`.
  5. Define I2C OLED: `PIN_I2C_SDA (21)`, `PIN_I2C_SCL (22)`, `OLED_I2C_ADDR (0x3C)`.
  6. Define timing parameters: `SENSING_CADENCE_MS (20)`, `RADAR_HOLD_MS (3000)`, `IR_DETECT_LEVEL (LOW)`.
- **Expected files:** `include/config.h` (Proposed).
- **Integration considerations:** Must include `#pragma once` and compile without external dependencies.
- **Testing & validation:** Include `config.h` in `src/main.cpp` and verify clean build with `pio run`.
- **Potential failure cases:** Typo in pin numbers causing conflict with strapping pins.
- **Acceptance criteria:** `include/config.h` exists, defines all required constants, and builds without warnings.
- **GitHub tracking:** Issue #2.

---

## Phase 2: Sensor Abstraction & Non-Blocking Hold Timing

### Task 2.1 — RCWL-0516 Radar Sensor Driver with 3-Second Hold Latch
- **Status:** Pending
- **Objective:** Create a non-blocking radar sensor driver module in `src/sensors/radar.*` with a 3000 ms software hold window.
- **Why this is necessary:** Microwave Doppler radars output momentary active-HIGH pulses only during physical motion. The 3-second hold bridges the gap between motion and a vehicle stopping beside the car.
- **Prerequisites:** Task 1.2.
- **Specification references:** `docs/flow.md` §7, §9; `docs/implementation.md` §4.1.
- **Implementation plan:**
  1. Create class `RadarSensor` taking pin number and hold duration.
  2. Implement `update(uint32_t now)`: reads pin, updates `_lastTriggerTimestamp` when `digitalRead == HIGH`, and calculates if elapsed time $< \text{holdTime}$.
  3. Implement accessors `isTriggered()` and `isHoldActive()`.
- **Expected files:** `src/sensors/radar.h`, `src/sensors/radar.cpp` (Proposed).
- **Integration considerations:** Zero `delay()` calls; purely timestamp arithmetic using unsigned 32-bit math.
- **Testing & validation:** Test pulse latching via standalone test harness.
- **Potential failure cases:** Timer rollover glitches if subtraction is not unsigned (`now - lastTrigger`).
- **Acceptance criteria:** Radar pulse sets hold flag for exactly 3000 ms after the pulse drops LOW.
- **GitHub tracking:** Issue #3.

### Task 2.2 — Configurable-Polarity IR Obstacle Sensor Driver
- **Status:** Pending
- **Objective:** Create an IR obstacle presence driver in `src/sensors/ir.*` with configurable detection level.
- **Why this is necessary:** IR sensors detect stationary objects; modules can be active-LOW or active-HIGH depending on manufacturer batches.
- **Prerequisites:** Task 1.2.
- **Specification references:** `docs/flow.md` §8; `AGENTS.md` §5.3.
- **Implementation plan:**
  1. Create class `IRSensor` taking pin number and active polarity level (`IR_DETECT_LEVEL`).
  2. Configure pin as `INPUT` (remembering GPIO 34/35 have no internal pull-ups).
  3. Implement `read()` returning `true` when obstacle is present.
  4. Add optional 30 ms non-blocking debounce filter.
- **Expected files:** `src/sensors/ir.h`, `src/sensors/ir.cpp` (Proposed).
- **Integration considerations:** Must never attempt `pinMode(pin, INPUT_PULLUP)` on GPIO 34 or 35.
- **Testing & validation:** Verify software inversion toggle switches detection state.
- **Potential failure cases:** Floating input if external sensor is unplugged.
- **Acceptance criteria:** Module reliably reports obstacle detection according to configured polarity.
- **GitHub tracking:** Issue #4.

---

## Phase 3: Sensor Fusion Engine & 4-State Machine

### Task 3.1 — Pure Boolean Fusion Calculation Module
- **Status:** Pending
- **Objective:** Implement `src/logic/fusion.*` calculating $\text{occupied} = \text{IR} \lor \text{radar\_hold}$.
- **Why this is necessary:** Decouples hardware GPIO reads from decision logic, enabling complete host unit-testing.
- **Prerequisites:** Tasks 2.1, 2.2.
- **Specification references:** `docs/flow.md` §9; `docs/architecture.md` §3.
- **Implementation plan:**
  1. Define input struct `SensorInputs { bool leftRadarHold, rightRadarHold, leftIRObject, rightIRObject; }`.
  2. Define output struct `FusedOccupancy { bool leftOccupied, rightOccupied; }`.
  3. Implement `FusedOccupancy evaluateFusion(const SensorInputs &inputs)`.
- **Expected files:** `src/logic/fusion.h`, `src/logic/fusion.cpp` (Proposed).
- **Integration considerations:** Pure functional logic; no external dependencies or side effects.
- **Testing & validation:** Unit test all 16 input combinations ($2^4$) in host simulation.
- **Acceptance criteria:** All 16 truth-table combinations match $\text{occupied} = \text{IR} \lor \text{Hold}$.
- **GitHub tracking:** Issue #5.

### Task 3.2 — Deterministic 4-State Global Machine
- **Status:** Pending
- **Objective:** Implement `src/logic/state_machine.*` mapping occupancy flags to `STATE_SAFE`, `STATE_LEFT_WARNING`, `STATE_RIGHT_WARNING`, and `STATE_BOTH_WARNING`.
- **Why this is necessary:** Centralizes system state transitions and enforces priority rules (Both > Left/Right > Safe).
- **Prerequisites:** Task 3.1.
- **Specification references:** `docs/flow.md` §10; `docs/design.md` §3.
- **Implementation plan:**
  1. Implement `SystemStateType evaluateFSM(bool leftOccupied, bool rightOccupied)`.
  2. When both true: return `STATE_BOTH_WARNING`.
  3. When left true only: return `STATE_LEFT_WARNING`.
  4. When right true only: return `STATE_RIGHT_WARNING`.
  5. When neither true: return `STATE_SAFE`.
- **Expected files:** `src/logic/state_machine.h`, `src/logic/state_machine.cpp` (Proposed).
- **Integration considerations:** Pure function without state retention to guarantee idempotency.
- **Testing & validation:** Unit test all 4 combinations with transitions.
- **Acceptance criteria:** Transitions match the state matrix documented in `docs/architecture.md`.
- **GitHub tracking:** Issue #6.

### Task 3.3 — Automated Unit Test Suite for Fusion and State Transitions
- **Status:** Pending
- **Objective:** Implement automated Unity unit tests in `test/test_fusion/test_fusion.cpp`.
- **Why this is necessary:** Guarantees regression protection for core safety logic without requiring physical hardware.
- **Prerequisites:** Tasks 3.1, 3.2.
- **Specification references:** `docs/implementation.md` §6.
- **Implementation plan:**
  1. Write Unity test cases asserting fusion truth table.
  2. Write Unity test cases asserting FSM transitions and priority overrides.
  3. Configure PlatformIO test runner.
- **Expected files:** `test/test_fusion/test_fusion.cpp` (Proposed).
- **Integration considerations:** Can run on target MCU via `pio test` or native test runner.
- **Testing & validation:** Run `/opt/homebrew/bin/pio test`.
- **Acceptance criteria:** 100% of fusion and FSM unit tests pass.
- **GitHub tracking:** Issue #7.

---

## Phase 4: Cockpit Alert Actuation (LEDs, Buzzer, OLED)

### Task 4.1 — Non-Blocking Left, Right, and Safe LED Controller
- **Status:** Pending
- **Objective:** Create `src/outputs/indicators.*` to manage Red L/R alert LEDs and Green Safe LED without `delay()`.
- **Why this is necessary:** Visual alerts must pulse or hold steady while OLED and sensors continue running smoothly.
- **Prerequisites:** Task 1.2.
- **Specification references:** `docs/flow.md` §11; `docs/design.md` §3.1.
- **Implementation plan:**
  1. Implement `IndicatorController` class.
  2. In `STATE_SAFE`: Green LED ON, Red LEDs OFF.
  3. In `STATE_LEFT_WARNING`: Left Red LED pulsed (120 ms cadence), Green OFF.
  4. In `STATE_RIGHT_WARNING`: Right Red LED pulsed (120 ms cadence), Green OFF.
  5. In `STATE_BOTH_WARNING`: Both Red LEDs solid ON, Green OFF.
- **Expected files:** `src/outputs/indicators.h`, `src/outputs/indicators.cpp` (Proposed).
- **Integration considerations:** Use `millis()` delta checks for blink timing.
- **Testing & validation:** Logic inspection and bench verification.
- **Acceptance criteria:** LEDs switch states immediately with zero blocking delays.
- **GitHub tracking:** Issue #8.

### Task 4.2 — Active Buzzer Cadence Generator
- **Status:** Pending
- **Objective:** Create `src/outputs/buzzer.*` to generate distinct audible alert patterns for single vs dual hazards.
- **Why this is necessary:** Audio cues alert the driver without taking their eyes off the road.
- **Prerequisites:** Task 1.2.
- **Specification references:** `docs/flow.md` §12.
- **Implementation plan:**
  1. Implement `BuzzerController` class.
  2. Silent during `STATE_SAFE`.
  3. Intermittent beep (120 ms ON / 120 ms OFF) for Left or Right warning.
  4. Rapid urgent beep (60 ms ON / 60 ms OFF) for `STATE_BOTH_WARNING`.
- **Expected files:** `src/outputs/buzzer.h`, `src/outputs/buzzer.cpp` (Proposed).
- **Integration considerations:** Driven via transistor / GPIO 14 push-pull.
- **Testing & validation:** Verify timing intervals using non-blocking state machine.
- **Acceptance criteria:** Buzzer silences immediately when returning to `STATE_SAFE`.
- **GitHub tracking:** Issue #9.

### Task 4.3 — SSD1306 0.96" OLED Display Driver & Screen Rendering
- **Status:** Pending
- **Objective:** Implement `src/outputs/display.*` to render crisp hazard arrows, system status, and telemetry on 128×64 OLED.
- **Why this is necessary:** Provides clear visual diagnosis and driver alert status in the cockpit.
- **Prerequisites:** Task 1.1, 1.2.
- **Specification references:** `docs/flow.md` §13; `docs/design.md` §4.1.
- **Implementation plan:**
  1. Initialize `Adafruit_SSD1306` over I2C pins 21/22 at address `0x3C`.
  2. Add bus timeout check to prevent system hang if OLED is disconnected.
  3. Design 4 screen buffers: SAFE clear screen, Left chevron warning, Right chevron warning, Both warning screen.
  4. Render status footer: IP address, Wi-Fi client count, system uptime.
- **Expected files:** `src/outputs/display.h`, `src/outputs/display.cpp` (Proposed).
- **Integration considerations:** Refresh rate capped at 10–20 Hz to conserve I2C bus bandwidth.
- **Testing & validation:** Verify screen renders correctly without crashing if I2C bus is unpowered.
- **Acceptance criteria:** Displays corresponding hazard screen within 50 ms of state change.
- **GitHub tracking:** Issue #10.

---

## Phase 5: FreeRTOS Concurrency & Dual-Core Sensing Task

### Task 5.1 — Thread-Safe Cross-Core State Manager
- **Status:** Pending
- **Objective:** Implement `src/logic/state_manager.*` providing microsecond-level atomic read/write of `SystemState`.
- **Why this is necessary:** Core 0 (sensing) and Core 1 (networking/display) access shared state concurrently; race conditions cause corrupt telemetry.
- **Prerequisites:** Task 3.2.
- **Specification references:** `docs/architecture.md` §2.2; `docs/implementation.md` §3.
- **Implementation plan:**
  1. Define `portMUX_TYPE stateSpinlock = portMUX_INITIALIZER_UNLOCKED;`.
  2. Implement `StateManager::writeState(const SystemState &state)` using `taskENTER_CRITICAL()` / `taskEXIT_CRITICAL()`.
  3. Implement `StateManager::readState()` returning a copied snapshot.
- **Expected files:** `src/logic/state_manager.h`, `src/logic/state_manager.cpp` (Proposed).
- **Integration considerations:** Never use heavy FreeRTOS mutexes on Core 0.
- **Testing & validation:** Concurrency stress test verifying zero data tearing.
- **Acceptance criteria:** Snapshot reads and writes take $< 5\ \mu\text{s}$ with zero torn reads.
- **GitHub tracking:** Issue #11.

### Task 5.2 — FreeRTOS Core 0 Sensing Task & Main Loop Orchestration
- **Status:** Pending
- **Objective:** Configure `sensorTask` on Core 0 running at 50 Hz, and organize Core 1 `setup()`/`loop()` in `src/main.cpp`.
- **Why this is necessary:** Isolates the 20 ms safety-critical loop from Wi-Fi jitter and I2C display latency.
- **Prerequisites:** Tasks 2.1–4.3, 5.1.
- **Specification references:** `docs/flow.md` §14, §30; `docs/architecture.md` §2.1.
- **Implementation plan:**
  1. In `src/main.cpp`, create `SensorTask` using `xTaskCreatePinnedToCore` on Core 0.
  2. Regulate cadence with `vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20))`.
  3. Execute sensors -> fusion -> state machine -> indicators -> state_manager.
  4. Core 1 handles OLED refresh and web server event pumps.
- **Expected files:** `src/main.cpp`.
- **Integration considerations:** Stack size budgeted at 4096 bytes with stack high-water monitoring.
- **Testing & validation:** Measure loop execution jitter via GPIO toggle on oscilloscope/logic analyzer.
- **Acceptance criteria:** Core 0 task maintains 50 Hz ($\pm 1\text{ ms}$) cadence continuously.
- **GitHub tracking:** Issue #12.

---

## Phase 6: Wi-Fi SoftAP, Web Server & Embedded PROGMEM Dashboard

### Task 6.1 — Wi-Fi SoftAP Network Controller
- **Status:** Pending
- **Objective:** Implement `src/network/wifi_manager.*` establishing standalone AP (`BlindSpottr-AP`, `192.168.4.1`).
- **Why this is necessary:** Delivers the phone dashboard without requiring external routers or internet access.
- **Prerequisites:** Task 1.2.
- **Specification references:** `docs/flow.md` §16; `docs/design.md` §4.2.
- **Implementation plan:**
  1. Configure SoftAP with static IP `192.168.4.1`, subnet `255.255.255.0`.
  2. Launch AP with SSID and WPA2 passphrase from `config.h`.
  3. Provide client connection callback for diagnostic reporting.
- **Expected files:** `src/network/wifi_manager.h`, `src/network/wifi_manager.cpp` (Proposed).
- **Integration considerations:** Run exclusively on Core 1.
- **Testing & validation:** Connect mobile phone to `BlindSpottr-AP` and verify DHCP assignment of `192.168.4.x`.
- **Acceptance criteria:** SoftAP broadcasts beacon, authenticates clients, and assigns IP within 3 seconds.
- **GitHub tracking:** Issue #13.

### Task 6.2 — Responsive Automotive Phone Dashboard in PROGMEM
- **Status:** Pending
- **Objective:** Implement `src/network/dashboard_html.h` containing self-contained HTML5/CSS3/JavaScript web app.
- **Why this is necessary:** Eliminates SPIFFS/LittleFS filesystem dependencies, preventing web delivery bricking.
- **Prerequisites:** Task 0.2.
- **Specification references:** `docs/flow.md` §21, §22; `docs/design.md` §4.2.
- **Implementation plan:**
  1. Build dark-mode automotive cluster UI: top-down car model, left/right blind spot hazard zones, status badges.
  2. Implement WebSocket client in JS connecting to `ws://192.168.4.1/ws` with auto-reconnect.
  3. Update DOM dynamically upon receiving JSON telemetry.
  4. Store as raw string literal `const char DASHBOARD_HTML[] PROGMEM`.
- **Expected files:** `src/network/dashboard_html.h` (Proposed).
- **Integration considerations:** All SVGs, CSS, and JS inlined. Zero external CDN dependencies.
- **Testing & validation:** Load page in desktop and mobile Safari/Chrome browsers.
- **Acceptance criteria:** Dashboard renders cleanly on mobile screen and responds to simulated WebSocket packets.
- **GitHub tracking:** Issue #14.

### Task 6.3 — Async Web Server & WebSocket Push Telemetry Engine
- **Status:** Pending
- **Objective:** Implement `src/network/web_server.*` serving `/` and broadcasting real-time JSON over `/ws`.
- **Why this is necessary:** Delivers sub-50ms dashboard responsiveness without inefficient HTTP polling.
- **Prerequisites:** Tasks 5.1, 6.1, 6.2.
- **Specification references:** `docs/flow.md` §17, §18; `docs/architecture.md` §5.4.
- **Implementation plan:**
  1. Register route `GET /` serving `DASHBOARD_HTML` with gzip/caching headers.
  2. Register WebSocket handler at `/ws`.
  3. Implement `broadcastState(const SystemState &state)` formatting JSON:
     `{"state":1,"left":true,"right":false,"radarL":true,"radarR":false,"irL":false,"irR":false,"uptime":12450}`.
  4. Trigger push on FSM state change or 200 ms periodic tick.
- **Expected files:** `src/network/web_server.h`, `src/network/web_server.cpp` (Proposed).
- **Integration considerations:** Prune dead client sockets to prevent memory leaks.
- **Testing & validation:** Verify WebSocket frame arrival in browser DevTools network tab.
- **Acceptance criteria:** Dashboard updates within 30 ms of physical sensor trigger.
- **GitHub tracking:** Issue #15.

---

## Phase 7: System Integration & Hardware Bring-Up

### Task 7.1 — Bench Bring-Up, Calibration & FSM Integration
- **Status:** Pending
- **Objective:** Assemble physical bench circuit (sensors, LEDs, buzzer, OLED) and perform full integration testing.
- **Why this is necessary:** Validates electrical tolerances, RF radar interference, and IR threshold distances.
- **Prerequisites:** Phases 1–6 complete.
- **Specification references:** `docs/flow.md` §37, §38.
- **Implementation plan:**
  1. Wire physical hardware according to `docs/flow.md` pinout table.
  2. Calibrate IR sensor potentiometers to 10–15 cm detection range.
  3. Test scenario A: approaching motion -> radar triggers -> warning active.
  4. Test scenario B: vehicle stops -> IR engages -> warning remains active past 3s.
  5. Test scenario C: dual approach -> both warnings active.
- **Testing & validation:** Execute physical hardware test matrix.
- **Acceptance criteria:** All 3 scenarios pass reliably with zero system crashes or freezes.
- **GitHub tracking:** Issue #16.

### Task 7.2 — Power Profiling, Noise Immunity & Fail-Safe Hardening
- **Status:** Pending
- **Objective:** Validate decoupling capacitors, verify current draws, and stress-test fail-safes.
- **Why this is necessary:** RF emissions from radar or Wi-Fi bursts can cause false triggers or brownouts if unshielded.
- **Prerequisites:** Task 7.1.
- **Specification references:** `docs/design.md` §5, §6.
- **Implementation plan:**
  1. Test Wi-Fi burst impact on radar sensitivity. Add bypass caps if jitter occurs.
  2. Disconnect OLED and verify Core 0 LEDs and buzzer continue operating uninterrupted.
  3. Disconnect Wi-Fi client and verify heap memory remains stable.
- **Testing & validation:** 1-hour continuous burn-in test.
- **Acceptance criteria:** Zero brownouts, heap memory leak $< 100$ bytes after 1 hour of continuous WebSocket traffic.
- **GitHub tracking:** Issue #17.

---

## Phase 8: CI Automation & Release Preparation

### Task 8.1 — GitHub Actions PlatformIO Build Verification Workflow
- **Status:** Pending
- **Objective:** Implement `.github/workflows/build.yml` compiling firmware and running unit tests on every push and PR.
- **Why this is necessary:** Guarantees broken code is never merged into `dev` or `main`.
- **Prerequisites:** Phase 1 complete.
- **Specification references:** `AGENTS.md` §6.
- **Implementation plan:**
  1. Create workflow file running on `ubuntu-latest`.
  2. Install Python and PlatformIO Core.
  3. Run `pio run` and `pio test`.
- **Expected files:** `.github/workflows/build.yml` (Proposed).
- **Testing & validation:** Trigger workflow run on GitHub.
- **Acceptance criteria:** GitHub Actions build check passes with green checkmark.
- **GitHub tracking:** Issue #18.

### Task 8.2 — Final Demonstration Guide & Release Packaging
- **Status:** Pending
- **Objective:** Create comprehensive demonstration guide and prepare stable release tag on `main`.
- **Why this is necessary:** Enables seamless live presentation and handover for course grading.
- **Prerequisites:** All prior tasks complete.
- **Specification references:** `docs/flow.md` §40.
- **Implementation plan:**
  1. Write step-by-step model car demo script.
  2. Create release PR `dev -> main`.
  3. Tag release `v1.0.0`.
- **Expected files:** `docs/demo_guide.md` (Proposed).
- **Testing & validation:** Perform live dry-run following guide.
- **Acceptance criteria:** Fully verified release merged into `main` with passing CI.
- **GitHub tracking:** Issue #19.
