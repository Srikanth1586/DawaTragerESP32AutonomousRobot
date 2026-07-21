#ifndef DRIVERS_VL53L0X_DRIVER_H
#define DRIVERS_VL53L0X_DRIVER_H

#include <Arduino.h>
#include <Adafruit_VL53L0X.h>

class VL53L0XDriver {
public:
    VL53L0XDriver() = default;
    ~VL53L0XDriver() = default;

    bool init();
    uint16_t readDistanceContinuous();

private:
    Adafruit_VL53L0X m_vl53;
    bool m_initialized = false;
};

#endif // DRIVERS_VL53L0X_DRIVER_H
