#ifndef DRIVERS_OLED_DRIVER_H
#define DRIVERS_OLED_DRIVER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "managers/QueueManager.h" // For DisplayData struct

class OLEDDriver {
public:
    OLEDDriver();
    ~OLEDDriver() = default;

    bool init();
    void updateScreen(const DisplayData& data);
    void showAlert(const char* title, const char* message);

private:
    Adafruit_SSD1306 m_display;
    bool m_initialized = false;
};

#endif // DRIVERS_OLED_DRIVER_H
