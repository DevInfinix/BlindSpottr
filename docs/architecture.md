# BlindSpottr — System Architecture Specification

> **AUTHORITY:** This document details the hardware and software architecture of the BlindSpottr system. For operational agent rules, see `AGENTS.md`. For authoritative technical requirements and pinouts, see `docs/flow.md`.

---

## 1. System Architecture Overview

BlindSpottr is an intelligent, dual-core embedded monitoring system implemented on an Espressif ESP32 microcontroller (`esp32doit-devkit-v1`). The architecture is split across hardware sensing, real-time sensor fusion, local alert actuation, and an isolated wireless telemetry pipeline.

```mermaid
graph TB
    subgraph Hardware_Inputs["Hardware Inputs"]
        RL["Left RCWL-0516 Radar (GPIO 32)"]
        RR["Right RCWL-0516 Radar (GPIO 33)"]
        IL["Left IR Sensor (GPIO 34)"]
        IR["Right IR Sensor (GPIO 35)"]
    end

    subgraph ESP32_Core0["ESP32 Core 0: Real-Time Sensing & Fusion (50 Hz)"]
        HAL["GPIO Hardware Abstraction"]
        FUSION["Sensor Fusion Engine\n(occupied = IR || Radar_Hold)"]
        FSM["Global 4-State Machine\n(SAFE / LEFT / RIGHT / BOTH)"]
        INDICATORS["Local Output Drivers\n(LEDs GPIO 25/26/27, Buzzer GPIO 14)"]
    end

    subgraph Thread_Safe_State["Thread-Safe Shared State"]
        STATE["SystemState Struct\n(Atomic / Critical Section Protected)"]
    end

    subgraph ESP32_Core1["ESP32 Core 1: Display & Connectivity Loop"]
        OLED["SSD1306 OLED Display Driver\n(I2C: GPIO 21/22 - 10-20 Hz)"]
        WIFI["Wi-Fi Access Point\n(SSID: BlindSpottr-AP)"]
        SERVER["AsyncWebServer & WebSockets\n(Port 80, /ws endpoint)"]
        HTML["Embedded PROGMEM Dashboard\n(Single-Page HTML/CSS/JS)"]
    end

    subgraph User_Interfaces["External Interfaces"]
        PHONE["Driver Mobile Phone Browser\n(http://192.168.4.1)"]
        PHYSICAL["Vehicle Cockpit / Dash\n(LED Cluster, Active Buzzer, OLED)"]
    end

    RL --> HAL
    RR --> HAL
    IL --> HAL
    IR --> HAL

    HAL --> FUSION
    FUSION --> FSM
    FSM --> INDICATORS
    FSM --> STATE

    STATE --> OLED
    STATE --> SERVER

    INDICATORS --> PHYSICAL
    OLED --> PHYSICAL
    WIFI --> PHONE
    SERVER <-->|WebSocket JSON Push| PHONE
    HTML -->|HTTP GET /| PHONE
```

---

## 2. Concurrency and Dual-Core Execution Model

The ESP32 dual-core Xtensa LX6 architecture is leveraged to physically separate high-priority, jitter-free real-time sensing from network-induced latency and non-deterministic I/O.

```mermaid
sequenceDiagram
    autonumber
    participant C0 as Core 0: sensorTask (20 ms Cadence)
    participant SS as Shared SystemState (portMUX Lock)
    participant C1 as Core 1: Arduino loop() & AsyncTCP
    participant WS as Connected WebSocket Client (Phone)

    loop Every 20 ms (50 Hz Cadence)
        C0->>C0: Sample GPIO 32, 33 (Radar) & 34, 35 (IR)
        C0->>C0: Update Radar Hold Timers (3000 ms window)
        C0->>C0: Evaluate Fusion: occupied = IR || hold_active
        C0->>C0: Evaluate FSM Transitions
        C0->>C0: Drive Red/Green LEDs & Alert Buzzer
        C0->>SS: Acquire spinlock, update snapshot, release spinlock
    end

    loop Every ~100 ms or on State Change
        C1->>SS: Acquire spinlock, copy snapshot, release spinlock
        C1->>C1: Refresh SSD1306 OLED via I2C (Wire)
        alt State Changed OR Heartbeat Timer Expired (200 ms)
            C1->>WS: Broadcast compact JSON packet via /ws
        end
    end
```

### 2.1 Core Allocation
- **Core 0 — Deterministic Real-Time Sensing (`sensorTask`):**
  - Priority: `tskIDLE_PRIORITY + 3` (high priority).
  - Pinned to Core 0 using `xTaskCreatePinnedToCore`.
  - Timing: Fixed 20 ms cadence (50 Hz) regulated by `vTaskDelayUntil()`.
  - Responsibilities: Digital pin reads, software radar hold-timer evaluation, sensor fusion logic, FSM state updates, and direct GPIO output control (LEDs, active buzzer).
- **Core 1 — Communication and Display (`loopTask`):**
  - Priority: Default Arduino `setup()` and `loop()`.
  - Responsibilities: Wi-Fi SoftAP stack, ESPAsyncWebServer request routing, AsyncTCP WebSocket broadcast handling, client connection life-cycle, and I2C OLED display refresh (10–20 Hz).

### 2.2 Cross-Core Thread Safety
Access to the shared `SystemState` structure must never block Core 0.
- Synchronization Mechanism: FreeRTOS `portMUX_TYPE` spinlock (`taskENTER_CRITICAL()` / `taskEXIT_CRITICAL()`) or C++11 `std::atomic` fields.
- Zero Mutexes: Heavy FreeRTOS mutexes (`xSemaphoreCreateMutex`) that can cause priority inversion or task suspension are strictly forbidden on the 20 ms sensing path.

---

## 3. Data Flow Architecture

The data pipeline progresses from raw physical sensor events to fused semantic states, then branches into cockpit alert outputs and remote telemetry.

```mermaid
flowchart LR
    subgraph Raw_Sensors["Raw Hardware Readings"]
        RL[Radar L]
        RR[Radar R]
        IL[IR L]
        IR[IR R]
    end

    subgraph Temporal_Filter["Temporal Processing"]
        TH_L["Left Radar Hold Window\n(now - t_last < 3000ms)"]
        TH_R["Right Radar Hold Window\n(now - t_last < 3000ms)"]
        POL_L["IR Polarity Normalizer\n(digitalRead == LOW)"]
        POL_R["IR Polarity Normalizer\n(digitalRead == LOW)"]
    end

    subgraph Fusion_Engine["Boolean Fusion"]
        F_L["Left Occupied = IR_L || Hold_L"]
        F_R["Right Occupied = IR_R || Hold_R"]
    end

    subgraph State_Resolution["State Resolution"]
        FSM{"Evaluate FSM\n(Both, Left, Right, Safe)"}
    end

    subgraph Actuation["Local Actuation"]
        LED["LED Indicators\n(Red L/R, Green Safe)"]
        BUZZ["Active Buzzer Cadence"]
        OLED["OLED Graphics Engine"]
    end

    subgraph Remote["Wireless Telemetry"]
        JSON["JSON Serializer"]
        WS["WebSocket Broadcast (/ws)"]
        UI["Phone Dashboard DOM"]
    end

    RL --> TH_L
    RR --> TH_R
    IL --> POL_L
    IR --> POL_R

    TH_L --> F_L
    POL_L --> F_L
    TH_R --> F_R
    POL_R --> F_R

    F_L --> FSM
    F_R --> FSM

    FSM --> LED
    FSM --> BUZZ
    FSM --> OLED
    FSM --> JSON
    JSON --> WS
    WS --> UI
```

---

## 4. Finite State Machine (FSM) Architecture

The system operates a deterministic 4-state Mealy/Moore hybrid state machine. The current state is resolved every 20 ms based on fused occupancy flags.

```mermaid
stateDiagram-v2
    [*] --> STATE_SAFE : System Boot & Init

    STATE_SAFE --> STATE_LEFT_WARNING : left_occupied == true && right_occupied == false
    STATE_SAFE --> STATE_RIGHT_WARNING : right_occupied == true && left_occupied == false
    STATE_SAFE --> STATE_BOTH_WARNING : left_occupied == true && right_occupied == true

    STATE_LEFT_WARNING --> STATE_SAFE : left_occupied == false
    STATE_LEFT_WARNING --> STATE_BOTH_WARNING : right_occupied == true

    STATE_RIGHT_WARNING --> STATE_SAFE : right_occupied == false
    STATE_RIGHT_WARNING --> STATE_BOTH_WARNING : left_occupied == true

    STATE_BOTH_WARNING --> STATE_LEFT_WARNING : right_occupied == false && left_occupied == true
    STATE_BOTH_WARNING --> STATE_RIGHT_WARNING : left_occupied == false && right_occupied == true
    STATE_BOTH_WARNING --> STATE_SAFE : left_occupied == false && right_occupied == false

    state STATE_SAFE {
        Green_LED_ON : Green LED ON (GPIO 27)
        Red_LEDs_OFF : Red LEDs OFF
        Buzzer_SILENT : Buzzer Silent
        OLED_CLEAR : OLED shows 'SYSTEM CLEAR'
    }

    state STATE_LEFT_WARNING {
        Left_Red_ACTIVE : Left Red LED ON/Blink (GPIO 25)
        Safe_LED_OFF_L : Green LED OFF
        Buzzer_PULSE_L : Buzzer Pulsed Alert
        OLED_HAZARD_L : OLED Left Hazard Arrow
    }

    state STATE_RIGHT_WARNING {
        Right_Red_ACTIVE : Right Red LED ON/Blink (GPIO 26)
        Safe_LED_OFF_R : Green LED OFF
        Buzzer_PULSE_R : Buzzer Pulsed Alert
        OLED_HAZARD_R : OLED Right Hazard Arrow
    }

    state STATE_BOTH_WARNING {
        Both_Red_ACTIVE : Both Red LEDs ON (GPIO 25 & 26)
        Safe_LED_OFF_B : Green LED OFF
        Buzzer_RAPID_B : Rapid High-Priority Buzzer Alert
        OLED_HAZARD_B : OLED Dual Hazard Warning
    }
```

---

## 5. Subsystem Component Architecture

### 5.1 Sensing Subsystem
- **Module Path:** `src/sensors/` (`radar.h`, `radar.cpp`, `ir.h`, `ir.cpp`).
- **Interfaces:** `void sensors_init()`, `void sensors_read(RawSensorData &out)`.
- **Hardware Decoupling:** Hardware pins and polarities are injected from `config.h`. No hardcoded GPIO accesses exist inside processing routines.

### 5.2 Fusion & State Machine Subsystem
- **Module Path:** `src/logic/` (`fusion.h`, `fusion.cpp`, `state_machine.h`, `state_machine.cpp`).
- **Interfaces:** `SystemState fusion_update(const RawSensorData &raw, uint32_t now)`.
- **Pure Logic:** Contains no direct hardware GPIO or display calls, enabling 100% unit-testability on host platforms or Unity simulators.

### 5.3 Local Actuation Subsystem
- **Module Path:** `src/outputs/` (`indicators.h`, `indicators.cpp`, `buzzer.h`, `buzzer.cpp`, `display.h`, `display.cpp`).
- **LED/Buzzer Driver:** Evaluates current state and manages non-blocking blinking/pulsing cadences using elapsed time checks.
- **OLED Display Driver:** Formats hazard icons, radar/IR activity badges, and system uptime into SSD1306 buffer over 400 kHz I2C.

### 5.4 Wireless & Web Subsystem
- **Module Path:** `src/network/` (`wifi_manager.h`, `wifi_manager.cpp`, `web_server.h`, `web_server.cpp`, `dashboard_html.h`).
- **SoftAP Configuration:** IP `192.168.4.1`, Subnet `255.255.255.0`, SSID `BlindSpottr-AP`, WPA2 Passphrase configured in `config.h`.
- **Embedded PROGMEM Dashboard:** Complete HTML5/CSS3/JavaScript web application stored as an escaped `const char DASHBOARD_HTML[] PROGMEM` string inside `dashboard_html.h`. Zero external filesystem partitions (SPIFFS/LittleFS) required.
- **WebSocket Protocol:** Endpoint `/ws`. Dispatches telemetry as JSON strings:
  ```json
  {"state":1,"left":true,"right":false,"radarL":true,"radarR":false,"irL":false,"irR":false,"uptime":12450}
  ```
