#include "tof_driver.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox;

bool tofInit()
{
    Wire.begin(I2C_SDA, I2C_SCL);

    if (!lox.begin())
    {
        Serial.println("VL53L0X NOT FOUND");
        return false;
    }

    Serial.println("VL53L0X READY");

    return true;
}

uint16_t readDistance()
{
    VL53L0X_RangingMeasurementData_t measure;

    lox.rangingTest(
        &measure,
        false);

    if (measure.RangeStatus != 4)
    {
        return measure.RangeMilliMeter;
    }

    return 8190;
}