#ifndef MANAGERS_STATE_MANAGER_H
#define MANAGERS_STATE_MANAGER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "config/robot_config.h"

class StateManager {
public:
    static StateManager& getInstance();

    // Prevent copying
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    void init();

    // Getters and Setters (thread-safe using mutex)
    RobotState getState();
    void setState(RobotState state);

    void getCurrentStation(char* dest, size_t maxLen);
    void setCurrentStation(const char* station);

    bool isWifiConnected();
    void setWifiConnected(bool connected);

    bool isServerConnected();
    void setServerConnected(bool connected);

    float getBatteryPercent();
    void setBatteryPercent(float pct);

    float getBatteryVolts();
    void setBatteryVolts(float volts);

    uint16_t getObstacleDistance();
    void setObstacleDistance(uint16_t mm);

    SemaphoreHandle_t getI2CMutex();

private:
    StateManager();
    ~StateManager() = default;

    SemaphoreHandle_t m_mutex = nullptr;
    SemaphoreHandle_t m_i2cMutex = nullptr;

    RobotState m_state = RobotState::IDLE;
    char m_currentStation[16] = "None";
    bool m_wifiConnected = false;
    bool m_serverConnected = false;
    float m_batteryPercent = 0.0f;
    float m_batteryVolts = 0.0f;
    uint16_t m_obstacleDistance = 9999;
};

#endif // MANAGERS_STATE_MANAGER_H
