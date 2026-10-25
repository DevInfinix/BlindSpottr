# BlindSpottr: Master Project Specification and System Flow

This document is the main reference for the BlindSpottr project. It explains how the system works in plain language, covers the hardware wiring and firmware architecture, and outlines the operating states without assuming prior knowledge of the codebase.

---

## 1. Project overview

BlindSpottr is an embedded blind-spot monitoring and headway warning system designed for toy trucks and miniature vehicle prototypes.

In real vehicles, blind spots are areas beside and slightly behind the car that the driver cannot see directly in side mirrors. In addition, unexpected obstacles in front of the vehicle require immediate distance tracking. BlindSpottr detects vehicles or obstacles lingering in these blind spots as well as front hazards, alerting the driver before a collision occurs.

```text
               Ahead / Front Ultrasonic (HC-SR04)
                            ▲
                            │
                 ┌──────────────────────┐
                 │     TOY TRUCK CAB    │
       [Left IR] │                      │ [Right IR]
       (GPIO 34) │                      │ (GPIO 35)
          ◄──────┤                      ├──────►
       Left Zone │                      │ Right Zone
                 │      CARGO BED       │
                 └──────────────────────┘
                            │
                            ▼
                     OLED HUD & Buzzer
```

The system uses:
1. **Infrared (IR) presence sensors**: Positioned on the left and right flanks of the vehicle to detect nearby objects.
2. **Ultrasonic distance sensor (HC-SR04)**: Mounted on the front bumper to measure headway distance (2 cm to 300 cm).
3. **SSD1306 OLED graphic display (128x64)**: Shows a top-down view of the truck, live distance in centimeters, animated radar waves, and text warnings.
4. **Multi-tone buzzer**: Produces distinct acoustic tones depending on which side or front detects an obstacle so the driver can react without taking their eyes off the road.

---

## 2. How the system works (layman explanation)

The microcontroller (ESP32) acts as the brain of the vehicle. It executes a continuous loop roughly 100 times per second (100 Hz):

1. **Reading the sensors**:
   - The ESP32 checks whether the Left IR sensor (GPIO 34) or Right IR sensor (GPIO 35) detects an object.
   - It fires the front ultrasonic sensor (TRIG: GPIO 26) every 80 ms and listens for the returned echo pulse on GPIO 36 (VP).
2. **Filtering noise**:
   - The raw IR readings pass through a 15 ms software debounce filter.
   - Ultrasonic echo durations are converted to centimeters and checked against plausible limits (2 cm to 300 cm).
3. **Evaluating vehicle safety**: The system decides which state the truck is in:
   - **ROAD CLEAR**: No obstacles on any monitored side or front.
   - **LEFT HAZARD**: An obstacle is detected on the left side only.
   - **RIGHT HAZARD**: An obstacle is detected on the right side only.
   - **FRONT HAZARD**: Front distance is closer than the 20 cm warning threshold.
   - **DUAL / MULTI HAZARD**: Both sides or multiple zones are obstructed simultaneously.
4. **Delivering alerts**:
   - **Visuals**: The OLED screen shows animated radar waves on the affected side, live centimeter distance ahead of the truck cab, and an alert banner.
   - **Audio**: The buzzer sounds a specific musical pitch for Left (2200 Hz), Right (2900 Hz), Front (1800 Hz urgent pulse), or a rapid two-tone alarm (3400/2400 Hz) for multi-hazards.
   - **Serial telemetry**: Diagnostics stream over USB at 115200 baud for bench testing.

---

## 3. Physical hardware connections

The system runs on a 38-pin ESP32 DevKit V1 board:

| Component | Pin on Component | ESP32 GPIO | Electrical description |
| :--- | :--- | :---: | :--- |
| **Left IR Sensor** | Digital Output (DO) | **GPIO 34** | Input-only (GPI). Active-LOW optical comparator. |
| **Right IR Sensor** | Digital Output (DO) | **GPIO 35** | Input-only (GPI). Active-LOW optical comparator. |
| **HC-SR04 Ultrasonic**| TRIG | **GPIO 26** | 3.3V digital output pulse (10 microseconds). |
| **HC-SR04 Ultrasonic**| ECHO | **GPIO 36 (VP)**| 5V echo pulse stepped down to ~3.3V via 1kΩ / 2.7kΩ divider. |
| **HC-SR04 Ultrasonic**| VCC | **VIN (5V)** | 5V supply rail from USB power. |
| **Buzzer Module** | Signal (I/O) | **GPIO 25** | Driven by ESP32 hardware PWM (LEDC timer channel 0). |
| **OLED Display** | SDA (Data) | **GPIO 21** | I2C serial data line (3.3V logic). |
| **OLED Display** | SCL (Clock) | **GPIO 22** | I2C serial clock line (400 kHz fast mode). |
| **Power Supply** | VCC | **3V3 & VIN** | OLED, IR, and buzzer run on 3.3V; ultrasonic runs on VIN (5V). |
| **Ground** | GND | **GND** | Common ground shared across all modules and the ESP32. |

---

## 4. System states and alert behaviors

| State | Condition | OLED Display Reaction | Buzzer Sound Reaction |
| :--- | :--- | :--- | :--- |
| **SAFE CLEAR** | No obstacles detected | Animated road markings stream backward past the truck; bottom banner displays `>>> ROAD CLEAR <<<`. | Completely silent (`0 Hz`). |
| **LEFT HAZARD** | Left IR detects an obstacle | Radar waves expand outward to the left; inverted `[!] L` badge flashes; bottom banner displays `<< LEFT HAZARD! <<`. | 2200 Hz pulsed tone (100 ms on / 120 ms off). Steady mid-range beep. |
| **RIGHT HAZARD** | Right IR detects an obstacle | Radar waves expand outward to the right; inverted `[!] R` badge flashes; bottom banner displays `>> RIGHT HAZARD! >>`. | 2900 Hz pulsed tone (100 ms on / 120 ms off). Sharper high-pitch beep. |
| **FRONT HAZARD** | Front distance < 20 cm | Warning bar appears ahead of the bumper; live distance in cm shown; bottom banner displays `^^ FRONT HAZARD! ^^`. | 1800 Hz urgent pulse (60 ms on / 60 ms off). |
| **DUAL / MULTI** | Multiple zones obstructed | Radar waves pulse on both sides; bottom banner alternates colors with `!! DUAL SIDE WARN !!`. | Rapid two-tone warble alarm switching between 3400 Hz and 2400 Hz every 50 ms. |

---

## 5. Firmware architecture and module roles

- **`include/config.h`**: Centralized GPIO pin assignments, ultrasonic thresholds, frequency definitions, and timing constants.
- **`src/sensors.h` / `src/sensors.cpp`**: Performs debounced IR reads and scheduled ultrasonic pulse measurements with bounded timeouts.
- **`src/display.h` / `src/display.cpp`**: Renders the animated top-down truck, live distance readouts, side radar arcs, and alert banners at ~16 FPS. Includes a 50 ms I2C timeout to prevent bus lockup.
- **`src/buzzer.h` / `src/buzzer.cpp`**: Controls the acoustic output via ESP32 hardware LEDC PWM with zero CPU overhead.
- **`src/main.cpp`**: Runs the non-blocking execution loop, resolves the state machine, and streams telemetry over UART.
