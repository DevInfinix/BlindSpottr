# BlindSpottr: Master Project Specification and System Flow

This document is the main reference for the BlindSpottr project. It explains how the system works in plain language, covers the hardware wiring and firmware architecture, and outlines the operating states without assuming prior knowledge of the codebase.

---

## 1. Project overview

BlindSpottr is an embedded blind-spot monitoring system designed for toy trucks and miniature vehicle prototypes.

In real vehicles, blind spots are areas beside and slightly behind the car that the driver cannot see directly in side mirrors. BlindSpottr detects vehicles or obstacles lingering in these blind spots and warns the driver before they attempt a lane change.

```text
               Ahead / Clear Road
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
2. **SSD1306 OLED graphic display (128x64)**: Shows a top-down view of the truck, live radar waves, animated road motion, and text warnings.
3. **Multi-tone buzzer**: Produces distinct acoustic tones depending on which side has an obstacle so the driver can react without taking their eyes off the road.

---

## 2. How the system works (layman explanation)

The microcontroller (ESP32) acts as the brain of the vehicle. It executes a continuous loop roughly 100 times per second (100 Hz):

1. **Reading the sensors**: The ESP32 checks whether the Left IR sensor (GPIO 34) or Right IR sensor (GPIO 35) detects an object.
2. **Filtering noise**: The raw readings pass through a 15 ms software debounce filter. This prevents transient optical reflections or electrical glitches from triggering false alarms.
3. **Evaluating vehicle safety**: The system decides which of the four main states the truck is in:
   - **ROAD CLEAR**: No obstacle on either side.
   - **LEFT HAZARD**: An obstacle is detected on the left side only.
   - **RIGHT HAZARD**: An obstacle is detected on the right side only.
   - **DUAL HAZARD**: Obstacles are detected on both sides simultaneously.
4. **Delivering alerts**:
   - **Visuals**: The OLED screen shows animated radar wave arcs on the affected side, an inverted warning badge (`[!] L` or `[!] R`), and an alert banner.
   - **Audio**: The buzzer sounds a specific musical pitch for Left (2200 Hz), Right (2900 Hz), or a rapid two-tone alarm (3400/2400 Hz) for Both.
   - **Serial telemetry**: A diagnostic line is sent over the USB cable at 115200 baud for testing.

---

## 3. Physical hardware connections

The system runs on a 38-pin ESP32 DevKit V1 board. All hardware signals connect directly to dedicated GPIO pins:

| Component | Pin on Component | ESP32 GPIO | Electrical description |
| :--- | :--- | :---: | :--- |
| **Left IR Sensor** | Digital Output (DO) | **GPIO 34** | Input-only pin (GPI). Active-LOW (pulls LOW when an object reflects light). |
| **Right IR Sensor** | Digital Output (DO) | **GPIO 35** | Input-only pin (GPI). Active-LOW (pulls LOW when an object reflects light). |
| **Buzzer Module** | Signal (I/O) | **GPIO 25** | Driven by ESP32 hardware PWM (LEDC timer channel 0). |
| **OLED Display** | SDA (Data) | **GPIO 21** | I2C bidirectional serial data line. Operates at 3.3V logic. |
| **OLED Display** | SCL (Clock) | **GPIO 22** | I2C serial clock line. Fast-mode clock set to 400 kHz. |
| **Power Supply** | VCC | **3V3 or VIN** | OLED runs on 3.3V. IR sensors and buzzer run on 3.3V or 5V (VIN). |
| **Ground** | GND | **GND** | Common ground shared across all modules and the ESP32. |

> **Note on ESP32 Input-Only Pins**: GPIO 34, 35, 36, and 39 are input-only (GPI) on the ESP32 silicon. They do not have internal software pull-up resistors. The IR sensor modules must provide an onboard pull-up resistor (standard on LM393 comparator breakout boards).

---

## 4. System states and alert behaviors

| State | Condition | OLED Display Reaction | Buzzer Sound Reaction |
| :--- | :--- | :--- | :--- |
| **SAFE CLEAR** | Neither sensor detects an obstacle | Animated road markings stream backward past the truck; bottom banner displays `>>> ROAD CLEAR <<<`. | Completely silent (`0 Hz`). |
| **LEFT HAZARD** | Left IR detects an obstacle | Radar waves expand outward to the left; inverted `[!] L` badge flashes; bottom banner displays `<< LEFT HAZARD! <<`. | 2200 Hz pulsed tone (100 ms on / 120 ms off). Steady mid-range beep. |
| **RIGHT HAZARD** | Right IR detects an obstacle | Radar waves expand outward to the right; inverted `[!] R` badge flashes; bottom banner displays `>> RIGHT HAZARD! >>`. | 2900 Hz pulsed tone (100 ms on / 120 ms off). Sharper high-pitch beep. |
| **DUAL HAZARD** | Both IR sensors detect obstacles | Radar waves expand on both sides; bottom banner alternates colors with `!! DUAL SIDE WARN !!`. | Rapid two-tone warble alarm switching between 3400 Hz and 2400 Hz every 50 ms. |

---

## 5. Firmware architecture and module roles

The code avoids monolithic design by separating hardware access, display routines, audio synthesis, and main execution into dedicated modules:

- **`include/config.h`**: The single source of hardware truth. Holds all GPIO pin assignments, frequency definitions, debounce intervals, and refresh rates. No magic numbers appear in source files.
- **`src/sensors.h` / `src/sensors.cpp`**: Configures the IR inputs and performs periodic debounced reads. Keeps raw readings and normalized flags clean and accessible.
- **`src/display.h` / `src/display.cpp`**: Initializes the SSD1306 OLED display using the Adafruit SSD1306 and GFX libraries. Renders the top-down truck, draws animated chevrons and radar arcs, and implements bus timeout protection (`Wire.setTimeOut(50)`).
- **`src/buzzer.h` / `src/buzzer.cpp`**: Controls the acoustic output via ESP32 hardware LEDC PWM. Manages non-blocking state transitions and immediate silence upon return to the safe state.
- **`src/main.cpp`**: Coordinates the setup sequence and hosts the non-blocking execution loop. Evaluates the state machine and periodically outputs telemetry to the serial monitor.

---

## 6. Non-blocking timing rules

Embedded systems that monitor safety hazards must never freeze. To guarantee responsiveness:
1. **No `delay()` in runtime loops**: A single `delay(500)` stops sensor polling, freezes display animations, and misses approaching obstacles.
2. **Wrap-safe elapsed time**: All timers use `millis()` arithmetic:
   ```cpp
   if (now - lastTimestamp >= intervalMs) {
       lastTimestamp = now;
       // perform periodic action
   }
   ```
3. **Fail-safe I2C bus**: If the display is disconnected or the I2C wires glitch, the 50 ms timeout prevents the firmware from hanging. Sensor polling and buzzer beeps continue normally.

---

## 7. Future expansion roadmap

- **Microwave Doppler Radar (RCWL-0516)**: Adding microwave radar sensors behind plastic bumpers to detect vehicles approaching from up to 7 meters away.
- **Front Collision Detection (HC-SR04)**: Mounting an ultrasonic sensor on the front bumper to measure headway distance and warn against tailgating.
- **Turn Signal Escalation**: Wiring physical turn signal switches to the ESP32. When turn signals are off, blind-spot alerts remain visual-only; if the driver engages a turn signal into an occupied lane, the buzzer sounds an urgent warning.
