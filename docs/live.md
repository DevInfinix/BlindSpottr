# BlindSpottr: Live Hardware Bench Test Guide

This document provides step-by-step physical testing procedures and troubleshooting instructions for the integrated BlindSpottr prototype.

---

## 1. Verified Active Hardware Setup

- **Left IR Sensor**: DO connected to **GPIO 34**, VCC to 3.3V, GND to Ground.
- **Right IR Sensor**: DO connected to **GPIO 35**, VCC to 3.3V, GND to Ground.
- **Cockpit Buzzer**: Signal connected to **GPIO 25**, VCC to 3.3V, GND to Ground.
- **SSD1306 OLED (128x64)**: SDA connected to **GPIO 21**, SCL connected to **GPIO 22**, VCC to 3.3V, GND to Ground.

---

## 2. Live Verification Checklist

Follow these steps to verify that every component is working as intended:

### Test 1: Power-On and Initialization
1. Plug the ESP32 into your computer via a USB data cable.
2. Open the serial console at 115200 baud:
   ```bash
   pio device monitor --baud 115200
   ```
3. Observe the boot sequence:
   - The OLED display lights up with a splash screen: `BLINDSPOTTR - SYSTEM READY`.
   - The buzzer remains completely silent.
   - The serial console logs:
     ```text
     ==================================================
       BLINDSPOTTR — TOY TRUCK BSM FIRMWARE v2.0
     ==================================================
     [INFO] System online. Dashboard and audio alerts active.
     ```

### Test 2: Safe Condition (Road Clear)
1. Ensure both IR sensors have a clear line of sight (no objects within 20 cm).
2. Look at the OLED display:
   - The center shows a top-down view of the truck chassis.
   - Forward scanning radar beams emit ahead of the truck cab.
   - Road guidelines stream backward along both sides of the truck.
   - The bottom banner reads: `>>> ROAD CLEAR <<<`.
3. Check audio:
   - The buzzer must remain completely silent (0 Hz).

### Test 3: Left Blind-Spot Detection
1. Place your hand 5 to 10 cm in front of the **Left IR sensor**.
2. Observe the reactions:
   - The indicator LED on the Left IR sensor module lights up.
   - On the OLED, animated radar wave arcs pulse outward to the left from the mirror.
   - An inverted `[!] L` badge illuminates on the left edge of the screen.
   - The bottom banner changes to: `<< LEFT HAZARD! <<`.
   - The buzzer plays a steady, rhythmic **2200 Hz tone** (100 ms on / 120 ms off).
   - Serial monitor prints: `LEFT_RAW=0 [OBJECT] | STATE=LEFT_HAZARD (Buzzer 2200Hz Beep)`.

### Test 4: Right Blind-Spot Detection
1. Remove your hand from the left sensor and place it 5 to 10 cm in front of the **Right IR sensor**.
2. Observe the reactions:
   - The indicator LED on the Right IR sensor module lights up.
   - On the OLED, animated radar wave arcs pulse outward to the right.
   - An inverted `[!] R` badge illuminates on the right edge of the screen.
   - The bottom banner changes to: `>> RIGHT HAZARD! >>`.
   - The buzzer plays a distinct, sharper **2900 Hz tone** (100 ms on / 120 ms off).
   - Serial monitor prints: `RIGHT_RAW=0 [OBJECT] | STATE=RIGHT_HAZARD (Buzzer 2900Hz Beep)`.

### Test 5: Dual Blind-Spot Detection (Both Sides Obstructed)
1. Place hands in front of **both sensors simultaneously**.
2. Observe the reactions:
   - On the OLED, radar waves pulse on both sides simultaneously.
   - The bottom banner flashes: `!! DUAL SIDE WARN !!`.
   - The buzzer sounds a rapid two-tone warble alarm alternating between **3400 Hz** and **2400 Hz** every 50 ms.
   - Serial monitor prints: `STATE=DUAL_HAZARD (Buzzer 3400/2400Hz Rapid Chirp)`.

### Test 6: Recovery to Safe State
1. Remove both hands.
2. The buzzer stops instantly.
3. The OLED returns immediately to the clear state with backward streaming road guidelines.

---

## 3. Hardware Troubleshooting

- **OLED does not light up**:
  - Check that SDA is on GPIO 21 and SCL is on GPIO 22.
  - Verify that VCC receives 3.3V and GND is connected to the common ground rail.
- **Buzzer clicks or stays silent**:
  - Verify the buzzer signal pin connects to GPIO 25.
  - Check if the module is passive or active. The firmware uses hardware PWM (`ledcWriteTone`), which drives both passive piezo transducers and active buzzers.
- **IR sensor triggers constantly or never triggers**:
  - Adjust the sensitivity potentiometer on the sensor module with a small screwdriver until the onboard indicator LED turns on only when your hand is within 10 cm.
