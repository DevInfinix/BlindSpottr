# BlindSpottr: Industrial Design and Visual Reference Prompt Library

This document is a prompt library for generative AI image pipelines (such as Midjourney v6, Stable Diffusion XL, FLUX.1, or Imagen 3). Use these prompts alongside user-supplied reference photographs of the scale-model toy truck, perfboard, and electronic modules (ESP32, TCRT5000 IR sensors, SSD1306 OLED, active buzzer) to generate clean, production-grade hardware visualizations of the completed build.

---

## 1. Master System Configuration and Design Language

All prompts adhere to this physical specification:
- **Base Vehicle**: Scale-model industrial flatbed / box toy truck (heavy-duty utilitarian chassis, matte commercial livery, rubber treaded tires, scale 1:16 or 1:18).
- **Core Electronics Bay**: Custom dark fiberglass FR-4 solder perfboard with green solder mask, clean soldered bus wiring, and gold-plated headers mounted securely onto the flatbed cargo deck.
- **Microcontroller**: ESP32 DevKit V1 (38-pin black matte PCB, silver micro-USB connector, metallic RF shielding can labeled ESP-WROOM-32, status SMD LEDs).
- **Driver Cockpit HUD**: 0.96-inch monochrome blue-on-black or white-on-black SSD1306 OLED display mounted on the vehicle cab roof or dashboard, showing an illuminated vector top-down truck graphic with animated radar wave arcs.
- **Flank Presence Sensors**: Dual TCRT5000 / LM393 infrared optical sensor modules integrated cleanly into the left and right wheel arches or side-skirt panels, angled outward at 90 degrees with precision 3D-printed black ABS bezels.
- **Acoustic Indicator**: Compact cylindrical black plastic piezo buzzer module mounted flush on the rear cargo bulkboard.
- **Wiring Harness**: Industrial silicone-insulated 28 AWG wire looms bundled with black braided spiral sleeves and heat-shrink tubing, routed neatly along chassis rail clips rather than loose breadboard jumpers.

---

## 2. Realistic 3D Product Renders

### Prompt 1.1: Isometric Three-Quarter Hero Studio Shot (Reference-Guided)
> **Prompt**: Studio product photography of the finalized BlindSpottr embedded hardware platform integrated onto the scale-model toy truck from the provided reference image. Isometric high three-quarter angle view displaying both the front cab and the right side flank. The truck features a matte-finish utilitarian chassis with the ESP32 microcontroller and custom soldered green FR-4 perfboard seated on the rear flatbed in an aluminum-bracketed mounting tray. Integrated into the right lower side skirt is a black ABS-shrouded TCRT5000 infrared sensor with visible twin optical lenses (clear IR LED and dark phototransistor) aimed directly outward. On top of the truck cab, a 0.96-inch SSD1306 monochrome OLED screen is active, glowing with sharp white vector line graphics depicting a top-down truck wireframe flanked by active curved radar wave arcs. All internal wiring consists of braided black silicone wire sleeves secured neatly along the chassis rails with micro zip-ties. Shot on a neutral grey matte studio workbench surface, softbox key lighting from top-left, gentle rim light highlighting the truck contours, 85mm prime lens, f/8 aperture, ISO 100, 1/125s shutter speed, ultra-crisp depth of field, photorealistic, industrial product design showcase, 8k resolution.
> **Negative Prompt**: Breadboard, loose jumper wires, rat nest cables, blurred text, plastic flash, fingerprints, scratches, distorted wheels, cartoon, low poly, noisy background, overexposed highlights.

### Prompt 1.2: Direct Overhead Planar Top View (Symmetry & Sensor Layout)
> **Prompt**: Direct 90-degree orthogonal top-down flat-lay product photograph of the completed BlindSpottr toy truck prototype from the user reference photograph. Perfectly centered overhead perspective illustrating strict bilateral symmetry. On the cab roof, the 128x64 SSD1306 OLED display is brightly illuminated, presenting crisp vector pixel art of the vehicle with animated chevrons and radar detection waves. Protruding subtly from the exact left and right middle flanks are the dual optical IR sensor assemblies, flush-fitted in custom black matte bezels. Across the rear cargo deck, the black ESP32 DevKit V1 board sits mounted on a custom perfboard with neat gold solder points, labeled silkscreen traces, and a small cylindrical black buzzer module. Wire looms are color-coded (red 3.3V, black GND, blue/yellow signal) and bound in spiral wrap, tracing straight geometric paths to the chassis anchor points. Even diffuse overhead ring lighting, shadowless neutral concrete texture backdrop, 50mm macro lens, f/11 aperture for edge-to-edge sharpness, highly detailed technical portfolio photograph, 8k.
> **Negative Prompt**: Perspective distortion, tilt, angled view, messy wiring, floating components, desaturated screen, glare, blown-out OLED display.

### Prompt 1.3: Low-Angle Dynamic Side Profile (Blind-Spot Sensor Focus)
> **Prompt**: Low-angle ground-level side elevation macro photograph focusing closely on the vehicle's right side blind-spot sensor pod. The toy truck from the reference photo is parked on a simulated asphalt road surface with crisp white lane markings. Embedded seamlessly into the side step-panel between the front and rear wheels is the optical obstacle sensor board, with precision brass standoffs and an indicator SMD LED softly glowing green to indicate active power. In the background, slightly out of focus, the illuminated cab OLED display and the neatly wire-routed ESP32 flatbed carrier are visible. Studio spotlight angled to cast long cinematic ground shadows, 100mm macro lens, f/4 aperture producing a creamy shallow depth of field on the background while keeping the sensor module tack-sharp, subtle reflections on the chassis lacquer, professional industrial engineering demo capture, 8k UHD.
> **Negative Prompt**: Loose loose wires, solder splatter, crooked mounting, blurry lens, oversaturation, dust, warped scale.

### Prompt 1.4: Cockpit & Display Macro Close-Up (Interface Telemetry)
> **Prompt**: Extreme close-up macro studio photograph of the driver cockpit HUD on the model truck. The 0.96-inch SSD1306 monochrome OLED display is the central subject, glowing with high-contrast emissive white pixels against an inky black background. The screen displays the active BlindSpottr telemetry: "BLINDSPOTTR" header, a detailed top-down vehicle icon in the center, animated outward radar warning pulses on the left mirror, and an inverted warning badge reading "[!] L" with "LEFT HAZARD!" along the bottom status bar. Individual OLED pixel grids are subtly visible under close macro inspection. Surrounding the bezel is a textured 3D-printed carbon-fiber-reinforced PETG cowl. Subtle blue ambient fill light, 105mm macro lens, f/5.6 aperture, ISO 100, pin-sharp glass reflection control with polarizing filter, high-tech automotive UI aesthetic.
> **Negative Prompt**: Low resolution, jagged unreadable text, washed-out LCD backlight glow, moiré patterns, screen glare obscuring text, scratches.

---

## 3. Exploded View & Mechanical Assembly

### Prompt 2.1: Isometric Exploded-View Technical Visualization
> **Prompt**: Clean technical exploded-view assembly rendering of the BlindSpottr system installed on the toy truck chassis based on the provided reference parts. The components are suspended vertically in progressive assembly layers above the lower chassis: at the base, the injection-molded truck frame with motor drive and rubber wheels; middle layer showing the two flank TCRT5000 IR sensor modules with their 3D-printed mounting brackets and captive M2 stainless screws; upper cargo deck showing the custom etched perfboard carrier with female socket headers, the plug-in ESP32 DevKit V1 board, and the cylindrical buzzer; top layer showing the truck cab shell with the cutout housing the 0.96-inch OLED screen and protective acrylic window. Thin, precise cyan guide lines connect screw holes and wire routes between the floating layers. Pure white studio background with soft ambient occlusion ground contact shadows, architectural and industrial CAD rendering style, Keyshot render, octane materials, hyper-realistic metallic and PCB textures, 8k.
> **Negative Prompt**: Cluttered lines, dark background, realistic dirt, wire tangled, solid opaque blocking, low-poly CAD mesh errors.

---

## 4. 2D Technical Schematics, Wireframes & Vector Diagrams

### Prompt 3.1: Clean Engineering Blueprint & Circuit Block Diagram
> **Prompt**: A crisp 2D CAD engineering schematic and block diagram of the BlindSpottr hardware ecosystem on a classic dark blueprint grid background with fine cyan grid lines and white vector technical lettering. Left side contains clean electrical block symbols: Left IR Module (labeled DO -> GPIO 34, VCC -> 3.3V, GND -> GND) and Right IR Module (DO -> GPIO 35). Center block displays the ESP32 DevKit V1 38-pin chip with every active GPIO pin clearly indexed and highlighted in bright amber text. Right side illustrates the output blocks: SSD1306 OLED Display (SDA -> GPIO 21, SCL -> GPIO 22) and Active Buzzer (SIG -> GPIO 25 via LEDC PWM). Clean orthogonal right-angled bus connection lines with electrical solder dots, component part callouts, dimension annotations, and title block in the bottom right reading "BLINDSPOTTR EMBEDDED ARCHITECTURE - REV 2.0". Crisp vector linework, high contrast, SVG schematic quality, technical publication standard.
> **Negative Prompt**: Photographic elements, 3D perspective, blurry text, handwriting, watermarks, crossed unlabeled wires.

### Prompt 3.2: 2D Chassis Sensor Coverage & Radar Radiation Cone Diagram
> **Prompt**: Top-down 2D vector technical diagram illustrating the optical and radar coverage zones surrounding the toy truck chassis. Center contains a simplified minimalist wireframe top-down silhouette of the truck. Extending outward from the left door and right door are wide 90-degree sector cones shaded in translucent gradient amber representing the infrared short-range detection envelopes (calibrated at 2cm to 15cm). Extending forward from the front bumper are concentric dashed cyan radar range rings representing optional ultrasonic headway monitoring. Clean dimension arrow markers indicating scale distances, legend box in top corner detailing "IR Blind Zone (Active-LOW)" and "Safe Operating Corridor". White background with minimalist slate-grey and safety-orange technical vector graphics, crisp typography, automotive engineering safety documentation style.
> **Negative Prompt**: 3D perspective, photorealistic textures, cluttered noise, illegible labels, pixelation.

---

## 5. Guidance for User-Supplied Image Inpainting & Transformation

When supplying your physical reference photos to an image-to-image or inpainting tool:

1. **Reference Photo Input**:
   - Provide a well-lit photo of your physical truck with the ESP32, perfboard, sensors, and OLED placed in their rough positions.
2. **Execution Instructions for AI Model**:
   - *"Preserve the core proportions, wheel placement, and chassis structure of the vehicle in the reference image. Replace all provisional loose jumper wires, breadboards, and temporary tape with custom black braided wire looms, soldered green FR-4 perfboard mounting plates, and flush-mounted black sensor bezels matching Prompt 1.1."*
3. **Target Aesthetic**:
   - Convert provisional prototyping artifacts into a finished, bench-tested industrial prototype ready for exhibition.
