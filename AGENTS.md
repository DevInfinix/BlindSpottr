# AGENTS.md — Master AI Coding Agent Engineering Rules

> **AUTHORITY:** This document is the supreme operational and engineering guide for all AI coding agents working on **BlindSpottr**. All agents MUST read, internalize, and strictly follow the directives herein prior to performing any analysis, editing, or execution.

---

## 1. Operating Philosophy & Agent Role

As an AI coding agent on this project, you operate as a **senior embedded systems firmware engineer, systems architect, QA lead, and maintainer**. You are expected to:

1. **Be Disciplined & Methodical:** Never jump directly into generating code. Understand existing context, inspect the codebase, verify dependencies, and plan before making modifications.
2. **Adhere to the Single Source of Truth:** `docs/flow.md` is the authoritative product specification. Never deviate from its defined hardware pinout, sensor-fusion rules, state transitions, or timing models.
3. **Preserve Continuity:** Every session starts with reading orientation documents and ends with updating project tracking. Do not leave the workspace in a fragmented, uncommitted, or unverified state.
4. **Distinguish Facts from Assumptions:**
   - `SPECIFIED`: Documented requirement in `docs/flow.md`.
   - `IMPLEMENTED`: Present in code and verified by inspection.
   - `BUILD-VERIFIED`: Compiles successfully with `pio run`.
   - `TEST-VERIFIED`: Passes automated unit/hardware-in-the-loop tests.
   - `HARDWARE-VERIFIED`: Confirmed on physical breadboard/chassis hardware.
   - `PROPOSED`: Architectural suggestion, not yet decided.
   - `OPEN QUESTION`: Unverified hardware behavior or missing user decision.

---

## 2. Mandatory Session Workflow & Reading Order

Whenever starting a new session or switching tasks, follow this strict sequence:

```text
1. AGENTS.md                (Review engineering mandates and workflow)
   ↓
2. hs.md                    (Review quick project context and current state)
   ↓
3. docs/flow.md             (Verify technical requirements and pinouts)
   ↓
4. docs/architecture.md     (Inspect concurrency, data flow, and subsystems)
   ↓
5. docs/design.md           (Review design decisions and state machine semantics)
   ↓
6. docs/implementation.md   (Review module boundaries and implementation rules)
   ↓
7. TODO.md                  (Identify active phase and task prerequisites)
   ↓
8. PROGRESS.md              (Confirm verified milestones and known issues)
   ↓
9. Inspect Codebase & Git   (Run 'git status', check active branch, diffs)
   ↓
10. Execute Assigned Task   (Implement incrementally, test, commit, push)
```

---

## 3. Documentation Authority Hierarchy

If contradictions arise across project documents, resolve them in this priority order:

1. **`AGENTS.md`**: Master rules for agent execution, safety, and Git/GitHub procedures.
2. **`docs/flow.md`**: Authoritative system specification and hardware truth.
3. **`docs/architecture.md`**: System subsystem relationships, data flow, and concurrency models.
4. **`docs/design.md`**: System design trade-offs, state semantics, and UI specifications.
5. **`docs/implementation.md`**: Implementation strategy, module APIs, and coding rules.
6. **`TODO.md`**: Phased task roadmap, dependencies, and acceptance criteria.
7. **`PROGRESS.md`**: Current verified implementation state, audit logs, and blocker tracking.
8. **`hs.md`**: Rapid orientation and handoff summary.
9. **`README.md`**: Public-facing overview, user setup, and hardware diagrams.

*Never silently alter a specification in `docs/flow.md` to fit an implementation convenience. If an unresolved ambiguity exists, document it in `PROGRESS.md` and consult the project lead.*

---

## 4. Mandatory Technical Skills Policy

The following skills are installed on this system (`~/.agents/skills/`, symlinked to `~/.config/kilo/skills/` and `~/.kilo/skills/`) and must be consulted when working in their respective domains:

1. **`platformio`**: Project setup, `platformio.ini` environment configuration, dependency resolution (`lib_deps`), build flags, and build/upload commands.
2. **`esp32`**: ESP32 hardware architecture, GPIO pinout rules, strapping pin hazards, input-only pin constraints, I2C bus management, and MMU/flash safety.
3. **`non-blocking-patterns`**: Finite state machines (FSM), timer loops (`millis()`), asynchronous event handling, and eliminating blocking calls.
4. **`embedded-systems`**: Peripheral drivers, electrical boundaries, noise filtering, debounce algorithms, and resource-constrained C/C++ architecture.
5. **`esp32-firmware-engineer`**: Dual-core task pinning, FreeRTOS task prioritization, cross-core synchronization, memory leak prevention, and crash dump analysis.
6. **`esp-idf`**: Low-level ESP32 peripheral subsystems, FreeRTOS primitives, and hardware timer/interrupt mechanisms when Arduino wrappers require lower-level clarification.

*Note: While ESP-IDF skills are available, the project targets the **Arduino framework under PlatformIO** on `esp32doit-devkit-v1`. Do not migrate the codebase away from the Arduino framework without explicit authorization.*

---

## 5. Non-Negotiable Embedded Systems Rules

### 5.1 Strict Non-Blocking Execution
- **NEVER use `delay()` in runtime control loops.** A single blocking delay freezes OLED updates, sensor polling, radar hold timers, and WebSocket frames.
- Use `millis()`-based timestamps for software timers and cadence regulation.
- In FreeRTOS tasks, use `vTaskDelayUntil()` or non-blocking queue/event yields.

### 5.2 Strict GPIO Pinout and Hardware Safety
All hardware pin definitions must be centralized in `include/config.h` (or `src/config.h`). Never hardcode magic GPIO numbers across modules.

| Subsystem | Signal Name | ESP32 GPIO | Electrical / Hardware Constraints |
| :--- | :--- | :--- | :--- |
| **Radar Left** | `PIN_RADAR_L` | **GPIO 32** | Digital input. Active-HIGH Doppler pulse. |
| **Radar Right** | `PIN_RADAR_R` | **GPIO 33** | Digital input. Active-HIGH Doppler pulse. |
| **IR Left** | `PIN_IR_L` | **GPIO 34** | **Input-only pin (GPI). No internal pull-up/pull-down.** Requires module external pull-up. |
| **IR Right** | `PIN_IR_R` | **GPIO 35** | **Input-only pin (GPI). No internal pull-up/pull-down.** Requires module external pull-up. |
| **LED Left (Red)** | `PIN_LED_L` | **GPIO 25** | Digital output. Push-pull active-HIGH. Current limit resistor required. |
| **LED Right (Red)**| `PIN_LED_R` | **GPIO 26** | Digital output. Push-pull active-HIGH. Current limit resistor required. |
| **LED Safe (Green)**| `PIN_LED_SAFE`| **GPIO 27** | Digital output. Push-pull active-HIGH. Lit when state is `SAFE`. |
| **Active Buzzer** | `PIN_BUZZER` | **GPIO 14** | Digital output. Active-HIGH (transistor/direct driven). |
| **OLED SDA** | `PIN_I2C_SDA` | **GPIO 21** | I2C Data (default ESP32 wire). 3.3V logic. Address `0x3C`. |
| **OLED SCL** | `PIN_I2C_SCL` | **GPIO 22** | I2C Clock (default ESP32 wire). 3.3V logic. |

#### Forbidden GPIOs:
- **Do NOT use strapping pins as generic outputs/inputs:**
  - `GPIO 0`: Bootloader mode pin.
  - `GPIO 2`: Strapping / onboard LED.
  - `GPIO 12`: Flash voltage select (strapping hazard — high risk of bootloop).
  - `GPIO 15`: Boot silent / JTAG strapping.
- **Do NOT attempt `pinMode(pin, OUTPUT)` or `INPUT_PULLUP` on GPIO 34–39:**
  - Pins 34, 35, 36, 39 are input-only on the ESP32 silicon and lack internal pull-up/down resistors.

### 5.3 Sensor Polarity & Configurable Abstraction
- **Radar Sensors:** Active-HIGH pulse on motion detection (`HIGH = motion`).
- **IR Sensors:** Common obstacle modules are active-LOW (`LOW = obstacle present`). Define `IR_DETECT_LEVEL` in `config.h` (default `LOW`) and support software inversion if hardware batches vary.
- **Sensor Fusion:**
  $$\text{occupied} = \text{IR\_detected} \lor \text{radar\_hold\_active}$$
  $$\text{radar\_hold\_active} = (\text{now} - \text{last\_radar\_trigger\_ms}) < \text{RADAR\_HOLD\_MS}\ (3000\text{ ms})$$

### 5.4 Deterministic Global State Machine
The system has exactly four authoritative states:
1. `STATE_SAFE` (0): No hazard on either side. Green LED ON, Red LEDs OFF, Buzzer OFF, OLED shows "CLEAR".
2. `STATE_LEFT_WARNING` (1): Left blind-spot hazard active. Red Left LED blinking/ON, Green LED OFF, Buzzer pulsed, OLED shows left arrow hazard.
3. `STATE_RIGHT_WARNING` (2): Right blind-spot hazard active. Red Right LED blinking/ON, Green LED OFF, Buzzer pulsed, OLED shows right arrow hazard.
4. `STATE_BOTH_WARNING` (3): Simultaneous hazards on both sides. High-priority state. Both Red LEDs active, Green LED OFF, rapid buzzer alert, OLED warns both sides.

### 5.5 Dual-Core Architecture & Shared State Thread Safety
- **Core 0 (Sensing & Fusion):** Dedicated FreeRTOS task running at a deterministic 20 ms cadence (50 Hz). Reads GPIOs, calculates hold windows, resolves FSM state, and updates local indicators.
- **Core 1 (Connectivity & Display):** Default Arduino core running `setup()` and `loop()`. Manages Wi-Fi SoftAP (`BlindSpottr-AP`), ESPAsyncWebServer, WebSocket broadcasts (`/ws`), and periodic OLED refresh.
- **Shared State (`SystemState`):** Cross-core access must be lightweight and safe. Use FreeRTOS `portMUX_TYPE` spinlocks or C++ `std::atomic` fields. Never use heavy mutexes that can block the 20 ms real-time sensing loop.

### 5.6 Web Dashboard Architecture
- Deliver the phone dashboard directly from the ESP32 SoftAP without requiring an external internet connection or local router.
- Use **Embedded PROGMEM HTML/CSS/JS** stored in a single header (`src/network/dashboard_html.h`) to guarantee zero LittleFS/SPIFFS flash dependencies and prevent filesystem corruption.
- Real-time updates must be pushed over WebSockets (`/ws`) as compact JSON packets upon state changes or a 200 ms periodic telemetry tick.

---

## 6. Git & GitHub Engineering Protocol

Git and GitHub standards are mandatory. The user has explicitly selected:
- **Branching Model:** `main` (stable releases) + `dev` (active integration) + topic/fix branches for breaking changes, paired with GitHub Pull Requests and Issues.

```text
       ┌─────────── feature / fix branch ───────────┐
       ↓                                            ↓ (Pull Request)
[main] ────────────────────────── [dev] ─────────────────────────── [main] (Release PR)
```

### 6.1 Branching Rules
1. `main`: Production/release-ready branch. Never commit experimental or unverified code directly to `main`.
2. `dev`: Primary integration branch. All feature and documentation work merges here via validated Pull Requests.
3. Feature / Task Branches: Branch from `dev` using descriptive names:
   - `docs/<feature>` (e.g., `docs/architecture-foundation`)
   - `feat/<module>` (e.g., `feat/sensor-abstraction`)
   - `fix/<bug>` (e.g., `fix/ir-polarity-config`)
   - `test/<suite>` (e.g., `test/fsm-unit-tests`)

### 6.2 Commit Discipline & Conventional Prefixes
Create frequent, small, well-scoped commits for every logical unit of work. Every commit message must use one of the approved conventional prefixes:

| Prefix | Description | Example |
| :--- | :--- | :--- |
| `FEAT:` | New firmware or architectural feature | `FEAT: add non-blocking radar hold timer module` |
| `FIX:` | Bug fix or hardware logic correction | `FIX: invert default IR sensor detection polarity` |
| `REFACTOR:` | Code restructuring without behavioral change | `REFACTOR: decouple OLED rendering from sensor task` |
| `UPDATE:` | Routine updates to implementation or config | `UPDATE: tune buzzer alert cadence to 100ms pulse` |
| `DOCS:` | Documentation creation, updates, or fixes | `DOCS: establish master AGENTS.md engineering rules` |
| `README:` | Edits specifically targeting root README.md | `README: add hardware pinout table and system diagrams` |
| `TEST:` | Adding or modifying automated unit/mock tests | `TEST: add unity test suite for sensor fusion logic` |
| `BUILD:` | PlatformIO configuration or dependency changes | `BUILD: add ESPAsyncWebServer-esphome to lib_deps` |
| `CONFIG:` | Firmware pinouts, timings, or parameters | `CONFIG: set RADAR_HOLD_MS to 3000 in config.h` |
| `CHORE:` | Repository maintenance, ignore files, formatting | `CHORE: add .pio and editor artifacts to .gitignore` |
| `CI:` | GitHub Actions workflow files | `CI: add platformio compile verification workflow` |

### 6.3 Verification Before Commit
Before committing any changes:
1. Review the git diff (`git diff`) to confirm only intended files are staged.
2. If configuration (`platformio.ini`) or code (`src/`, `include/`, `lib/`) was altered, run a build verification:
   ```bash
   /opt/homebrew/bin/pio run
   ```
3. Ensure no secrets, tokens, or untracked temporary files are committed.
4. Update `PROGRESS.md` and `TODO.md` to reflect the exact state.

### 6.4 Push and Synchronization
- Push validated commits to the remote tracking branch immediately after committing.
- Confirm successful push with `git status` and remote check.
- Never force-push (`git push -f`) to shared integration branches (`main`, `dev`).

---

## 7. Change Handoff & Session Completion Standard

At the end of every agent turn:
1. Verify all modified files are saved, coherent, and accounted for.
2. Ensure `PROGRESS.md` reflects the exact verified state and next action.
3. Ensure `TODO.md` has the active task updated (`in_progress` or `completed`).
4. Provide a clear, technical summary following Section 24 of the Master Initialization Directive.
5. Never end a session by implementing code outside the authorized phase. Respect the phase boundaries.
