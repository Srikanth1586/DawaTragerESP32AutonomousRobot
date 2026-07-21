#include "drivers/VL53L0XDriver.h"
#include "managers/StateManager.h"
#include "config/pins.h"
#include <Wire.h>

bool VL53L0XDriver::init() {
    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();
    bool success = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Initialize sensor using Adafruit library
        // VL53L0X_I2C_ADDRESS is 0x29
        success = m_vl53.begin(VL53L0X_I2C_ADDRESS, false, &Wire);
        xSemaphoreGive(i2cMutex);
    }

    m_initialized = success;
    return success;
}

uint16_t VL53L0XDriver::readDistanceContinuous() {
    if (!m_initialized) {
        return 9999;
    }

    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();
    uint16_t distance = 9999;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        VL53L0X_RangingMeasurementData_t measure;
        m_vl53.rangingTest(&measure, false); // false for debug print flag

        // Phase status 0 means valid measurement
        if (measure.RangeStatus != 4) {
            distance = measure.RangeMilliMeter;
        }
        xSemaphoreGive(i2cMutex);
    }

    return distance;
}
