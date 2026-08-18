#ifndef MANAGERS_QUEUE_MANAGER_H
#define MANAGERS_QUEUE_MANAGER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "config/robot_config.h"

// Define strongly typed queue structures
struct SensorData {
    float batteryVoltage;
    float batteryPercentage;
    uint16_t obstacleDistanceMm;
    float imuHeading;
    float imuAngularVelocityZ;
    bool leftLineDetected;
    bool rightLineDetected;
};

struct EncoderData {
    float leftRPM;
    float rightRPM;
    float leftSpeed;  // m/s
    float rightSpeed; // m/s
    int32_t leftTicks;
    int32_t rightTicks;
};

struct MotorCommand {
    int16_t leftTargetSpeed;  // Target speed / PWM value (-255 to 255)
    int16_t rightTargetSpeed; // Target speed / PWM value (-255 to 255)
    bool isEmergencyStop;
};

struct NFCData {
    char stationID[16];
    bool tagDetected;
};

struct NavigationCommand {
    RobotState targetState;
    char nextStation[16];
    bool hasTargetSpeed;
    int16_t leftTargetSpeed;
    int16_t rightTargetSpeed;
};

struct DisplayData {
    float batteryPercentage;
    RobotState currentState;
    char currentStation[16];
    bool wifiConnected;
    bool serverConnected;
    char robotID[16];
    uint16_t obstacleDistanceMm;
    bool obstacleAlert;
};

struct CommunicationMessage {
    char payload[256];
    size_t length;
};

// QueueManager Singleton Class
class QueueManager {
public:
    static QueueManager& getInstance();

    // Prevent copying
    QueueManager(const QueueManager&) = delete;
    QueueManager& operator=(const QueueManager&) = delete;

    void init();

    // Queue Handles accessors
    QueueHandle_t getSensorQueue() const      { return m_sensorQueue; }
    QueueHandle_t getEncoderQueue() const     { return m_encoderQueue; }
    QueueHandle_t getMotorQueue() const       { return m_motorQueue; }
    QueueHandle_t getNFCQueue() const         { return m_nfcQueue; }
    QueueHandle_t getNavigationQueue() const  { return m_navigationQueue; }
    QueueHandle_t getDisplayQueue() const     { return m_displayQueue; }
    QueueHandle_t getCommunicationQueue() const { return m_communicationQueue; }

    // Helpers to send data (Thread-Safe Wrapper)
    bool sendSensorData(const SensorData& data, TickType_t wait = 0);
    bool sendEncoderData(const EncoderData& data, TickType_t wait = 0);
    bool sendMotorCommand(const MotorCommand& data, TickType_t wait = 0);
    bool sendNFCData(const NFCData& data, TickType_t wait = 0);
    bool sendNavigationCommand(const NavigationCommand& data, TickType_t wait = 0);
    bool sendDisplayData(const DisplayData& data, TickType_t wait = 0);
    bool sendCommunicationMessage(const CommunicationMessage& data, TickType_t wait = 0);

    // ISR Helpers
    bool sendMotorCommandFromISR(const MotorCommand& data, BaseType_t* pxHigherPriorityTaskWoken);
    bool sendNavigationCommandFromISR(const NavigationCommand& data, BaseType_t* pxHigherPriorityTaskWoken);

private:
    QueueManager() = default;
    ~QueueManager() = default;

    QueueHandle_t m_sensorQueue = nullptr;
    QueueHandle_t m_encoderQueue = nullptr;
    QueueHandle_t m_motorQueue = nullptr;
    QueueHandle_t m_nfcQueue = nullptr;
    QueueHandle_t m_navigationQueue = nullptr;
    QueueHandle_t m_displayQueue = nullptr;
    QueueHandle_t m_communicationQueue = nullptr;
};

#endif // MANAGERS_QUEUE_MANAGER_H
