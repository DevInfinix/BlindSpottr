<div align="center">

![BlindSpottr Banner](docs/demo/blindspottr-github-banner.png)

# BlindSpottr
### Autonomous Blind-Spot Monitoring & Headway Perception Platform

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Core%206.2.0-orange.svg?style=flat-square&logo=platformio)](https://platformio.org/)
[![Target MCU](https://img.shields.io/badge/MCU-ESP32%20DevKit%20V1-red.svg?style=flat-square&logo=espressif)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20C%2B%2B-00979D.svg?style=flat-square&logo=arduino)](https://github.com/espressif/arduino-esp32)
[![Web Telemetry](https://img.shields.io/badge/HUD-Tesla%20Cockpit%20View-18181b.svg?style=flat-square)](http://192.168.4.1)
[![OLED Display](https://img.shields.io/badge/Cockpit%20Display-SSD1306%20128x64%20I2C-brightgreen.svg?style=flat-square)](https://www.adafruit.com/)
[![Acoustics](https://img.shields.io/badge/Acoustics-LEDC%20Multi--Tone%20PWM-yellow.svg?style=flat-square)]()
[![Release](https://img.shields.io/badge/Release-v1.0.0-blue.svg?style=flat-square)]()
[![License](https://img.shields.io/badge/License-MIT-gray.svg?style=flat-square)](LICENSE)

<p align="center">
  A real-time blind-spot monitoring, headway perception, and Tesla-style HUD platform for scale-model trucks using an ESP32, dual optical presence sensors, an HC-SR04 ultrasonic distance sensor, an SSD1306 OLED HUD, and a browser-accessible WebGL cockpit.
</p>

</div>

---

## Visual Demonstration & Hardware Prototype

| Hardware Assembly & Cockpit Integration | Operational HUD & Bench Testing |
| :---: | :---: |
| ![Hardware Top Angle](docs/demo/blindspottr-hw-1.png)<br/><sub><b>Hardware Prototype: Scale Model Vehicle with ESP32 & Sensors</b></sub> | ![Live Web HUD Active](docs/demo/blindspottr-web-1.png)<br/><sub><b>Tesla Cockpit HUD: Live Perception & Speedometer</b></sub> |
| ![Hardware Front Elevation](docs/demo/blindspottr-hw-2.png)<br/><sub><b>Chassis Detail: Front Ultrasonic Sensor & OLED Display</b></sub> | ![Web Settings Modal](docs/demo/blindspottr-web-2.png)<br/><sub><b>In-Browser Config: Wi-Fi Hotspot & Safety Calibration</b></sub> |
| ![Hardware Side Profile](docs/demo/blindspottr-hw-3.png)<br/><sub><b>Flank Optical Presence Sensor & Power Distribution</b></sub> | ![Hardware Rear Deck](docs/demo/blindspottr-hw-4.png)<br/><sub><b>Rear Deck & Soldered Header Circuit Integration</b></sub> |

---

## Table of contents

- [Overview](#overview)
- [Key architectural highlights](#key-architectural-highlights)
- [Hardware schematic and wiring](#hardware-schematic-and-wiring)
- [State machine and threat arbitration](#state-machine-and-threat-arbitration)
- [Cockpit OLED HUD](#cockpit-oled-hud)
- [Tesla-style mobile WebGL perception HUD](#tesla-style-mobile-webgl-perception-hud)
- [Audio alert matrix](#audio-alert-matrix)
- [Building and flashing](#building-and-flashing)
- [Connecting your phone or laptop](#connecting-your-phone-or-laptop)
- [Project structure](#project-structure)
- [License](#license)

---

## Overview

Side mirrors leave blind spots where overtaking vehicles disappear from a driver's peripheral view. In scale models and robotics, simple obstacle detection scripts often rely on `delay()`, which halts execution, starves sensor reads, and causes display stutter.

BlindSpottr uses a 100 Hz non-blocking loop running on an ESP32. It samples two infrared distance sensors and a front ultrasonic sensor, updates an SSD1306 OLED display with animated top-down telemetry, and streams live telemetry at 20 Hz over an asynchronous WebSocket server to an interactive Tesla-style perception dashboard accessible on any mobile browser.

```mermaid
graph LR
    subgraph PERCEPTION ["Perception Sensors"]
        IR_L["Left Optical Sensor\n(GPIO 35)"]
        IR_R["Right Optical Sensor\n(GPIO 32)"]
        US["Front Ultrasonic Sensor\n(TRIG: 26, ECHO: 36)"]
    end

    subgraph BRAIN ["ESP32 Dual-Core Engine"]
        FSM{"Deterministic Threat FSM\n100 Hz Non-Blocking"}
        WS["Async WebSocket Server\n(ws://192.168.4.1/ws)"]
    end

    subgraph ACTUATION ["Cockpit & Remote Alerts"]
        OLED["SSD1306 128x64 OLED\nTop-Down Vehicle Wireframe"]
        BUZZ["LEDC Multi-Tone Buzzer\n2200 Hz Left / 2900 Hz Right"]
        WEB["Tesla-Style Web HUD\n3D Perspective Perception"]
    end

    IR_L --> FSM
    IR_R --> FSM
    US --> FSM

    FSM --> OLED
    FSM --> BUZZ
    FSM --> WS
    WS --> WEB
```

---

## Key architectural highlights

- Non-blocking loop: The firmware uses timestamp arithmetic with `millis()` rather than `delay()`. Sensor polling, display refreshes, and buzzer cadences run independently.
- Acoustic pitch separation: Left and right warnings use distinct sound frequencies (2200 Hz vs 2900 Hz). The buzzer alerts only on blind spot presence and remains silent for front headway distance.
- Tesla-style perception dashboard: Served directly from ESP32 flash memory (`http://192.168.4.1/`), rendering an animated 3D truck, perspective road lines, dynamic front obstacle proxy, and volumetric blind-spot radar arcs with zero external internet dependencies.
- In-browser settings modal: Easily configure personal phone hotspot credentials via a web form saved to ESP32 non-volatile storage (NVS) without modifying or re-flashing code.
- Acoustic ringdown rejection: Ultrasonic measurements discard false transducer reflections below 400 microseconds (< 7 cm) and apply a 3-sample median filter to eliminate erratic readings.

---

## Hardware schematic and wiring

```mermaid
flowchart TD
    ESP["ESP32 DevKit V1 (38-Pin)"]

    IR_L["Left IR Sensor (LM393)"]
    IR_R["Right IR Sensor (LM393)"]
    US["HC-SR04 Ultrasonic Sensor"]
    OLED["SSD1306 OLED Display (128x64)"]
    BUZZ["Cockpit Piezo Buzzer"]

    IR_L -->|DO -> GPIO 35| ESP
    IR_R -->|DO -> GPIO 32| ESP
    ESP -->|GPIO 26 -> TRIG| US
    US -->|ECHO -> GPIO 36 VP| ESP
    ESP -->|GPIO 21 -> SDA| OLED
    ESP -->|GPIO 22 -> SCL| OLED
    ESP -->|GPIO 25 PWM -> SIG| BUZZ

    ESP -.->|3.3V Logic Rail| IR_L
    ESP -.->|3.3V Logic Rail| IR_R
    ESP -.->|3.3V Logic Rail| OLED
    ESP -.->|3.3V Logic Rail| BUZZ
    ESP -.->|VIN 5V Supply Rail| US
    ESP -.->|Common Ground GND| IR_L
    ESP -.->|Common Ground GND| IR_R
    ESP -.->|Common Ground GND| US
    ESP -.->|Common Ground GND| OLED
    ESP -.->|Common Ground GND| BUZZ
```

| Subsystem | Signal | ESP32 GPIO | Pin type | Wiring note |
| :--- | :--- | :---: | :---: | :--- |
| Left IR sensor | `PIN_IR_LEFT` | GPIO 35 | Input | Input-only (GPI). Active-low optical comparator |
| Right IR sensor | `PIN_IR_RIGHT` | GPIO 32 | Input | Digital input with pull-up. Active-low optical comparator |
| Ultrasonic trigger | `PIN_US_TRIG` | GPIO 26 | Output | 10 microsecond trigger pulse |
| Ultrasonic echo | `PIN_US_ECHO` | GPIO 36 (VP) | Input | Ultrasonic echo pulse input |
| Ultrasonic VCC | VCC | VIN | Power | 5V DC supply from USB bus |
| Buzzer | `PIN_BUZZER` | GPIO 25 | Output | Driven by LEDC PWM, 8-bit duty resolution |
| OLED SDA | `PIN_OLED_SDA` | GPIO 21 | I2C Data | 3.3V logic, standard hardware Wire |
| OLED SCL | `PIN_OLED_SCL` | GPIO 22 | I2C Clock | 3.3V logic, 400 kHz fast mode |

---

## State machine and threat arbitration

```mermaid
stateDiagram-v2
    [*] --> SAFE_CLEAR

    SAFE_CLEAR --> WARN_LEFT : Left IR active
    SAFE_CLEAR --> WARN_RIGHT : Right IR active
    SAFE_CLEAR --> WARN_BOTH : Both IR active

    WARN_LEFT --> SAFE_CLEAR : Left IR clears
    WARN_LEFT --> WARN_BOTH : Right IR active

    WARN_RIGHT --> SAFE_CLEAR : Right IR clears
    WARN_RIGHT --> WARN_BOTH : Left IR active

    WARN_BOTH --> WARN_LEFT : Right IR clears
    WARN_BOTH --> WARN_RIGHT : Left IR clears
    WARN_BOTH --> SAFE_CLEAR : Both IR clear
```

---

## Cockpit OLED HUD

```mermaid
graph TD
    subgraph SCREEN ["SSD1306 128x64 Monochrome HUD Layout"]
        TOP["Top Header: BLINDSPOTTR with Animated Chevrons"]
        MID["Center Graphic: Top-Down Truck + Live Headway cm"]
        SIDES["Flank Zones: Dynamic Left & Right Radar Arcs"]
        BOT["Bottom Status Banner: ROAD CLEAR / HAZARD DETECTED"]
    end
    TOP --- MID
    MID --- SIDES
    SIDES --- BOT
```

---

## Tesla-style mobile WebGL perception HUD

When opening the dashboard in a browser, a high-performance 3D perception canvas provides full situational awareness:
- **Central Vehicle**: 3D commercial truck with headlights, cargo bed, and suspension oscillation.
- **Dynamic 3D Obstacle Proxy**: Visualized vehicle model that moves closer or farther matching measured headway in centimeters.
- **Volumetric Radar Rings**: Red pulsing arcs ignite on the left and right mirrors when blind spots are obstructed.
- **Speed & Gear Cluster**: Digital drive gear pills (D/N/R/P) and dynamic velocity indicator.
- **Interactive Settings Modal**: Adjust warning distance thresholds and configure phone hotspot credentials on the fly.

---

## Audio alert patterns

The buzzer connects to GPIO 25 and runs through ESP32 hardware PWM (LEDC channel 0). It alerts solely on blind-spot obstacles:

| State | Frequency | Cadence | Tone description |
| :--- | :--- | :--- | :--- |
| Clear | Silent | Off | No output |
| Left obstacle | 2200 Hz | 100 ms on, 120 ms off | Steady mid-pitch pulse |
| Right obstacle | 2900 Hz | 100 ms on, 120 ms off | High-pitch pulse |
| Both sides blocked | 3400 Hz and 2400 Hz | 50 ms per step | Rapid alternating chirp |

---

## Building and flashing

### 1. Build and Upload
```bash
pio run --target upload
```

### 2. Monitor Serial Output
```bash
pio device monitor --baud 115200
```
The terminal displays the Wi-Fi network credentials and IP address without spamming telemetry lines.

---

## Connecting your phone or laptop

1. Connect your device to the truck's access point:
   - **SSID**: `BlindSpottr-AP`
   - **Password**: `blindspottr123`
2. Open Safari or Chrome and navigate to:
   👉 **`http://192.168.4.1`** (or `http://blindspottr.local` on supported networks)
3. To connect the truck to your personal phone hotspot instead:
   - Tap **SETTINGS** on the web HUD.
   - Enter your hotspot SSID and password, then tap **Save & Connect Truck**.

---

## Project structure

```text
BlindSpottr/
├── platformio.ini         # PlatformIO build configuration and libraries
├── include/
│   └── config.h           # Central pinout, timing, and network configuration
├── src/
│   ├── main.cpp           # Main loop and state machine resolution
│   ├── sensors.h / .cpp   # Non-blocking IR and ultrasonic sensor acquisition
│   ├── display.h / .cpp   # SSD1306 OLED HUD graphics and animation
│   ├── buzzer.h / .cpp    # Multi-frequency buzzer driver using LEDC PWM
│   ├── telemetry_server.h / .cpp # Async WebSocket server (ws://192.168.4.1/ws)
│   └── dashboard_html.h   # In-memory Tesla-style WebGL perception HUD
├── mobile/                # Cross-platform Flutter mobile workspace
└── docs/                  # Architecture, specifications, and prompt library
    ├── demo/              # Demonstration images and GitHub banner
    └── design.md          # 3D prompt library for generative AI rendering
```

---

## License

This project is licensed under the MIT License. See `LICENSE` for details.
