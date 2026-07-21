#ifndef DRIVERS_MPU6050_DRIVER_H
#define DRIVERS_MPU6050_DRIVER_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

struct IMUData {
    float heading;        // Integrated Yaw angle in degrees
    float gyroZ;          // Angular velocity Z (deg/s)
    float accelX;         // Acceleration X (m/s^2)
    float accelY;         // Acceleration Y (m/s^2)
    float accelZ;         // Acceleration Z (m/s^2)
};

class MPU6050Driver {
public:
    MPU6050Driver() = default;
    ~MPU6050Driver() = default;

    bool init();
    bool readData(IMUData& data, float dtSeconds);
    void resetHeading();

private:
    Adafruit_MPU6050 m_mpu;
    bool m_initialized = false;
    float m_heading = 0.0f;
    float m_gyroBiasZ = 0.0f; // Calibrated gyro bias
};

#endif // DRIVERS_MPU6050_DRIVER_H
