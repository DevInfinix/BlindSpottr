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
    _display.setCursor(20, 15);
    _display.println(F("BLINDSPOTTR"));
    _display.drawRoundRect(10, 28, 108, 12, 3, SSD1306_WHITE);
    _display.fillRoundRect(12, 30, 104, 8, 2, SSD1306_WHITE);
    _display.setCursor(26, 45);
    _display.println(F("SYSTEM READY"));
    _display.display();
    delay(250);

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
    _animPhase = (_animPhase + 1) % 16;

    renderDashboard(data, state, now);
}

void DisplayController::drawTruckChassis(int cx, int cy) {
    // 1. Truck Body & Cab (20px wide, 32px tall)
    _display.drawRoundRect(cx - 10, cy - 16, 20, 32, 3, SSD1306_WHITE);
    _display.drawRoundRect(cx - 8,  cy - 14, 16, 10, 2, SSD1306_WHITE); // Front windshield

    // 2. Headlights / Grille
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

    // 5. Cargo Bed with cross-hatch detail
    _display.drawRect(cx - 7, cy - 1, 14, 14, SSD1306_WHITE);
    _display.drawPixel(cx, cy + 6, SSD1306_WHITE);
}

void DisplayController::drawBlindSpotWaves(bool leftHazard, bool rightHazard, uint8_t animTick) {
    const int cx = 64;
    const int cy = 29;

    // LEFT ZONE (Physically on the LEFT side of the display: x < cx)
    if (leftHazard) {
        // Multi-tier animated radar pulse expanding to the LEFT
        const int waveStep = (animTick % 4);
        for (int w = 0; w <= waveStep; w++) {
            const int radius = 8 + (w * 5);
            const int startX = cx - 12 - radius;
            if (startX >= 20) {
                _display.drawFastVLine(startX, cy - radius / 2, radius, SSD1306_WHITE);
            }
        }
        _display.fillTriangle(cx - 16, cy, cx - 22, cy - 6, cx - 22, cy + 6, SSD1306_WHITE);

        // Inverted Left Alert Badge explicitly labeled "L"
        _display.fillRoundRect(2, 14, 16, 32, 2, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(6, 20);
        _display.println(F("!"));
        _display.setCursor(6, 32);
        _display.println(F("L"));
        _display.setTextColor(SSD1306_WHITE);
    } else {
        // Clear: animated road line markers flowing backwards (forward motion illusion)
        const int dashOffset = (animTick % 8);
        for (int y = 12 + dashOffset; y < 48; y += 8) {
            _display.drawPixel(cx - 20, y, SSD1306_WHITE);
        }
    }

    // RIGHT ZONE (Physically on the RIGHT side of the display: x > cx)
    if (rightHazard) {
        // Multi-tier animated radar pulse expanding to the RIGHT
        const int waveStep = (animTick % 4);
        for (int w = 0; w <= waveStep; w++) {
            const int radius = 8 + (w * 5);
            const int startX = cx + 12 + radius;
            if (startX <= 108) {
                _display.drawFastVLine(startX, cy - radius / 2, radius, SSD1306_WHITE);
            }
        }
        _display.fillTriangle(cx + 16, cy, cx + 22, cy - 6, cx + 22, cy + 6, SSD1306_WHITE);

        // Inverted Right Alert Badge explicitly labeled "R"
        _display.fillRoundRect(110, 14, 16, 32, 2, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(115, 20);
        _display.println(F("!"));
        _display.setCursor(115, 32);
        _display.println(F("R"));
        _display.setTextColor(SSD1306_WHITE);
    } else {
        // Clear: animated road line markers flowing backwards
        const int dashOffset = (animTick % 8);
        for (int y = 12 + dashOffset; y < 48; y += 8) {
            _display.drawPixel(cx + 20, y, SSD1306_WHITE);
        }
    }
}

void DisplayController::drawWarningChevrons(bool leftHazard, bool rightHazard, uint8_t animTick) {
    const int step = animTick % 6;

    if (leftHazard && !rightHazard) {
        for (int i = 0; i < 3; i++) {
            const int x = 32 - (i * 8) - step;
            if (x > 4) {
                _display.drawLine(x + 4, 1, x, 4, SSD1306_WHITE);
                _display.drawLine(x, 4, x + 4, 7, SSD1306_WHITE);
            }
        }
    } else if (rightHazard && !leftHazard) {
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

    // 2. FORWARD SCANNING RADAR ARCS & DISTANCE HUD
    const int cx = 64;
    const int cy = 29;

    if (data.frontValid) {
        // Render live distance readout centered above the truck cab
        _display.setCursor(cx - 16, 11);
        _display.print((int)data.frontDistanceCm);
        _display.print(F("cm"));

        if (data.frontObstacle) {
            // Flashing proximity warning bar right in front of bumper
            _display.fillRect(cx - 10, cy - 18, 20, 2, SSD1306_WHITE);
        }
    } else {
        // Continuous sweeping radar beam emitting ahead of the truck
        const int frontTick = (_animPhase % 5);
        for (int r = 1; r <= 3; r++) {
            const int waveY = (cy - 16) - ((frontTick + r * 4) % 12);
            if (waveY >= 10 && waveY <= cy - 16) {
                _display.drawLine(cx - 6, waveY, cx + 6, waveY, SSD1306_WHITE);
            }
        }
    }

    // 3. CENTER GRAPHIC: TOY TRUCK & BLIND SPOT ZONES
    // Clean, direct 1:1 mapping: data.leftObstacle draws on left with 'L', data.rightObstacle on right with 'R'
    drawTruckChassis(cx, cy);
    drawBlindSpotWaves(data.leftObstacle, data.rightObstacle, _animPhase);

    // 4. BOTTOM DYNAMIC STATUS BANNER
    _display.drawLine(0, 50, 127, 50, SSD1306_WHITE);

    if (state == BsmState::SAFE_CLEAR) {
        _display.setCursor(24, 54);
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
    } else if (state == BsmState::WARN_FRONT) {
        _display.fillRect(0, 52, 128, 12, SSD1306_WHITE);
        _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        _display.setCursor(10, 54);
        _display.print(F("^^ FRONT HAZARD! ^^"));
    } else if (state == BsmState::WARN_BOTH || state == BsmState::WARN_MULTI) {
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
