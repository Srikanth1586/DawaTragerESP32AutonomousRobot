#include "drivers/BatteryDriver.h"
#include "config/pins.h"
#include "config/robot_config.h"

void BatteryDriver::init() {
    pinMode(PIN_BATTERY_ADC, INPUT);
    // Configure ESP32 ADC resolution and attenuation
    analogReadResolution(12); // 12-bit (0 - 4095)
    // analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db); // Standard for 3.3V full-scale
}

void BatteryDriver::readBattery(float& voltage, float& percentage) {
    // Read multiple times and average to reduce noise
    uint32_t rawSum = 0;
    constexpr int numSamples = 10;
    
    for (int i = 0; i < numSamples; i++) {
        rawSum += analogRead(PIN_BATTERY_ADC);
        delayMicroseconds(100);
    }
    
    float rawAverage = static_cast<float>(rawSum) / static_cast<float>(numSamples);

    // Calculate voltage on pin (ADC Reference = 3.3V)
    float pinVolts = (rawAverage / 4095.0f) * BATTERY_ADC_REF_V;

    // Calculate actual battery voltage (after resistor divider)
    voltage = pinVolts * BATTERY_DIVIDER_RATIO;

    // Calculate battery percentage (linear interpolation)
    if (voltage >= BATTERY_MAX_V) {
        percentage = 100.0f;
    } else if (voltage <= BATTERY_MIN_V) {
        percentage = 0.0f;
    } else {
        percentage = ((voltage - BATTERY_MIN_V) / (BATTERY_MAX_V - BATTERY_MIN_V)) * 100.0f;
    }
}
