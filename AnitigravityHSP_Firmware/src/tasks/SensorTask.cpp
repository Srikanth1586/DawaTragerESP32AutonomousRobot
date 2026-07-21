#include "tasks/SensorTask.h"
#include "tasks/WatchdogTask.h"
#include "config/pins.h"
#include "config/robot_config.h"
#include "drivers/VL53L0XDriver.h"
#include "drivers/MPU6050Driver.h"
#include "drivers/BatteryDriver.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"

void SensorTask::run(void* pvParameters) {
    // Instantiate drivers
    VL53L0XDriver tof;
    MPU6050Driver imu;
    BatteryDriver battery;

    // Initialize line sensor pins
    pinMode(PIN_LINE_L, INPUT);
    pinMode(PIN_LINE_R, INPUT);

    // Initialize hardware drivers
    bool tofOk = tof.init();
    bool imuOk = imu.init();
    battery.init();

    // Notify watchdog that task is running
    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(20); // 20ms period

    IMUData imuData = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float lastTime = millis() / 1000.0f;

    while (true) {
        float now = millis() / 1000.0f;
        float dt = now - lastTime;
        if (dt <= 0.0f) dt = 0.02f;
        lastTime = now;

        SensorData data;

        // 1. Read Line Sensors (True if black line detected, False if white)
        // Adjust logic based on sensor type (active high vs active low)
        data.leftLineDetected = (digitalRead(PIN_LINE_L) == HIGH);
        data.rightLineDetected = (digitalRead(PIN_LINE_R) == HIGH);

        // 2. Read ToF Sensor
        if (tofOk) {
            data.obstacleDistanceMm = tof.readDistanceContinuous();
        } else {
            data.obstacleDistanceMm = 9999;
        }

        // 3. Read IMU
        if (imuOk && imu.readData(imuData, dt)) {
            data.imuHeading = imuData.heading;
            data.imuAngularVelocityZ = imuData.gyroZ;
        } else {
            data.imuHeading = 0.0f;
            data.imuAngularVelocityZ = 0.0f;
        }

        // 4. Read Battery (Optional but implemented)
        battery.readBattery(data.batteryVoltage, data.batteryPercentage);

        // Update StateManager with battery and obstacle details for global access
        StateManager::getInstance().setBatteryPercent(data.batteryPercentage);
        StateManager::getInstance().setBatteryVolts(data.batteryVoltage);
        StateManager::getInstance().setObstacleDistance(data.obstacleDistanceMm);

        // Check for Low Battery event
        if (data.batteryPercentage < BATTERY_LOW_THRESHOLD_PCT) {
            EventManager::getInstance().setBits(EventManager::LOW_BATTERY);
        } else {
            EventManager::getInstance().clearBits(EventManager::LOW_BATTERY);
        }

        // 5. Publish to SensorQueue
        QueueManager::getInstance().sendSensorData(data);

        // Notify watchdog
        WatchdogTask::feed(SENSOR_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
