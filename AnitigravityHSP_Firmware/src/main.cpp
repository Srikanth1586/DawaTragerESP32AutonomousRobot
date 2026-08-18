#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Configs
#include "config/pins.h"
#include "config/robot_config.h"

// Managers
#include "managers/QueueManager.h"
#include "managers/EventManager.h"
#include "managers/StateManager.h"

// Tasks
#include "tasks/SensorTask.h"
#include "tasks/EncoderTask.h"
#include "tasks/LineFollowerTask.h"
#include "tasks/ObstacleDetectionTask.h"
#include "tasks/NavigationTask.h"
#include "tasks/NFCTask.h"
#include "tasks/MotorTask.h"
#include "tasks/OLEDTask.h"
#include "tasks/ButtonTask.h"
#include "tasks/CommTask.h"
#include "tasks/HeartbeatTask.h"
#include "tasks/WatchdogTask.h"

// Task Handles (for WatchdogTask to track or for notifications)
TaskHandle_t sensorTaskHandle = nullptr;
TaskHandle_t encoderTaskHandle = nullptr;
TaskHandle_t lineFollowTaskHandle = nullptr;
TaskHandle_t obstacleTaskHandle = nullptr;
TaskHandle_t navTaskHandle = nullptr;
TaskHandle_t nfcTaskHandle = nullptr;
TaskHandle_t motorTaskHandle = nullptr;
TaskHandle_t oledTaskHandle = nullptr;
TaskHandle_t buttonTaskHandle = nullptr;
TaskHandle_t commTaskHandle = nullptr;
TaskHandle_t heartbeatTaskHandle = nullptr;
TaskHandle_t watchdogTaskHandle = nullptr;

void setup() {
    // 1. Initialize serial debugging
    Serial.begin(115200);
    while (!Serial && millis() < 1000) {
        // Wait for serial interface
    }
    Serial.println("\n==============================================");
    Serial.println("ESP32 Hospital Delivery Robot Firmware Starting");
    Serial.println("==============================================");

    // 2. Initialize Shared Buses
    // Initialize I2C Bus with custom pins
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000); // 400 kHz Fast Mode
    Serial.println("[SYSTEM] I2C bus initialized.");

    // Initialize SPI Bus (Default VSPI: SCLK=18, MISO=19, MOSI=23, SS=5)
    SPI.begin(PIN_NFC_SCK, PIN_NFC_MISO, PIN_NFC_MOSI, PIN_NFC_SS);
    Serial.println("[SYSTEM] SPI bus initialized.");

    // 3. Initialize Singleton Managers
    EventManager::getInstance().init();
    QueueManager::getInstance().init();
    StateManager::getInstance().init();
    Serial.println("[SYSTEM] Singleton Managers initialized.");

    // Set initial station name and battery percents
    StateManager::getInstance().setCurrentStation("None");
    StateManager::getInstance().setBatteryPercent(100.0f);
    StateManager::getInstance().setBatteryVolts(12.6f);

    // 4. Spawn FreeRTOS Tasks
    // Core 0: Network and Communication Tasks (isolated from real-time motion)
    // Core 1: Physical Motion, Sensors, and PID Controllers (strict timing)

    Serial.println("[SYSTEM] Spawning FreeRTOS Tasks...");

    // Watchdog Task: Priority 6 (Highest), Core 1
    xTaskCreatePinnedToCore(
        WatchdogTask::run,
        "WatchdogTask",
        3072,
        nullptr,
        6,
        &watchdogTaskHandle,
        1
    );

    // Motor Control Task: Priority 5 (Highest control), Core 1
    xTaskCreatePinnedToCore(
        MotorTask::run,
        "MotorTask",
        3072,
        nullptr,
        5,
        &motorTaskHandle,
        1
    );

    // Sensor Read Task: Priority 4 (High), Core 1
    xTaskCreatePinnedToCore(
        SensorTask::run,
        "SensorTask",
        4096,
        nullptr,
        4,
        &sensorTaskHandle,
        1
    );

    // Encoder Count/Odometry Task: Priority 4 (High), Core 1
    xTaskCreatePinnedToCore(
        EncoderTask::run,
        "EncoderTask",
        2048,
        nullptr,
        4,
        &encoderTaskHandle,
        1
    );

    // Line Follower PID Loop Task: Priority 4 (High), Core 1
    xTaskCreatePinnedToCore(
        LineFollowerTask::run,
        "LineFollowerTask",
        3072,
        nullptr,
        4,
        &lineFollowTaskHandle,
        1
    );

    // Obstacle Detection Task: Priority 4 (High), Core 1
    xTaskCreatePinnedToCore(
        ObstacleDetectionTask::run,
        "ObstacleTask",
        2048,
        nullptr,
        4,
        &obstacleTaskHandle,
        1
    );

    // Navigation State Machine Task: Priority 3 (Medium), Core 1
    xTaskCreatePinnedToCore(
        NavigationTask::run,
        "NavTask",
        3072,
        nullptr,
        3,
        &navTaskHandle,
        1
    );

    // Button User Interface Task: Priority 3 (Medium), Core 1
    xTaskCreatePinnedToCore(
        ButtonTask::run,
        "ButtonTask",
        2048,
        nullptr,
        3,
        &buttonTaskHandle,
        1
    );

    // PN532 NFC Tag Read Task: Priority 3 (Medium), Core 1
    xTaskCreatePinnedToCore(
        NFCTask::run,
        "NFCTask",
        4096,
        nullptr,
        3,
        &nfcTaskHandle,
        1
    );

    // Communication WebSocket Task: Priority 3 (Medium), Core 0 (Network)
    xTaskCreatePinnedToCore(
        CommTask::run,
        "CommTask",
        8192,
        nullptr,
        3,
        &commTaskHandle,
        0
    );

    // OLED GUI Task: Priority 1 (Low), Core 1
    xTaskCreatePinnedToCore(
        OLEDTask::run,
        "OLEDTask",
        4096,
        nullptr,
        1,
        &oledTaskHandle,
        1
    );

    // Server Heartbeat Task: Priority 1 (Low), Core 0 (Network)
    xTaskCreatePinnedToCore(
        HeartbeatTask::run,
        "HeartbeatTask",
        4096,
        nullptr,
        1,
        &heartbeatTaskHandle,
        0
    );

    Serial.println("[SYSTEM] Setup completed successfully.");
}

void loop() {
    // FreeRTOS is handling execution. The Arduino loop task is deleted or put to sleep.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
