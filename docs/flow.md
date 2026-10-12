# BLINDSPOTTR --- Intelligent Blind-Spot Monitoring

## Coding & Firmware Source of Truth

**Project:** BlindSpottr\
**Course:** MicroProcessors (MP)\
**Target MCU:** ESP32 38-pin development board\
**Firmware environment:** PlatformIO in VS Code\
**Framework:** Arduino framework on ESP32\
**Document purpose:** This file is the single-context specification for
an AI coding agent. It describes what must be implemented, how the
hardware behaves, how firmware components interact, and what constraints
must not be violated.

> **Important:** This document is intentionally focused on
> implementation. Procurement prices, shopping links, project budget,
> presentation/report schedules, and decorative construction costs are
> excluded.

------------------------------------------------------------------------

# 1. Project Identity

## 1.1 What BlindSpottr is

BlindSpottr is a miniature automotive blind-spot monitoring system
implemented on an ESP32-powered model car.

The system detects activity in the vehicle's **left and right blind-spot
zones** using two different sensing mechanisms:

-   **RCWL-0516 microwave Doppler radar ×2** detects
    **moving/approaching objects**.
-   **IR obstacle/presence sensors ×2** detect a **stationary
    object/vehicle that remains in the blind spot**.

The ESP32 fuses these signals and produces:

1.  Local visual warnings using left/right red LEDs and a green SAFE
    LED.
2.  Local audible warnings using an active buzzer.
3.  Local status information on a 0.96-inch SSD1306 OLED.
4.  A live phone dashboard served directly by the ESP32 over Wi-Fi.
5.  Real-time dashboard updates through WebSocket push rather than HTTP
    polling.

The intended conceptual progression is:

**SAFE → APPROACHING → OCCUPIED**

The radar provides the early-warning/approaching information, while IR
presence detection maintains awareness of a vehicle that has stopped
inside the blind spot.

------------------------------------------------------------------------

# 2. Product / Concept Positioning

The project is technically a student-scale blind-spot detector, but its
implementation and software should present it as a **compact automotive
driver-assistance prototype**, not as a generic "ESP32 + sensors"
experiment.

The coding agent must therefore build the software around these
concepts:

-   automotive-style left/right zone awareness;
-   sensor fusion;
-   deterministic warning states;
-   low-latency event propagation;
-   embedded real-time behavior;
-   local/offline operation;
-   a driver-facing dashboard;
-   clear separation between sensing, decision logic, outputs, and
    connectivity.

Avoid presenting the system as:

-   an IR obstacle detector;
-   a generic IoT dashboard;
-   a web-controlled ESP32;
-   a simple LED project.

The radar + IR fusion, directional blind-spot model, non-blocking
firmware, and real-time dashboard are core to the identity of the
project.

------------------------------------------------------------------------

# 3. Functional Requirements

## 3.1 Required functions

The firmware must:

-   read two RCWL-0516 radar inputs;
-   read two IR presence inputs;
-   distinguish left and right sensor zones;
-   debounce/noise-filter sensor inputs;
-   maintain a radar motion hold timer;
-   combine radar and IR signals into left/right occupancy;
-   derive one of four global warning states;
-   drive three indicator LEDs;
-   drive an active buzzer using non-blocking timing;
-   update an SSD1306 OLED;
-   create an ESP32 Wi-Fi Access Point;
-   serve the dashboard HTML;
-   expose a WebSocket endpoint;
-   push state changes to connected clients;
-   send periodic telemetry heartbeat data;
-   expose enough diagnostic information for development/testing;
-   remain responsive while sensing and serving the dashboard.

## 3.2 Non-functional requirements

The firmware should:

-   avoid blocking operations in runtime paths;
-   never use `delay()` for warning behaviour, blinking, beeping, or
    sensor scheduling;
-   use `millis()`/FreeRTOS timing primitives instead;
-   keep sensing independent from network activity;
-   use deterministic state transitions;
-   keep hardware pin definitions centralized;
-   keep sensor polarity configurable;
-   avoid unnecessary dynamic memory allocation in high-frequency paths;
-   make constants configurable rather than scattering magic numbers;
-   be structured so individual components can be tested independently.

------------------------------------------------------------------------

# 4. Hardware Context

## 4.1 Main components

  ------------------------------------------------------------------------
  Component                                      Qty Firmware role
  --------------------- ---------------------------- ---------------------
  ESP32 38-pin Dev                                 1 Main MCU, Wi-Fi AP,
  Board                                              WebSocket server,
                                                     sensor fusion

  RCWL-0516 microwave                              2 Left/right motion
  radar                                              detection

  IR obstacle/presence                             2 Left/right stationary
  module                                             presence detection

  SSD1306 0.96" I2C                                1 Local status display
  OLED                                               

  Active buzzer module                             1 Audible warning

  Red LEDs                                         2 Left/right warning
                                                     indicators

  Green LED                                        1 SAFE indicator
  ------------------------------------------------------------------------

The final physical system also uses a rechargeable battery/power
subsystem and a soldered/perfboard-style ECU assembly, but those are
hardware implementation concerns rather than firmware interfaces.

------------------------------------------------------------------------

# 5. Pin Map --- AUTHORITATIVE

Do not change these pins unless the hardware design is explicitly
changed.

  ESP32 GPIO   Direction   Device          Signal
  ------------ ----------- --------------- -----------------
  GPIO34       INPUT       RCWL-L          Radar left OUT
  GPIO35       INPUT       RCWL-R          Radar right OUT
  GPIO32       INPUT       IR-L            IR left OUT
  GPIO33       INPUT       IR-R            IR right OUT
  GPIO26       OUTPUT      Red LED L       Left warning
  GPIO27       OUTPUT      Red LED R       Right warning
  GPIO14       OUTPUT      Green LED       SAFE
  GPIO25       OUTPUT      Active buzzer   Warning audio
  GPIO21       I/O         SSD1306         I2C SDA
  GPIO22       I/O         SSD1306         I2C SCL

### Power assumptions

-   RCWL-0516 VIN receives the 5 V rail.
-   RCWL OUT is treated as a 3.3 V-compatible digital signal.
-   IR modules are powered from 3.3 V so their output remains within
    ESP32 logic limits.
-   OLED is powered from 3.3 V.
-   All modules share common ground.

### ESP32 pin constraints

GPIO34 and GPIO35 are input-only pins and do not provide normal internal
pull-ups/pull-downs. Do not configure them as outputs.

------------------------------------------------------------------------

# 6. Sensor Semantics

## 6.1 Left/right naming

The firmware must consistently use:

-   `RAD_L` → left radar
-   `RAD_R` → right radar
-   `IR_L` → left IR
-   `IR_R` → right IR
-   `LED_L` → left red warning
-   `LED_R` → right red warning

Do not swap physical sides in software.

------------------------------------------------------------------------

# 7. RCWL-0516 Radar Behaviour

The RCWL-0516 is a microwave Doppler motion sensor.

For this project:

-   `HIGH` means radar motion detected.
-   `LOW` means no current radar trigger.
-   The radar is treated as a **motion detector**, not a persistent
    occupancy detector.
-   A radar event therefore starts/refreshes a software hold timer.

The source design expects directional behaviour to be created physically
using shielded radar pods rather than through software.

## 7.1 Radar hold

A radar trigger must refresh a **3-second hold timer** for that side.

Conceptually:

``` text
radar HIGH
    ↓
refresh radar hold timestamp
    ↓
radar_hold_active = true for 3 seconds after last trigger
```

This solves the case where a moving vehicle enters the blind spot and
then stops before the IR sensor becomes the persistent presence signal.

Do not immediately clear the warning when radar goes LOW.

------------------------------------------------------------------------

# 8. IR Sensor Behaviour

IR modules are used as short-range presence detectors.

The source design expects many common IR obstacle modules to be
**active-low**:

``` cpp
IR_DETECT = LOW
```

However, the actual module batch must be verified during hardware
testing.

Therefore:

-   keep IR polarity configurable;
-   do not bury the polarity assumption throughout the code;
-   define it in one configuration location;
-   provide a simple way to invert it if required.

Expected demo tuning is approximately 10--15 cm, with the broader module
capability described as roughly 2--30 cm.

------------------------------------------------------------------------

# 9. Sensor Fusion Model

This is one of the most important parts of the project.

For each side:

``` text
side_occupied = IR_detected || radar_hold_active
```

Therefore:

``` text
left_occupied  = IR_L_detected || left_radar_hold_active
right_occupied = IR_R_detected || right_radar_hold_active
```

### Why this exists

Radar detects motion but does not inherently represent a stopped object.

IR provides short-range persistent presence.

Together:

``` text
moving/approaching vehicle
        ↓
     RADAR
        ↓
  early warning
        ↓
vehicle stops
        ↓
       IR
        ↓
persistent warning
```

The firmware must preserve this behaviour.

------------------------------------------------------------------------

# 10. Global Warning State Machine

There are exactly four primary application states:

``` text
SAFE
LEFT_WARNING
RIGHT_WARNING
BOTH_WARNING
```

State derivation:

``` text
if (!left && !right)       → SAFE
if ( left && !right)       → LEFT_WARNING
if (!left &&  right)       → RIGHT_WARNING
if ( left &&  right)       → BOTH_WARNING
```

Equivalent compact logic:

``` cpp
state = left && right ? BOTH :
        left          ? LEFT :
        right         ? RIGHT :
                        SAFE;
```

## 10.1 State transitions

``` text
                 L occupied
SAFE ─────────────────────────→ LEFT_WARNING
SAFE ─────────────────────────→ RIGHT_WARNING
SAFE ─────────────────────────→ BOTH_WARNING

LEFT_WARNING ── R occupied ──→ BOTH_WARNING
LEFT_WARNING ── L clear ──────→ SAFE

RIGHT_WARNING ─ R occupied ───→ BOTH_WARNING
RIGHT_WARNING ─ R clear ───────→ SAFE

BOTH_WARNING ── R clear ───────→ LEFT_WARNING
BOTH_WARNING ── L clear ───────→ RIGHT_WARNING

BOTH_WARNING ── both clear ────→ SAFE
```

The implementation may calculate the state directly from left/right
occupancy rather than implementing a large switch-based transition
engine, provided externally observable behaviour remains identical.

------------------------------------------------------------------------

# 11. Output Behaviour

## 11.1 LED behaviour

  State           Green   Red Left     Red Right
  --------------- ------- ------------ ------------
  SAFE            ON      OFF          OFF
  LEFT_WARNING    OFF     Blink        OFF
  RIGHT_WARNING   OFF     OFF          Blink
  BOTH_WARNING    OFF     Fast blink   Fast blink

Source timing targets:

-   single-side warning: approximately **600 ms ON / 400 ms OFF**;
-   both-side warning: approximately **150 ms cadence**.

The exact implementation should use a non-blocking state machine.

## 11.2 Buzzer behaviour

  State           Buzzer
  --------------- ------------
  SAFE            Silent
  LEFT_WARNING    Slow beep
  RIGHT_WARNING   Slow beep
  BOTH_WARNING    Continuous

The buzzer must never be controlled using blocking delays.

## 11.3 OLED behaviour

The OLED should communicate the same high-level state locally:

``` text
SAFE
LEFT BLIND SPOT!
RIGHT BLIND SPOT!
BOTH SIDES!!
```

The display may also show compact sensor/status information where
useful, but the main warning state must remain immediately readable.

OLED communication should be updated:

-   on state change;
-   or when displayed information actually changes.

Do not redraw the entire display unnecessarily on every 20 ms sensor
cycle.

------------------------------------------------------------------------

# 12. Runtime Architecture

## 12.1 Dual-core model

The ESP32 is treated as a dual-core FreeRTOS MCU.

### Core 0 --- sensing

A dedicated FreeRTOS task handles:

-   radar input reads;
-   IR input reads;
-   debouncing/filtering;
-   radar hold timers;
-   sensor fusion;
-   application state calculation;
-   local output state management.

Target sampling period:

**20 ms**

That corresponds to approximately:

**50 sensor evaluations per second**

### Core 1 --- connectivity/application loop

The normal Arduino `loop()` handles:

-   WebSocket state publication;
-   telemetry heartbeat;
-   connection-related application work;
-   other non-real-time tasks that should not block sensing.

The asynchronous web server operates independently of the sensor task.

------------------------------------------------------------------------

# 13. Timing Architecture

## 13.1 Required timing targets

  Operation                                                  Target
  ------------------------------- ---------------------------------
  Sensor sampling                                             20 ms
  Debounce/filter window                                     ≤40 ms
  Radar hold                                                3000 ms
  WebSocket local push                approximately 5--15 ms target
  Phone render                      approximately one display frame
  End-to-end motion → dashboard                     target \<100 ms

These are engineering targets, not guarantees. The implementation must
not falsely claim measured latency unless it has actually been measured.

## 13.2 Non-blocking rule

**Never use `delay()` in the runtime warning/control path.**

Use:

-   `millis()` state machines for LED blinking;
-   `millis()` state machines for buzzer cadence;
-   FreeRTOS `vTaskDelay()` only for the sensor task's periodic
    scheduling;
-   asynchronous networking for HTTP/WebSocket operations.

`vTaskDelay(pdMS_TO_TICKS(20))` is appropriate at the end of the sensor
task's periodic loop.

------------------------------------------------------------------------

# 14. Shared State Between Cores

The sensor task produces a small shared system state consumed by the
connectivity layer.

Conceptually:

``` cpp
struct SystemState {
    uint8_t state;
    bool left;
    bool right;
};
```

The source design uses a shared `volatile` state.

The implementation should make cross-core access safe and deterministic.
If the chosen implementation requires a critical section, atomic access,
or another synchronization primitive, use the lightest correct
mechanism.

Do not introduce a large shared mutable object.

Recommended logical fields:

``` text
state
leftOccupied
rightOccupied
leftRadarActive
rightRadarActive
leftRadarHold
rightRadarHold
uptime
```

Only expose fields that are actually useful to the
dashboard/diagnostics.

------------------------------------------------------------------------

# 15. Event Model

The firmware should distinguish between:

### Raw sensor events

``` text
radar_left
radar_right
ir_left
ir_right
```

### Derived events

``` text
left_occupied
right_occupied
state_changed
```

### Output events

``` text
LED pattern changed
buzzer pattern changed
OLED status changed
dashboard state changed
```

This separation makes debugging and future extensions easier.

------------------------------------------------------------------------

# 16. Web Connectivity

## 16.1 Wi-Fi mode

The ESP32 operates as a **Wi-Fi Access Point**.

No internet connection or external router is required.

Current intended configuration:

``` text
SSID: BSD-RADAR
Password: blindspot123
AP IP: 192.168.4.1
```

Keep SSID/password in one configuration section so they can be changed
without modifying application logic.

The fixed default AP address should be treated as the expected dashboard
address unless the network configuration is intentionally changed.

------------------------------------------------------------------------

# 17. HTTP Server

The ESP32 serves the dashboard from the root route:

``` text
GET /
```

Expected response:

``` text
text/html
```

The dashboard is intended to be embedded in the firmware as a static
HTML/CSS/JavaScript asset.

The original design calls this embedded asset:

``` text
DASH_HTML
```

The implementation may use a separate header/source file or another
PlatformIO-friendly embedding strategy, but the resulting firmware must
serve the complete dashboard locally.

------------------------------------------------------------------------

# 18. WebSocket Interface

WebSocket endpoint:

``` text
/ws
```

Client connection:

``` javascript
new WebSocket("ws://192.168.4.1/ws")
```

The dashboard is **push-based**.

Do not implement periodic browser polling such as:

``` text
setInterval(fetch(...))
```

for the main state.

The ESP32 must push updates when the application state changes.

------------------------------------------------------------------------

# 19. JSON Protocol

The original minimum state frame is:

``` json
{
  "s": 0,
  "L": 0,
  "R": 0
}
```

Where:

  Field   Meaning
  ------- ------------------------
  `s`     global state ID
  `L`     left occupied boolean
  `R`     right occupied boolean

Recommended state IDs:

``` text
0 = SAFE
1 = LEFT_WARNING
2 = RIGHT_WARNING
3 = BOTH_WARNING
```

The protocol may be expanded with diagnostic fields if useful, but do
not break the minimum fields above.

A possible extended telemetry frame may contain:

``` json
{
  "s": 1,
  "L": 1,
  "R": 0,
  "radarL": 1,
  "radarR": 0,
  "irL": 1,
  "irR": 0,
  "uptime": 12345
}
```

If extended fields are added, the dashboard must remain compatible with
the core state fields.

------------------------------------------------------------------------

# 20. WebSocket Update Policy

### On state change

Immediately broadcast the new state.

Example:

``` text
SAFE
 ↓
LEFT_WARNING
 ↓
broadcast immediately
```

### Heartbeat

The source design specifies a telemetry heartbeat approximately every:

**500 ms**

The heartbeat can contain uptime and sensor diagnostics.

Do not use the heartbeat as a substitute for event-driven state updates.

------------------------------------------------------------------------

# 21. Dashboard Requirements

The dashboard should be a **dark automotive instrument-cluster UI**.

Core visual elements:

1.  Top-view representation of the model car.
2.  Clearly separated left and right blind-spot zones.
3.  Left zone glows/highlights red when left side is active.
4.  Right zone glows/highlights red when right side is active.
5.  Central status banner.
6.  Buzzer/audio-warning indicator.
7.  Raw left/right sensor status chips.
8.  Connection status.
9.  Optional compact telemetry/diagnostic information.

The dashboard is not a generic admin panel.

It should visually communicate:

``` text
CAR
├── LEFT BLIND SPOT
├── VEHICLE STATUS
└── RIGHT BLIND SPOT
```

The dashboard must remain usable on a phone screen.

------------------------------------------------------------------------

# 22. Dashboard State Mapping

  Firmware state   Dashboard
  ---------------- ---------------------------------------------------
  SAFE             Neutral/green safe state
  LEFT_WARNING     Left blind-spot zone highlighted
  RIGHT_WARNING    Right blind-spot zone highlighted
  BOTH_WARNING     Both zones highlighted with high-priority warning

The dashboard should visibly distinguish:

-   safe;
-   active left;
-   active right;
-   simultaneous left + right hazard.

State changes should happen from WebSocket messages rather than polling.

------------------------------------------------------------------------

# 23. Firmware Module Boundaries

The implementation should not become one enormous `.ino`/`.cpp` file.

Prefer logical modules such as:

``` text
src/
├── main.cpp
├── config.h
├── sensors/
│   ├── radar.h
│   ├── radar.cpp
│   ├── ir.h
│   └── ir.cpp
├── logic/
│   ├── fusion.h
│   ├── fusion.cpp
│   └── state_machine.h
├── outputs/
│   ├── indicators.h
│   ├── indicators.cpp
│   ├── buzzer.h
│   ├── buzzer.cpp
│   ├── display.h
│   └── display.cpp
├── network/
│   ├── wifi_manager.h
│   ├── wifi_manager.cpp
│   ├── websocket.h
│   └── websocket.cpp
└── web/
    └── dashboard.h
```

This is a recommended organization, not a mandatory exact file layout.

The important architectural boundaries are:

``` text
hardware input
      ↓
sensor abstraction
      ↓
filtering / timing
      ↓
sensor fusion
      ↓
application state
      ↓
outputs + network publication
```

------------------------------------------------------------------------

# 24. PlatformIO Requirements

The project must be implemented as a **PlatformIO project**, not an
Arduino IDE-only sketch.

The coding agent must create/maintain:

``` text
platformio.ini
src/
include/        (if useful)
lib/            (if useful)
test/           (if useful)
```

The exact PlatformIO board identifier must match the actual ESP32 38-pin
board being used. If the board is the common ESP32 Dev Module/DevKit
equivalent, `esp32dev` is a likely identifier, but the physical board
definition must be verified before finalizing the configuration.

Use the Arduino framework.

The source specification explicitly requires functionality corresponding
to:

-   `WiFi.h`
-   `ESPAsyncWebServer`
-   `AsyncWebSocket`
-   `Adafruit_GFX`
-   `Adafruit_SSD1306`

Dependency versions must be chosen based on actual PlatformIO/ESP32
compatibility rather than blindly copying arbitrary versions.

If `ESPAsyncWebServer` requires an accompanying ESP32 asynchronous TCP
dependency for the selected library version, configure that dependency
correctly.

------------------------------------------------------------------------

# 25. PlatformIO Configuration Principles

The configuration should:

-   target the correct ESP32 architecture;
-   use the Arduino framework;
-   support serial monitoring for diagnostics;
-   define an appropriate monitor baud rate consistently with firmware;
-   include all required libraries;
-   avoid unnecessary dependencies;
-   avoid downloading or depending on cloud services at runtime;
-   build reproducibly.

A typical development configuration may include:

``` ini
[env:esp32dev]
platform = espressif32
framework = arduino
monitor_speed = 115200
```

The board value must be confirmed against the actual hardware before
treating this example as authoritative.

------------------------------------------------------------------------

# 26. Configuration Constants

Centralize configurable values.

At minimum:

``` text
RAD_L
RAD_R
IR_L
IR_R
LED_L
LED_R
LED_G
BUZ

OLED_SDA
OLED_SCL
OLED_ADDRESS

IR_DETECT_LEVEL

SENSOR_PERIOD_MS = 20
RADAR_HOLD_MS = 3000
DEBOUNCE_MS ≈ 40

WIFI_SSID
WIFI_PASSWORD

WS_PATH = "/ws"
```

Avoid magic numbers throughout implementation files.

------------------------------------------------------------------------

# 27. OLED Requirements

Controller:

``` text
SSD1306
```

Resolution:

``` text
128 × 64
```

I2C address:

``` text
0x3C
```

Pins:

``` text
SDA = GPIO21
SCL = GPIO22
```

The firmware should initialize the display during startup and fail
gracefully if initialization fails.

A display failure must not stop sensor fusion, LEDs, buzzer, or the
network dashboard.

------------------------------------------------------------------------

# 28. Serial Diagnostics

Serial output is important during development.

Provide a debug mode/configuration option.

Useful diagnostics include:

``` text
boot/startup
firmware version
Wi-Fi AP started
AP IP address
WebSocket client connected/disconnected
raw radar L/R
raw IR L/R
derived left/right occupancy
global state changes
OLED initialization result
```

Do not print raw sensor values every 20 ms indefinitely in normal
operation; that can flood the serial output and interfere with useful
debugging.

Prefer:

-   state-change logging;
-   periodic diagnostic summaries;
-   explicit sensor-test mode.

------------------------------------------------------------------------

# 29. Startup Sequence

Recommended startup order:

``` text
1. Initialize serial
2. Configure GPIO modes
3. Initialize I2C
4. Initialize OLED
5. Initialize outputs to SAFE/off
6. Initialize Wi-Fi AP
7. Initialize WebSocket/server
8. Start sensor FreeRTOS task
9. Enter normal runtime
```

At startup, the system must not falsely report an active blind spot due
to uninitialized state.

Default logical state:

``` text
left = false
right = false
state = SAFE
```

------------------------------------------------------------------------

# 30. Sensor Task Behaviour

Conceptual implementation:

``` cpp
void sensorTask(void*) {
    for (;;) {
        uint32_t now = millis();

        bool radarLeft  = digitalRead(RAD_L) == HIGH;
        bool radarRight = digitalRead(RAD_R) == HIGH;

        if (radarLeft)  leftRadarHoldTimestamp  = now;
        if (radarRight) rightRadarHoldTimestamp = now;

        bool leftRadarHeld =
            (now - leftRadarHoldTimestamp) < RADAR_HOLD_MS;

        bool rightRadarHeld =
            (now - rightRadarHoldTimestamp) < RADAR_HOLD_MS;

        bool leftIR =
            digitalRead(IR_L) == IR_DETECT_LEVEL;

        bool rightIR =
            digitalRead(IR_R) == IR_DETECT_LEVEL;

        bool leftOccupied  = leftIR || leftRadarHeld;
        bool rightOccupied = rightIR || rightRadarHeld;

        updateSystemState(leftOccupied, rightOccupied);

        updateLocalOutputsNonBlocking();

        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}
```

This is architectural pseudocode, not necessarily the final
implementation.

------------------------------------------------------------------------

# 31. Debouncing / Filtering

The source design calls for approximately ≤40 ms debounce/filtering.

The implementation should prevent very short sensor glitches from
causing visible state chatter.

Do not over-filter radar signals to the point that genuine approaching
motion is ignored.

Radar hold is the primary persistence mechanism.

IR filtering should be conservative because IR is responsible for
stationary occupancy.

------------------------------------------------------------------------

# 32. Output State Machine

Local output timing must be independent from sensor sampling.

Do not write:

``` cpp
delay(600);
digitalWrite(...);
delay(400);
```

Instead maintain timestamps:

``` text
lastBlinkChange
lastBuzzerChange
currentOutputPhase
```

and update them each runtime iteration.

This allows:

-   sensor processing to continue;
-   WebSocket communication to continue;
-   OLED updates to continue;
-   LEDs/buzzer to animate simultaneously.

------------------------------------------------------------------------

# 33. OLED Update Strategy

Maintain a cached display state.

Only redraw when:

``` text
global state changed
OR
display text changed
OR
diagnostic page changed
```

Avoid performing I2C display writes every 20 ms.

The OLED is an output device, not part of the real-time sensing loop.

------------------------------------------------------------------------

# 34. Error Handling

The system should degrade gracefully.

### OLED unavailable

Continue:

``` text
sensor fusion
LEDs
buzzer
Wi-Fi
dashboard
```

### WebSocket client unavailable

Continue:

``` text
sensor fusion
LEDs
buzzer
OLED
```

The car must never depend on the phone dashboard to perform blind-spot
detection.

### Wi-Fi unavailable

Local safety outputs must continue operating.

### One sensor disconnected

The firmware should remain running and expose the abnormal condition
through diagnostics where practical.

Do not silently reinterpret a hardware failure as a valid "SAFE"
condition if the failure can be detected.

------------------------------------------------------------------------

# 35. Development / Test Modes

The project should ideally support a compile-time or runtime diagnostic
mode.

Useful test modes:

## Sensor test

Display/log:

``` text
RAD-L
RAD-R
IR-L
IR-R
```

and their raw logic levels.

## Output test

Cycle:

``` text
SAFE
LEFT
RIGHT
BOTH
```

without requiring physical sensors.

This allows:

-   LED verification;
-   buzzer verification;
-   OLED verification;
-   dashboard verification;
-   WebSocket verification.

## Network test

Verify:

``` text
AP visible
phone connected
HTTP page served
WebSocket connected
state frame received
```

------------------------------------------------------------------------

# 36. Acceptance Criteria

The firmware is considered functionally complete when all of the
following work.

### Sensor acceptance

-   left radar detects motion;
-   right radar detects motion;
-   left IR detects nearby presence;
-   right IR detects nearby presence;
-   IR polarity is verified/configurable;
-   radar hold persists for approximately 3 seconds after motion stops.

### Fusion acceptance

-   left activity produces LEFT_WARNING;
-   right activity produces RIGHT_WARNING;
-   both sides produce BOTH_WARNING;
-   clearing one side from BOTH returns to the other side's warning;
-   clearing both sides returns to SAFE.

### Local output acceptance

-   SAFE → green LED;
-   LEFT → left red warning;
-   RIGHT → right red warning;
-   BOTH → both red warnings;
-   buzzer cadence matches state;
-   OLED text matches state.

### Network acceptance

-   phone connects to ESP32 AP;
-   `192.168.4.1` serves dashboard;
-   `/ws` accepts a WebSocket connection;
-   state changes are pushed immediately;
-   500 ms heartbeat/telemetry is available;
-   dashboard left/right visualization follows firmware state.

### Concurrency acceptance

-   sensor task continues at approximately 20 ms;
-   dashboard remains responsive during sensor activity;
-   no blocking `delay()` is used in warning/control logic;
-   local outputs continue if phone disconnects.

------------------------------------------------------------------------

# 37. Physical Assumptions Relevant to Firmware

The firmware relies on the following physical design:

-   two radar modules are mounted on opposite rear corners;
-   radar pods are physically shielded with foil;
-   each radar points approximately outward/rearward at 45°;
-   IR sensors are positioned on the rear-quarter sides;
-   IR range is adjusted for the scale-model blind spot;
-   the OLED/LED cluster is visible to the examiner;
-   ESP32 and electronics are mounted inside the model car.

Software must not attempt to compensate for physical radar
directionality that is intended to be handled by the sensor pods.

------------------------------------------------------------------------

# 38. Radar Range-Tuning Context

The RCWL-0516 has a much larger default sensing range than a small
tabletop model requires.

The hardware design specifies a resistor across the module's `R-GN` pads
to reduce effective range.

Target demo behaviour is approximately:

``` text
1–2 m radar detection envelope
```

The exact resistor value must be experimentally selected.

Candidate testing range from the source design:

``` text
470 kΩ – 2 MΩ
```

A nominal \~1 MΩ value is suggested as the starting point.

This is a hardware calibration concern; do not encode a fake software
"range" value and present it as physical radar range.

------------------------------------------------------------------------

# 39. Important Electrical Constraints

Never violate these assumptions:

-   Do not feed 5 V directly into normal ESP32 GPIO inputs.
-   RCWL VIN may use the 5 V rail.
-   IR modules should use 3.3 V.
-   GPIO34/35 are inputs only.
-   LEDs require current-limiting resistors.
-   The buzzer module is assumed to contain its own driver/transistor.
-   All grounds must be common.

The firmware should not attempt to solve unsafe electrical wiring
through software.

------------------------------------------------------------------------

# 40. Power-System Context

The final hardware uses:

``` text
18650 Li-ion
   ↓
TP4056 charging/protection
   ↓
MT3608 boost
   ↓
5 V system rail
```

The 5 V rail supplies:

-   ESP32 VIN;
-   RCWL sensors;
-   buzzer module.

The ESP32 3.3 V rail supplies:

-   IR sensors;
-   OLED.

The firmware should assume a stable supply and should not perform
aggressive high-frequency display/network operations that unnecessarily
increase power consumption.

------------------------------------------------------------------------

# 41. Dashboard / Firmware Contract

The dashboard and firmware are two parts of one product.

The firmware owns:

``` text
sensor truth
fusion
state
timing
safety outputs
```

The dashboard owns:

``` text
visualization
status presentation
connection presentation
human-readable telemetry
```

The dashboard must not independently infer the blind-spot state from raw
sensor values when the firmware already provides the authoritative
state.

The dashboard should render the state received from the ESP32.

------------------------------------------------------------------------

# 42. Security / Runtime Scope

This is a local demonstration system.

Requirements:

-   no cloud backend;
-   no external API;
-   no internet dependency;
-   no database;
-   no authentication service;
-   no external telemetry service.

The Wi-Fi AP password should not be hard-coded in multiple places.

For the final demo, local AP operation is intentional.

------------------------------------------------------------------------

# 43. Code Quality Rules for the AI Agent

When modifying or generating code:

1.  Read this entire specification before making architectural
    decisions.
2.  Preserve the pin map unless explicitly instructed otherwise.
3.  Preserve the sensor-fusion model.
4.  Preserve the four-state state model.
5.  Preserve the 3-second radar hold.
6.  Preserve 20 ms sensing cadence as the target.
7.  Never introduce blocking `delay()` into runtime control logic.
8.  Keep sensing independent of networking.
9.  Keep dashboard state derived from firmware state.
10. Keep configuration constants centralized.
11. Do not silently change sensor polarity.
12. Do not silently change Wi-Fi endpoint paths.
13. Do not add cloud services or unnecessary dependencies.
14. Do not replace WebSocket push with polling.
15. Do not add speculative hardware features without clearly marking
    them.
16. Prefer small, testable modules over a monolithic source file.
17. Preserve backward compatibility with the minimum WebSocket JSON
    fields.
18. Explain any hardware assumption that is not explicitly defined in
    this document.
19. Before changing architecture, identify the requirement that
    motivates the change.
20. Build and test after meaningful changes.

------------------------------------------------------------------------

# 44. Expected Repository Structure

Recommended starting structure:

``` text
BlindSpottr/
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── config.h
│   ├── sensors/
│   ├── logic/
│   ├── outputs/
│   ├── network/
│   └── web/
├── include/
├── lib/
├── test/
└── README.md
```

The AI agent may choose a slightly different structure if it improves
maintainability, but the logical separation must remain clear.

------------------------------------------------------------------------

# 45. Suggested Implementation Order

Implement in this order to reduce debugging complexity:

## Phase 1 --- PlatformIO skeleton

-   create PlatformIO project;
-   configure ESP32;
-   configure serial;
-   verify build/upload.

## Phase 2 --- GPIO/sensor validation

-   configure all pins;
-   read radar inputs;
-   read IR inputs;
-   print diagnostic states;
-   verify IR polarity.

## Phase 3 --- Fusion logic

-   implement radar hold;
-   implement debounce/filtering;
-   implement left/right occupancy;
-   implement four-state model;
-   unit-test or simulate state transitions where practical.

## Phase 4 --- Local outputs

-   LEDs;
-   buzzer;
-   OLED;
-   non-blocking output timing.

## Phase 5 --- Wi-Fi

-   start AP;
-   verify fixed IP;
-   serve a basic page.

## Phase 6 --- WebSocket

-   add `/ws`;
-   push state changes;
-   add heartbeat;
-   verify browser/client reception.

## Phase 7 --- Dashboard

-   implement automotive cluster UI;
-   connect WebSocket;
-   map state to visual zones;
-   add connection/diagnostic presentation.

## Phase 8 --- Integration

-   test all four states physically;
-   test radar → hold → IR persistence;
-   test both-side warning;
-   disconnect phone and verify local outputs;
-   verify long-running stability.

------------------------------------------------------------------------

# 46. Known Source Decisions That Must Be Preserved

The following are intentional design decisions from the original project
plan:

### Decision 1 --- Radar + IR fusion

Do not simplify the system to IR-only or radar-only during the primary
implementation.

### Decision 2 --- Radar is an early-warning sensor

Radar represents motion/approach, not permanent occupancy.

### Decision 3 --- IR handles stationary presence

IR exists specifically to cover radar's Doppler limitation.

### Decision 4 --- Three-second radar hold

The hold bridges the transition from moving vehicle to stationary
vehicle.

### Decision 5 --- Dual-core architecture

Sensor processing should be isolated from dashboard/network work.

### Decision 6 --- WebSocket push

The phone dashboard is event-driven, not polling-driven.

### Decision 7 --- Local AP

The project must work without internet or a router.

### Decision 8 --- Local outputs remain authoritative

The car must still warn the user if the phone is disconnected.

### Decision 9 --- Automotive presentation

Software/UI should communicate an automotive driver-assistance product
rather than a generic electronics experiment.

------------------------------------------------------------------------

# 47. Known Ambiguities / Items Requiring Hardware Verification

The coding agent must not invent facts where the source specification
leaves them unresolved.

These items must be verified during implementation/testing:

1.  Exact PlatformIO board identifier for the physical ESP32 38-pin
    board.
2.  Exact ESP32/Arduino platform version compatible with selected
    asynchronous web libraries.
3.  Exact `ESPAsyncWebServer`/`AsyncTCP` dependency versions.
4.  Actual IR module active level.
5.  Actual OLED I2C address, expected `0x3C`.
6.  Actual radar range after physical shielding and resistor tuning.
7.  Whether the physical RCWL modules behave consistently with
    `HIGH = motion`.
8.  Actual display rendering limits for the chosen SSD1306 library
    version.
9.  Actual power stability under Wi-Fi transmission.
10. Real measured end-to-end latency.

When these are tested, update configuration/documentation with measured
facts rather than assumptions.

------------------------------------------------------------------------

# 48. Reference Project Material

A separate `reference` project/folder may contain previous-project
implementation patterns.

When available to the coding agent:

-   inspect it before deciding on repository conventions;
-   reuse useful proven patterns where appropriate;
-   do not copy unrelated architecture merely because it exists in the
    reference;
-   do not allow reference code to override this specification's
    hardware pin map, sensor semantics, state machine, or network
    contract;
-   treat this document as the project-specific source of truth.

------------------------------------------------------------------------

# 49. Final System Model

The entire runtime can be reduced to this pipeline:

``` text
                 ┌───────────────┐
                 │ RCWL LEFT     │
                 └──────┬────────┘
                        │ motion
                        ▼
                 ┌───────────────┐
                 │ RADAR HOLD    │
                 │ 3 seconds     │
                 └──────┬────────┘
                        │
                        ├──────────────┐
                        │              │
                 ┌──────▼──────┐ ┌────▼───────┐
                 │ IR LEFT     │ │ IR RIGHT   │
                 └──────┬──────┘ └────┬───────┘
                        │             │
                        ▼             ▼
                   LEFT FUSION    RIGHT FUSION
                        │             │
                        └──────┬──────┘
                               ▼
                    ┌──────────────────┐
                    │ STATE MACHINE    │
                    │ SAFE / LEFT /    │
                    │ RIGHT / BOTH     │
                    └───────┬──────────┘
                            │
          ┌─────────────────┼──────────────────┐
          ▼                 ▼                  ▼
     LEDs + Buzzer        OLED          WebSocket Push
                                             │
                                             ▼
                                      Phone Dashboard
```

The core product behaviour is:

``` text
APPROACHING VEHICLE
        ↓
   RADAR DETECTS
        ↓
  EARLY WARNING
        ↓
 VEHICLE STOPS
        ↓
    IR DETECTS
        ↓
  OCCUPIED WARNING
        ↓
 PHONE + OLED + LEDs + BUZZER
```

------------------------------------------------------------------------

# 50. One-Sentence Definition

**BlindSpottr is an ESP32-based automotive driver-assistance prototype
that fuses Doppler radar motion detection with IR stationary-presence
detection to provide low-latency left/right blind-spot warnings locally
and through a live phone dashboard.**
