#include "tasks/EncoderTask.h"
#include "tasks/WatchdogTask.h"
#include "drivers/EncoderDriver.h"
#include "managers/QueueManager.h"
#include "config/robot_config.h"

void EncoderTask::run(void* pvParameters) {
    EncoderDriver& encoder = EncoderDriver::getInstance();
    encoder.init();

    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(10); // 10ms period

    int32_t lastTicksL = 0;
    int32_t lastTicksR = 0;
    uint32_t lastTimeMs = millis();

    while (true) {
        int32_t ticksL = 0;
        int32_t ticksR = 0;
        encoder.getTicks(ticksL, ticksR);

        uint32_t nowMs = millis();
        float dt = static_cast<float>(nowMs - lastTimeMs) / 1000.0f;
        if (dt <= 0.0f) dt = 0.01f; // Avoid division by zero

        // Calculate tick differences
        int32_t dTicksL = ticksL - lastTicksL;
        int32_t dTicksR = ticksR - lastTicksR;

        lastTicksL = ticksL;
        lastTicksR = ticksR;
        lastTimeMs = nowMs;

        // Calculate RPM: (pulses / PPR) / minutes
        float rpmL = (static_cast<float>(dTicksL) / ENCODER_PULSES_PER_REV) * (60.0f / dt);
        float rpmR = (static_cast<float>(dTicksR) / ENCODER_PULSES_PER_REV) * (60.0f / dt);

        // Calculate speed (m/s): rpm * circumference / 60
        float speedL = rpmL * WHEEL_CIRCUMFERENCE_M / 60.0f;
        float speedR = rpmR * WHEEL_CIRCUMFERENCE_M / 60.0f;

        // Publish odometry data
        EncoderData data;
        data.leftRPM = rpmL;
        data.rightRPM = rpmR;
        data.leftSpeed = speedL;
        data.rightSpeed = speedR;
        data.leftTicks = ticksL;
        data.rightTicks = ticksR;

        QueueManager::getInstance().sendEncoderData(data);

        // Notify watchdog
        WatchdogTask::feed(ENCODER_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
