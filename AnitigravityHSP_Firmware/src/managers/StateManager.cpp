#include "managers/StateManager.h"
#include <string.h>

StateManager& StateManager::getInstance() {
    static StateManager instance;
    return instance;
}

StateManager::StateManager() {
    m_mutex = xSemaphoreCreateMutex();
    m_i2cMutex = xSemaphoreCreateMutex();
}

void StateManager::init() {
    if (m_mutex == nullptr) {
        m_mutex = xSemaphoreCreateMutex();
    }
    if (m_i2cMutex == nullptr) {
        m_i2cMutex = xSemaphoreCreateMutex();
    }
}

SemaphoreHandle_t StateManager::getI2CMutex() {
    return m_i2cMutex;
}

RobotState StateManager::getState() {
    RobotState current = RobotState::IDLE;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        current = m_state;
        xSemaphoreGive(m_mutex);
    }
    return current;
}

void StateManager::setState(RobotState state) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_state = state;
        xSemaphoreGive(m_mutex);
    }
}

void StateManager::getCurrentStation(char* dest, size_t maxLen) {
    if (dest == nullptr || maxLen == 0) return;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        strncpy(dest, m_currentStation, maxLen - 1);
        dest[maxLen - 1] = '\0';
        xSemaphoreGive(m_mutex);
    }
}

void StateManager::setCurrentStation(const char* station) {
    if (station == nullptr) return;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        strncpy(m_currentStation, station, sizeof(m_currentStation) - 1);
        m_currentStation[sizeof(m_currentStation) - 1] = '\0';
        xSemaphoreGive(m_mutex);
    }
}

bool StateManager::isWifiConnected() {
    bool connected = false;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        connected = m_wifiConnected;
        xSemaphoreGive(m_mutex);
    }
    return connected;
}

void StateManager::setWifiConnected(bool connected) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_wifiConnected = connected;
        xSemaphoreGive(m_mutex);
    }
}

bool StateManager::isServerConnected() {
    bool connected = false;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        connected = m_serverConnected;
        xSemaphoreGive(m_mutex);
    }
    return connected;
}

void StateManager::setServerConnected(bool connected) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_serverConnected = connected;
        xSemaphoreGive(m_mutex);
    }
}

float StateManager::getBatteryPercent() {
    float pct = 0.0f;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        pct = m_batteryPercent;
        xSemaphoreGive(m_mutex);
    }
    return pct;
}

void StateManager::setBatteryPercent(float pct) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_batteryPercent = pct;
        xSemaphoreGive(m_mutex);
    }
}

float StateManager::getBatteryVolts() {
    float volts = 0.0f;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        volts = m_batteryVolts;
        xSemaphoreGive(m_mutex);
    }
    return volts;
}

void StateManager::setBatteryVolts(float volts) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_batteryVolts = volts;
        xSemaphoreGive(m_mutex);
    }
}

uint16_t StateManager::getObstacleDistance() {
    uint16_t mm = 9999;
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        mm = m_obstacleDistance;
        xSemaphoreGive(m_mutex);
    }
    return mm;
}

void StateManager::setObstacleDistance(uint16_t mm) {
    if (xSemaphoreTake(m_mutex, portMAX_DELAY) == pdTRUE) {
        m_obstacleDistance = mm;
        xSemaphoreGive(m_mutex);
    }
}
