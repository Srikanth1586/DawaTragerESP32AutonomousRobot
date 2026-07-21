#include "drivers/MPU6050Driver.h"
#include "managers/StateManager.h"
#include <Wire.h>

bool MPU6050Driver::init() {
    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();
    bool success = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        success = m_mpu.begin(0x68, &Wire); // 0x68 is default MPU6050 address
        if (success) {
            m_mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
            m_mpu.setGyroRange(MPU6050_RANGE_250_DEG);
            m_mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        }
        xSemaphoreGive(i2cMutex);
    }

    if (!success) {
        m_initialized = false;
        return false;
    }

    m_initialized = true;
    m_heading = 0.0f;

    // Gyro calibration (sample 100 times to find Z-gyro bias)
    float sumZ = 0.0f;
    int samples = 50;
    int validSamples = 0;
    
    for (int i = 0; i < samples; i++) {
        sensors_event_t a, g, temp;
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
            m_mpu.getEvent(&a, &g, &temp);
            sumZ += g.gyro.z;
            validSamples++;
            xSemaphoreGive(i2cMutex);
        }
        delay(10);
    }
    
    if (validSamples > 0) {
        m_gyroBiasZ = sumZ / static_cast<float>(validSamples);
    } else {
        m_gyroBiasZ = 0.0f;
    }

    return true;
}

bool MPU6050Driver::readData(IMUData& data, float dtSeconds) {
    if (!m_initialized) return false;

    SemaphoreHandle_t i2cMutex = StateManager::getInstance().getI2CMutex();
    sensors_event_t a, g, temp;
    bool success = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        success = m_mpu.getEvent(&a, &g, &temp);
        xSemaphoreGive(i2cMutex);
    }

    if (!success) return false;

    // Convert gyro.z from rad/s to deg/s and remove bias
    float gyroZRad = g.gyro.z - m_gyroBiasZ;
    float gyroZDeg = gyroZRad * (180.0f / 3.14159265f);

    // Integrate gyro Z to get heading (Euler integration)
    m_heading += gyroZDeg * dtSeconds;
    
    // Normalize heading to [0, 360)
    if (m_heading >= 360.0f) {
        m_heading -= 360.0f;
    } else if (m_heading < 0.0f) {
        m_heading += 360.0f;
    }

    data.heading = m_heading;
    data.gyroZ = gyroZDeg;
    data.accelX = a.acceleration.x;
    data.accelY = a.acceleration.y;
    data.accelZ = a.acceleration.z;

    return true;
}

void MPU6050Driver::resetHeading() {
    m_heading = 0.0f;
}
