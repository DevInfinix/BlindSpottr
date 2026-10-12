# BlindSpottr — Detailed System Design Specification

> **AUTHORITY:** This document details the system design, hardware/software boundaries, UI behavior, and engineering trade-offs of the BlindSpottr project. Refer to `AGENTS.md` for coding directives and `docs/flow.md` for authoritative hardware truth.

---

## 1. Design Intent & Problem Statement

### 1.1 The Blind-Spot Problem
Automotive side blind spots are a major cause of highway and city lane-change accidents. In conventional low-cost embedded prototypes:
1. **Ultrasonic / IR sensors alone** fail to detect rapidly approaching vehicles outside their narrow short-range cone.
2. **Doppler radar sensors alone** detect approaching vehicles dynamically, but immediately lose detection when a vehicle matches speed or stops beside the car (zero Doppler frequency shift).

### 1.2 The BlindSpottr Solution
BlindSpottr solves this failure mode through **heterogeneous multi-sensor fusion**:
- **RCWL-0516 Microwave Radar (3 GHz Doppler):** Provides wide-angle, early dynamic warning of approaching objects up to several meters away.
- **Configurable Software Hold Timer (3000 ms):** Bridges the temporal gap as a vehicle decelerates into the blind spot.
- **Short-Range IR Optical Sensors:** Confirms stationary presence once the vehicle comes to a complete halt alongside the sensor.

---

## 2. Sensor Operation & Fusion Mechanics

### 2.1 Sensor Roles & Characteristics

| Parameter | RCWL-0516 Microwave Radar | IR Obstacle Module (TCRT5000 / LM393) |
| :--- | :--- | :--- |
| **Detection Mechanism** | RF Doppler frequency shift | Active infrared reflection |
| **Primary Target** | Moving / approaching objects | Stationary / present objects |
| **Detection Range** | ~3–7 meters | ~10–15 cm (calibrated) |
| **Field of View** | Semi-omnidirectional (~360° / hemispherical) | Narrow directional beam (~30°) |
| **Output Polarity** | Digital pulse: `HIGH` on motion | Digital output: Configurable (default `LOW` on detect) |
| **Blind-Spot Function** | Early advance alert before lane change | Prevents drop-out when stopped at lights / lane match |

### 2.2 Mathematical Fusion Model
For each monitored vehicle side $s \in \{\text{Left}, \text{Right}\}$:

$$\text{RadarHold}_s(t) = \begin{cases} 
1 & \text{if } (t - t_{\text{last\_radar}, s}) < T_{\text{hold}} \\
0 & \text{otherwise}
\end{cases}$$

$$\text{Occupied}_s(t) = \text{IR}_s(t) \lor \text{RadarHold}_s(t)$$

Where:
- $T_{\text{hold}} = 3000\text{ ms}$ (`RADAR_HOLD_MS`).
- $t$ is current system time from `millis()`.
- $\text{IR}_s(t) = 1$ when physical sensor matches `IR_DETECT_LEVEL`.

---

## 3. Global Warning State Machine & Priority Semantics

The system is strictly governed by four mutually exclusive states with deterministic priority:

```text
Priority 1: STATE_BOTH_WARNING   (left_occupied == true AND right_occupied == true)
Priority 2: STATE_LEFT_WARNING   (left_occupied == true AND right_occupied == false)
Priority 3: STATE_RIGHT_WARNING  (left_occupied == false AND right_occupied == true)
Priority 4: STATE_SAFE           (left_occupied == false AND right_occupied == false)
```

### 3.1 Actuation Matrix Across States

| State | Left Red LED (GPIO 25) | Right Red LED (GPIO 26) | Green Safe LED (GPIO 27) | Active Buzzer (GPIO 14) | OLED Display (128×64) | Dashboard UI |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **`STATE_SAFE` (0)** | OFF | OFF | **ON** | Silent (OFF) | `[ SAFE / CLEAR ]` centered banner | Neutral car silhouette, Green status badge |
| **`STATE_LEFT_WARNING` (1)** | **Pulsed / Blinking** | OFF | OFF | Intermittent pulse (e.g. 100ms ON / 100ms OFF) | `◀◀ HAZARD LEFT` arrow graphic | Left zone pulsed Amber/Red |
| **`STATE_RIGHT_WARNING` (2)** | OFF | **Pulsed / Blinking** | OFF | Intermittent pulse (e.g. 100ms ON / 100ms OFF) | `HAZARD RIGHT ▶▶` arrow graphic | Right zone pulsed Amber/Red |
| **`STATE_BOTH_WARNING` (3)** | **Solid ON** | **Solid ON** | OFF | Rapid high-cadence alert (50ms ON / 50ms OFF) | `◀◀ DUAL HAZARD ▶▶` flashing frame | Both zones flashing Red, Warning alert banner |

---

## 4. User Interface Specifications

### 4.1 SSD1306 0.96" OLED Interface (I2C)
The OLED provides immediate line-of-sight status to the driver without requiring a mobile phone connection.

```text
+--------------------------------+
| BLINDSPOTTR            [WIFI]  |  <- Header: System title & AP connection icon
+--------------------------------+
|                                |
|    <<<   LEFT HAZARD   <<<     |  <- Main Alert Zone: Large 16pt font / graphics
|                                |
+--------------------------------+
| R: DETECTED     IR: CLEAR      |  <- Diagnostics Footer: Sensor raw indicators
| IP: 192.168.4.1       UP: 042s |
+--------------------------------+
```

- **Bus Speed:** 400 kHz Fast-Mode I2C.
- **Fail-Safe Design:** If OLED initialization fails (`Wire.endTransmission() != 0`), the firmware logs the error to Serial and continues executing sensing, LED alerts, and web serving without crashing.

### 4.2 Web Dashboard Interface (Phone Browser)
Served over Wi-Fi SoftAP at `http://192.168.4.1`:

1. **Header Bar:** Brand name ("BlindSpottr"), active connection pulse, current system uptime.
2. **Top-Down Vehicle Model:**
   - Visual model car centered on screen.
   - Left side blind-spot zone highlighting (Green when clear, Red with animated waves when occupied).
   - Right side blind-spot zone highlighting (Green when clear, Red with animated waves when occupied).
3. **Sensor Raw Status Badges:**
   - `RADAR L`: `ACTIVE` / `HOLD` / `IDLE`
   - `RADAR R`: `ACTIVE` / `HOLD` / `IDLE`
   - `IR L`: `OBJECT` / `CLEAR`
   - `IR R`: `OBJECT` / `CLEAR`
4. **Responsive Mobile View:** CSS Grid/Flexbox with dark-mode automotive styling, touch-friendly, zero external CDN dependencies (all fonts, SVGs, and styles are inlined in PROGMEM).

---

## 5. Electrical & Hardware Constraints

### 5.1 Power Distribution Architecture
- **ESP32 Vin / 5V Rail:** Powered via USB or regulated 5V breadboard power supply.
- **Sensor Power Rails:**
  - RCWL-0516 modules operate reliably from 4.0V–28V. Connect VCC to the 5V rail; 3.3V OUT pin provides 3.3V TTL compatible logic output.
  - IR obstacle modules operate at 3.3V or 5V. Prefer 3.3V to protect ESP32 GPIO inputs from overvoltage.
- **Decoupling Capacitors:** Microwave radar modules emit RF transients. Place 100 µF electrolytic + 0.1 µF ceramic bypass capacitors across 5V/GND near radar headers to prevent false triggers.

### 5.2 GPIO Pin Safety Checklist
- **GPIO 34 & 35 (IR Inputs):** Input-only silicon pins (GPI). Cannot drive outputs. Internal pull-up resistors are **not** present on these pins. IR sensor modules must provide an onboard active push-pull or pull-up output (standard for LM393 comparator modules).
- **GPIO 0, 2, 12, 15 (Strapping Pins):** Completely avoided to guarantee reboot reliability and prevent entering flash download mode.
- **GPIO 25, 26, 27 (LEDs):** Driven via current-limiting resistors (220 Ω to 330 Ω) to keep sink/source current well below the 12 mA ESP32 recommended limit per pin.

---

## 6. Fault Handling & Fallback Policies

1. **Sensor Disconnection:**
   - Radar floating input: Hardware pull-down resistor or sensor baseline check.
   - IR disconnection: Handled safely through configurable polarity logic.
2. **OLED I2C Bus Hang:**
   - The I2C Wire driver must use a timeout (`Wire.setTimeOut(50)`).
   - Core 0 runs on FreeRTOS independent of Core 1 I2C calls; an I2C bus lockup on Core 1 will never freeze LED warnings or radar sensing on Core 0.
3. **Wi-Fi Client Disconnects / Web Server Flooding:**
   - Asynchronous TCP handles connection drops cleanly.
   - WebSocket buffers are pruned if clients fail to acknowledge, preventing heap exhaustion.
