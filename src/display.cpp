#include "display.h"

DisplayController::DisplayController()
    : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      _isInitialized(false),
      _lastRefreshMs(0),
      _animPhase(0) {}

bool DisplayController::begin() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    Wire.setClock(400000); // 400kHz fast mode for smooth animations
    Wire.setTimeOut(50);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        _isInitialized = false;
        Serial.println(F("[ERROR] SSD1306 allocation failed!"));
        return false;
    }

    _isInitialized = true;
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);

    // Boot splash screen
    _display.setTextSize(1);
    _display.setCursor(18, 12);
    _display.println(F("BLINDSPOTTR"));
    _display.drawRect(10, 26, 108, 14, SSD1306_WHITE);
    _display.fillRect(12, 28, 104, 10, SSD1306_WHITE);
    _display.setCursor(26, 46);
    _display.println(F("SYSTEM READY"));
    _display.display();
    delay(400);

    return true;
}

bool DisplayController::isAvailable() const {
    return _isInitialized;
}

void DisplayController::update(const SensorData& data, BsmState state, uint32_t now) {
    if (!_isInitialized) return;

    if (now - _lastRefreshMs < OLED_REFRESH_INTERVAL_MS) {
        return;
    }
    _lastRefreshMs = now;
    _animPhase = (_animPhase + 1) % 12;

    renderDashboard(data, state, now);
}

void DisplayController::drawTruckChassis(int cx, int cy) {
    // Top-down view of the toy truck (32px tall, 20px wide)
    // 1. Cab and windshield
    _display.drawRoundRect(cx - 10, cy - 16, 20, 32, 3, SSD1306_WHITE);
    _display.drawRoundRect(cx - 8,  cy - 14, 16, 10, 2, SSD1306_WHITE); // Front windshield

    // 2. Headlights / Front grille
    _display.drawLine(cx - 8, cy - 16, cx - 4, cy - 16, SSD1306_WHITE);
    _display.drawLine(cx + 4, cy - 16, cx + 8, cy - 16, SSD1306_WHITE);

    // 3. Side Mirrors
    _display.fillRect(cx - 13, cy - 12, 3, 4, SSD1306_WHITE);
    _display.fillRect(cx + 10, cy - 12, 3, 4, SSD1306_WHITE);

    // 4. Wheels
    _display.fillRect(cx - 12, cy - 8, 2, 7, SSD1306_WHITE);  // Front Left
    _display.fillRect(cx + 10, cy - 8, 2, 7, SSD1306_WHITE);  // Front Right
    _display.fillRect(cx - 12, cy + 6, 2, 7, SSD1306_WHITE);  // Rear Left
    _display.fillRect(cx + 10, cy + 6, 2, 7, SSD1306_WHITE);  // Rear Right

    // 5. Truck Cargo Bed
    _display.drawRect(cx - 7, cy - 1, 14, 14, SSD1306_WHITE);
}

void DisplayController::drawBlindSpotWaves(bool leftHazard, bool rightHazard, uint8_t animTick) {
    const int cx = 64;
    const int cy = 29;

    // LEFT ZONE WAVES
    if (leftHazard) {
        // Flashing pulsating alert bars on left
        const int offset = (animTick % 4) * 3;
        _display.fillTriangle(cx - 16 - offset, cy, cx - 22 - offset, cy - 8, cx - 22 - offset, cy + 8, SSD1306_WHITE);
        _display.fillRoundRect(2, 14, 16, 32, 2, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(6, 20);
        _display.println(F("!"));
        _display.setCursor(4, 32);
        _display.println(F("L"));
        _display.setTextColor(SSD1306_WHITE);
    } else {
        // Clear zone: subtle dotted road guide
        _display.drawPixel(cx - 18, cy - 10, SSD1306_WHITE);
        _display.drawPixel(cx - 22, cy,      SSD1306_WHITE);
        _display.drawPixel(cx - 18, cy + 10, SSD1306_WHITE);
    }

    // RIGHT ZONE WAVES
    if (rightHazard) {
        // Flashing pulsating alert bars on right
        const int offset = (animTick % 4) * 3;
        _display.fillTriangle(cx + 16 + offset, cy, cx + 22 + offset, cy - 8, cx + 22 + offset, cy + 8, SSD1306_WHITE);
        _display.fillRoundRect(110, 14, 16, 32, 2, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(115, 20);
        _display.println(F("!"));
        _display.setCursor(114, 32);
        _display.println(F("R"));
        _display.setTextColor(SSD1306_WHITE);
    } else {
        // Clear zone: subtle dotted road guide
        _display.drawPixel(cx + 18, cy - 10, SSD1306_WHITE);
        _display.drawPixel(cx + 22, cy,      SSD1306_WHITE);
        _display.drawPixel(cx + 18, cy + 10, SSD1306_WHITE);
    }
}

void DisplayController::drawWarningChevrons(bool leftHazard, bool rightHazard, uint8_t animTick) {
    // Top banner dynamic chevrons
    const int step = animTick % 6;

    if (leftHazard && !rightHazard) {
        // Animated left arrows: <<<
        for (int i = 0; i < 3; i++) {
            const int x = 32 - (i * 8) - step;
            if (x > 4) {
                _display.drawLine(x + 4, 1, x, 4, SSD1306_WHITE);
                _display.drawLine(x, 4, x + 4, 7, SSD1306_WHITE);
            }
        }
    } else if (rightHazard && !leftHazard) {
        // Animated right arrows: >>>
        for (int i = 0; i < 3; i++) {
            const int x = 96 + (i * 8) + step;
            if (x < 124) {
                _display.drawLine(x, 1, x + 4, 4, SSD1306_WHITE);
                _display.drawLine(x + 4, 4, x, 7, SSD1306_WHITE);
            }
        }
    }
}

void DisplayController::renderDashboard(const SensorData& data, BsmState state, uint32_t now) {
    _display.clearDisplay();

    // 1. TOP STATUS HEADER
    _display.setTextSize(1);
    _display.setTextColor(SSD1306_WHITE);
    _display.setCursor(34, 0);
    _display.print(F("BLINDSPOTTR"));
    _display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    // Dynamic warning arrows in header
    drawWarningChevrons(data.leftObstacle, data.rightObstacle, _animPhase);

    // 2. CENTER GRAPHIC: TRUCK & BLIND SPOT ZONES
    drawTruckChassis(64, 29);
    drawBlindSpotWaves(data.leftObstacle, data.rightObstacle, _animPhase);

    // 3. BOTTOM BANNER
    _display.drawLine(0, 50, 127, 50, SSD1306_WHITE);

    if (state == BsmState::SAFE_CLEAR) {
        _display.setCursor(26, 54);
        _display.print(F(">>> ROAD CLEAR <<<"));
    } else if (state == BsmState::WARN_LEFT) {
        _display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(12, 54);
        _display.print(F("<< LEFT HAZARD! <<"));
    } else if (state == BsmState::WARN_RIGHT) {
        _display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(12, 54);
        _display.print(F(">> RIGHT HAZARD! >>"));
    } else if (state == BsmState::WARN_BOTH) {
        // High visibility alternating inverted alert box
        const bool invert = ((now / 150) % 2 == 0);
        if (invert) {
            _display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
            _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        } else {
            _display.setTextColor(SSD1306_WHITE);
        }
        _display.setCursor(8, 54);
        _display.print(F("!! DUAL SIDE WARN !!"));
    }

    _display.display();
}
