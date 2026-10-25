# BlindSpottr: System Architecture and Hardware Wiring

This document illustrates the complete physical and functional architecture of the BlindSpottr prototype, including a 2D wiring layout, pin-by-pin hardware mapping, signal data flows, and firmware structure.

---

## 1. 2D Physical Layout and Component Wiring

Below is a 2D visual representation of the ESP32 microcontroller board and all connected peripheral components. It details the exact verified physical wiring of the dual IR presence sensors, cockpit buzzer, SSD1306 OLED, and the HC-SR04 ultrasonic sensor.

```text
 ┌─────────────────────────┐                                 ┌─────────────────────────┐
 │   LEFT IR SENSOR        │                                 │   RIGHT IR SENSOR       │
 │   (TCRT5000 / LM393)    │                                 │   (TCRT5000 / LM393)    │
 │  ┌───────────────────┐  │                                 │  ┌───────────────────┐  │
 │  │ [VCC] [GND] [DO]  │  │                                 │  │ [VCC] [GND] [DO]  │  │
 └──┴───┬─────┬────┬────┴──┘                                 └──┴───┬─────┬────┬────┴──┘
        │     │    │                                                │     │    │
        │     │    └──────────────────────────────────────┐         │     │    │
        │     │                                           │         │     │    │
        │     │    ┌──────────────────────────────────────┴─────────┼─────┼────┘
        │     │    │            ESP32 DEVKIT V1                     │     │
        │     │    │               (38-PIN)                         │     │
        │     │    │                                                │     │
        │     │    │  [VIN] ──────── 5V DC Supply Rail ─────────────┼─────┤
        │     │    │  [3V3] ──────── 3.3V DC Logic Rail ────────────┼─────┤
        │     │    │  [GND] ──────── Common Ground Rail ────────────┼─────┤
        │     │    │                                                │     │
        │     └────┤► [GND]                            [GPIO 22] ◄──┼─────┼───────┐ (SCL)
        └──────────┤► [3V3]                            [GPIO 21] ◄──┼─────┼───┐   │ (SDA)
                   │  [GPIO 35] ◄── (Left DO)          [GPIO 25] ───┼─────┼─┐ │   │   │ (Tone)
                   │  [GPIO 32] ◄── (Right DO)         [GPIO 26] ───┼─────┼─┼─┼───┼───┼─────┐ (TRIG)
                   │  [GPIO 36] ◄── (ECHO Pulse)       [GND]        │     │ │ │   │   │     │
                   └──────────────────▲─────────────────────────────┘     │ │ │   │   │     │
                                      │                                   │ │ │   │   │     │
                                      │                                   │ │ │   │   │     │
             ┌────────────────────────┘                                   │ │ │   │   │     │
             │                                                            │ │ │   │   │     │
             ▼                                                            ▼ ▼ ▼   ▼   ▼     ▼
 ┌─────────────────────────┐                              ┌─────────────────────────┐ ┌─────────────────────────┐
 │   BUZZER MODULE         │                              │   SSD1306 OLED (128x64) │ │   HC-SR04 ULTRASONIC    │
 │   (Active or Passive)   │                              │   (I2C Display)         │ │   (Front Headway)       │
 │  ┌───────────────────┐  │                              │  ┌───────────────────┐  │ │  ┌───────────────────┐  │
 │  │ [GND] [VCC] [SIG] │  │                              │  │[GND] [VCC] [SCL] [SDA│ │  │[VCC] [TRIG][ECHO][GND]│
 └──┴───┬─────┬─────▲───┴──┘                              └──┴──┬─────┬─────▲─────▲─┘ └──┴──┬─────▲─────┬────┬───┘
        │     │     │ (GPIO 25 PWM)                             │     │     │     │         │     │     │    │
        │     └─────┼──────────────── 3.3V Rail ────────────────┼─────┘     │     │         │     │     │    │
        └───────────┼──────────────── Ground Rail ──────────────┴───────────┼─────┼─────────┼─────┼─────┼────┘
                    │                                                       │     │         │     │     │
                    │                                                       │     │    (5V) ┘     │     │
                    │                                                       │     │               │     │
                    │                      (ESP32 GPIO 21 SDA) ─────────────┼─────┘               │     │
                    │                      (ESP32 GPIO 22 SCL) ─────────────┘                     │     │
                    │                                                                             │     │
                    └─────────────────────────────────────────────────────────────────────────────┼─────┘
                                           (ESP32 GPIO 26 Trigger Pulse) ─────────────────────────┘
```

---

## 2. Complete Pin Assignment Matrix

| ESP32 Pin | Function | Peripheral | Pin on Peripheral | Logic Level | Electrical Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **GPIO 35** | Digital Input | Left IR Sensor | DO (Digital Out) | 3.3V Logic | Input-only (GPI). Active-LOW optical comparator. |
| **GPIO 32** | Digital Input | Right IR Sensor | DO (Digital Out) | 3.3V Logic | Digital Input with pull-up capability. Active-LOW. |
| **GPIO 26** | Digital Output | Ultrasonic Sensor | TRIG | 3.3V Output | Generates 10µs ultrasonic start pulse. |
| **GPIO 36 (VP)**| Digital Input | Ultrasonic Sensor | ECHO | ~3.3V Input | Input-only (GPI). Reads echo pulse width. |
| **GPIO 25** | PWM Output | Cockpit Buzzer | SIG / I/O | 3.3V PWM | Driven by LEDC Channel 0 (8-bit multi-frequency tone). |
| **GPIO 21** | I2C Data | SSD1306 OLED | SDA | 3.3V Logic | ESP32 hardware Wire data line. 400 kHz fast mode. |
| **GPIO 22** | I2C Clock | SSD1306 OLED | SCL | 3.3V Logic | ESP32 hardware Wire clock line. 400 kHz fast mode. |
| **VIN** | 5V Power | HC-SR04 | VCC | 5V DC | 5V power supply from USB bus. |
| **3V3** | 3.3V Power | IR & OLED & Buzzer | VCC | 3.3V DC | Regulated onboard 3.3V logic rail. |
| **GND** | Ground | All peripherals | GND | 0V Reference | Common reference ground shared across all components. |

---

## 3. End-to-End Functional Architecture

```mermaid
graph TD
    subgraph SENSING ["1. SENSING LAYER"]
        IR_L["Left IR Sensor (GPIO 35)\nActive-LOW Optical Comparator"]
        IR_R["Right IR Sensor (GPIO 32)\nActive-LOW Optical Comparator"]
        US_FRONT["HC-SR04 Front Sensor\nTRIG: GPIO 26 | ECHO: GPIO 36"]
    end

    subgraph PROCESSING ["2. SIGNAL CONDITIONING & LOGIC"]
        DEB_L["Left Glitch Filter (15 ms)"]
        DEB_R["Right Glitch Filter (15 ms)"]
        US_FILTER["Pulse Conversion & Range Validation (2-300 cm)"]
        FSM{"State Engine Resolver\n(CLEAR / LEFT / RIGHT / FRONT / DUAL / MULTI)"}
    end

    subgraph OUTPUTS ["3. COCKPIT ACTUATION LAYER"]
        BUZZ["Buzzer Controller (GPIO 25)\n- Left: 2200 Hz\n- Right: 2900 Hz\n- Front: 1800 Hz\n- Dual/Multi: 3400/2400 Hz Warble"]
        OLED["SSD1306 HUD (GPIO 21/22)\n- 16 FPS Top-Down Vehicle Wireframe\n- Live Distance Reading (e.g. 15cm)\n- Dynamic Side Radar Wave Arcs\n- Road Guide Markers"]
        UART["Serial Diagnostics\n- 115200 Baud Telemetry Stream"]
    end

    IR_L --> DEB_L
    IR_R --> DEB_R
    US_FRONT --> US_FILTER

    DEB_L --> FSM
    DEB_R --> FSM
    US_FILTER --> FSM

    FSM --> BUZZ
    FSM --> OLED
    FSM --> UART
```
