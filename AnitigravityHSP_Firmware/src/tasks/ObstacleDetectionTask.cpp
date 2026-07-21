#include "tasks/ObstacleDetectionTask.h"
#include "tasks/WatchdogTask.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"
#include "managers/QueueManager.h"
#include "config/robot_config.h"

void ObstacleDetectionTask::run(void* pvParameters) {
    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(50); // 50ms period

    while (true) {
        // Read distance updated by SensorTask
        uint16_t dist = StateManager::getInstance().getObstacleDistance();

        if (dist < OBSTACLE_DISTANCE_THRESHOLD_MM) {
            // Set obstacle event bit
            EventManager::getInstance().setBits(EventManager::OBSTACLE_PRESENT);
            
            // Send obstacle warning to navigation queue for state machine transition
            NavigationCommand navCmd = {RobotState::OBSTACLE_AVOIDANCE, "Obstacle", false, 0, 0};
            QueueManager::getInstance().sendNavigationCommand(navCmd);
        } else {
            // Clear obstacle event bit if path is clear
            EventManager::getInstance().clearBits(EventManager::OBSTACLE_PRESENT);
        }

        // Notify watchdog
        WatchdogTask::feed(OBSTACLE_DETECTION_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
