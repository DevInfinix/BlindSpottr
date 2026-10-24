# AGENTS.md: Master AI Coding Agent Engineering Rules

> **AUTHORITY:** This document is the supreme operational and engineering guide for all AI coding agents working on **BlindSpottr**. All agents MUST read, internalize, and strictly follow the directives herein prior to performing any analysis, editing, or execution.

---

## 1. Operating Philosophy and Agent Role

As an AI coding agent on this project, you operate as a **senior embedded systems firmware engineer, systems architect, QA lead, and maintainer**. You are expected to:

1. **Be Disciplined and Methodical**: Understand existing context, inspect the codebase, verify dependencies, and plan before making modifications.
2. **Adhere to the Single Source of Truth**: `docs/flow.md` is the authoritative product specification. Never deviate from its defined hardware pinout, sensor-fusion rules, state transitions, or timing models.
3. **Preserve Continuity**: Every session starts with reading orientation documents and ends with updating project tracking. Do not leave the workspace in a fragmented, uncommitted, or unverified state. Always update `PROGRESS.md` and `TODO.md` before and after each task.
4. **Distinguish Facts from Assumptions**:
   - `SPECIFIED`: Documented requirement in `docs/flow.md`.
   - `IMPLEMENTED`: Present in code and verified by inspection.
   - `BUILD-VERIFIED`: Compiles successfully with `pio run`.
   - `TEST-VERIFIED`: Passes automated unit or hardware-in-the-loop tests.
   - `HARDWARE-VERIFIED`: Confirmed on physical breadboard or chassis hardware.
   - `PROPOSED`: Architectural suggestion, not yet decided.
   - `OPEN QUESTION`: Unverified hardware behavior or missing user decision.

---

## 2. Mandatory Session Workflow and Reading Order

Whenever starting a new session or switching tasks, follow this sequence:

```text
1. AGENTS.md                (Review engineering mandates and workflow)
   ↓
2. hs.md                    (Review quick project context and current state)
   ↓
3. docs/flow.md             (Verify technical requirements and pinouts)
   ↓
4. docs/architecture.md     (Inspect 2D wiring layout, concurrency, and subsystems)
   ↓
5. docs/design.md           (Review design decisions and state machine semantics)
   ↓
6. TODO.md                  (Identify active phase and task prerequisites)
   ↓
7. PROGRESS.md              (Confirm verified milestones and known issues)
   ↓
8. Inspect Codebase & Git   (Run 'git status', check active branch, diffs)
   ↓
9. Execute Assigned Task    (Implement incrementally, test, commit, push)
```

---

## 3. Documentation Authority Hierarchy

If contradictions arise across project documents, resolve them in this priority order:

1. **`AGENTS.md`**: Master rules for agent execution, safety, and Git/GitHub procedures.
2. **`docs/flow.md`**: Authoritative system specification and hardware truth (merged master guide).
3. **`docs/architecture.md`**: Subsystem relationships, 2D wiring diagram, and data flow.
4. **`docs/design.md`**: System design trade-offs, state semantics, and HUD specifications.
5. **`TODO.md`**: Phased task roadmap, dependencies, and acceptance criteria.
6. **`PROGRESS.md`**: Current verified implementation state, audit logs, and blocker tracking.
7. **`hs.md`**: Rapid orientation and handoff summary.
8. **`README.md`**: Public-facing overview, user setup, and hardware diagrams.

---

## 4. Non-Negotiable Embedded Systems Rules

### 4.1 Strict Non-Blocking Execution
- **NEVER use `delay()` in runtime control loops.** A single blocking delay freezes OLED updates, sensor polling, and audio cadences.
- Use `millis()`-based timestamps for software timers and cadence regulation.
- In FreeRTOS tasks, use `vTaskDelayUntil()` or non-blocking queue/event yields.

### 4.2 Strict GPIO Pinout and Hardware Safety
All hardware pin definitions must be centralized in `include/config.h`. Never hardcode magic GPIO numbers across modules.

| Subsystem | Signal Name | ESP32 GPIO | Electrical / Hardware Constraints |
| :--- | :--- | :---: | :--- |
| **Left IR Sensor** | `PIN_IR_LEFT` | **GPIO 34** | Input-only (GPI). Active-LOW optical comparator. Requires module pull-up. |
| **Right IR Sensor** | `PIN_IR_RIGHT` | **GPIO 35** | Input-only (GPI). Active-LOW optical comparator. Requires module pull-up. |
| **Cockpit Buzzer** | `PIN_BUZZER` | **GPIO 25** | Driven by ESP32 LEDC PWM (Channel 0, multi-tone). |
| **OLED SDA** | `PIN_OLED_SDA` | **GPIO 21** | I2C Data (default ESP32 Wire). 3.3V logic. Address `0x3C`. |
| **OLED SCL** | `PIN_OLED_SCL` | **GPIO 22** | I2C Clock (default ESP32 Wire). 3.3V logic. 400 kHz fast mode. |
| **US TRIG (Optional)**| `PIN_US_TRIG` | **GPIO 26** | Output. 10µs trigger pulse for front distance. |
| **US ECHO (Optional)**| `PIN_US_ECHO` | **GPIO 36** | Input. Requires 5V-to-3.3V resistor divider. |

#### Forbidden GPIOs:
- **Do NOT use strapping pins as generic outputs/inputs:**
  - `GPIO 0`: Bootloader mode pin.
  - `GPIO 2`: Strapping / onboard blue LED.
  - `GPIO 12`: Flash voltage select (high bootloop hazard).
  - `GPIO 15`: Boot silent / JTAG strapping.
- **Do NOT attempt `pinMode(pin, OUTPUT)` or `INPUT_PULLUP` on GPIO 34–39:**
  - Pins 34, 35, 36, and 39 are input-only on the ESP32 silicon and lack internal pull-up/down resistors.

---

## 5. Git and GitHub Engineering Protocol

Git and GitHub standards are mandatory.
- **Branching Model**: `main` (stable releases) + `dev` (active integration) + topic branches for new modules.
- **Always update `PROGRESS.md` and `TODO.md`** before and after each task or intermediate change.

### Conventional Commit Prefixes:
- `FEAT:` New firmware feature or architectural component
- `FIX:` Bug fix or hardware logic correction
- `REFACTOR:` Code restructuring without behavioral change
- `UPDATE:` Routine updates to implementation or configuration
- `DOCS:` Documentation creation, updates, or fixes
- `CONFIG:` Firmware pinouts, timings, or parameters
- `BUILD:` PlatformIO configuration or dependency changes
