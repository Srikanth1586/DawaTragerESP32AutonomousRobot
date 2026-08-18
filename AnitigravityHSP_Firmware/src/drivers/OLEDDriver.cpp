#include "drivers/OLEDDriver.h"
#include "managers/StateManager.h"
#include <Wire.h>

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr int8_t OLED_RESET = -1;
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

OLEDDriver::OLEDDriver() : m_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {}

bool OLEDDriver::init() {
    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();
    bool success = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        success = m_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
        if (success) {
            m_display.clearDisplay();
            m_display.setTextColor(SSD1306_WHITE);
            m_display.setTextSize(1);
            m_display.setCursor(0, 0);
            m_display.print("HSP Robot Booting...");
            m_display.display();
        }
        xSemaphoreGive(i2cMutex);
    }

    m_initialized = success;
    return success;
}

void OLEDDriver::updateScreen(const DisplayData& data) {
    if (!m_initialized) return;

    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        m_display.clearDisplay();

        // 1. Draw Status/Header Bar
        m_display.setTextSize(1);
        m_display.setCursor(0, 0);
        m_display.printf("ID:%s", data.robotID);

        // WiFi Status
        m_display.setCursor(65, 0);
        m_display.printf("W:%s", data.wifiConnected ? "ON" : "..");

        // Server Status
        m_display.setCursor(90, 0);
        m_display.printf("S:%s", data.serverConnected ? "ON" : "..");

        // Draw line under header
        m_display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

        // 2. State Information
        m_display.setTextSize(1);
        m_display.setCursor(0, 16);
        m_display.print("STATE:");
        m_display.setCursor(45, 16);
        m_display.print(robotStateToString(data.currentState));

        // 3. Station Info
        m_display.setCursor(0, 28);
        m_display.print("STN  :");
        m_display.setCursor(45, 28);
        m_display.print(data.currentStation);

        // 4. Battery voltage info
        m_display.setCursor(0, 40);
        m_display.printf("BATT :");
        m_display.setCursor(45, 40);
        m_display.printf("%.0f%%", data.batteryPercentage);

        // 5. Obstacle Distance
        m_display.setCursor(0, 52);
        if (data.obstacleAlert) {
            m_display.printf("!!! OBSTACLE: %dmm !!!", data.obstacleDistanceMm);
        } else {
            m_display.printf("ToF  : %d mm", data.obstacleDistanceMm);
        }

        m_display.display();
        xSemaphoreGive(i2cMutex);
    }
}

void OLEDDriver::showAlert(const char* title, const char* message) {
    if (!m_initialized) return;

    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        m_display.clearDisplay();

        m_display.setTextSize(2);
        m_display.setCursor(10, 10);
        m_display.print(title);

        m_display.setTextSize(1);
        m_display.setCursor(10, 35);
        m_display.print(message);

        // Simple decorative border
        m_display.drawRect(0, 0, 128, 64, SSD1306_WHITE);

        m_display.display();
        xSemaphoreGive(i2cMutex);
    }
}
