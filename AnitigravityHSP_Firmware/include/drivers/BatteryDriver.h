#ifndef DRIVERS_BATTERY_DRIVER_H
#define DRIVERS_BATTERY_DRIVER_H

#include <Arduino.h>

class BatteryDriver {
public:
    BatteryDriver() = default;
    ~BatteryDriver() = default;

    void init();
    void readBattery(float& voltage, float& percentage);
};

#endif // DRIVERS_BATTERY_DRIVER_H
