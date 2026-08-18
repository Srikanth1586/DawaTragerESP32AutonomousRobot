#include "tasks/LineFollowerTask.h"
#include "tasks/WatchdogTask.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"
#include "config/robot_config.h"

void LineFollowerTask::run(void* pvParameters) {
    float error = 0.0f;
    float lastError = 0.0f;
    float integral = 0.0f;
    float derivative = 0.0f;
    
    constexpr float dt = 0.02f; // 20ms loop interval

    uint32_t lostLineCounter = 0;
    constexpr uint32_t lostLineThresholdCycles = LOST_LINE_TIMEOUT_MS / 20;

    SensorData sensorData;

    while (true) {
        // Block until new sensor data is available (published every 20ms by SensorTask)
        if (xQueueReceive(QueueManager::getInstance().getSensorQueue(), &sensorData, portMAX_DELAY) == pdPASS) {
            
            // Only execute control loop if we are in the LINE_FOLLOWING state
            if (StateManager::getInstance().getState() == RobotState::LINE_FOLLOWING) {
                
                bool left = sensorData.leftLineDetected;
                bool right = sensorData.rightLineDetected;

                // 1. Determine error and update LINE_DETECTED bit
                if (left && right) {
                    // Both on black: forward
                    error = 0.0f;
                    lostLineCounter = 0;
                    EventManager::getInstance().setBits(EventManager::LINE_DETECTED);
                } 
                else if (left && !right) {
                    // Left on black, right on white: turn left
                    error = -1.0f;
                    lostLineCounter = 0;
                    EventManager::getInstance().setBits(EventManager::LINE_DETECTED);
                } 
                else if (!left && right) {
                    // Left on white, right on black: turn right
                    error = 1.0f;
                    lostLineCounter = 0;
                    EventManager::getInstance().setBits(EventManager::LINE_DETECTED);
                } 
                else {
                    // Both on white: lost line
                    lostLineCounter++;
                    EventManager::getInstance().clearBits(EventManager::LINE_DETECTED);
                    
                    // Maintain previous error direction to try and recover
                    if (lostLineCounter >= lostLineThresholdCycles) {
                        // Lost line for too long: stop robot
                        MotorCommand stopCmd = {0, 0, false};
                        QueueManager::getInstance().sendMotorCommand(stopCmd);
                        
                        // Notify Navigation of line loss
                        NavigationCommand navCmd = {RobotState::IDLE, "Lost Line", false, 0, 0};
                        QueueManager::getInstance().sendNavigationCommand(navCmd);
                        continue;
                    }
                }

                // 2. PID Calculations
                integral += error * dt;
                // Clamp integral to prevent windup
                integral = constrain(integral, -50.0f, 50.0f);
                derivative = (error - lastError) / dt;
                lastError = error;

                float steering = (LINE_KP * error) + (LINE_KI * integral) + (LINE_KD * derivative);

                // 3. Compute target speeds
                int16_t leftTarget = static_cast<int16_t>(BASE_MOTOR_SPEED + steering);
                int16_t rightTarget = static_cast<int16_t>(BASE_MOTOR_SPEED - steering);

                // Constrain targets to valid L298N PWM range (0 to 255)
                leftTarget = constrain(leftTarget, 0, MAX_MOTOR_SPEED);
                rightTarget = constrain(rightTarget, 0, MAX_MOTOR_SPEED);

                MotorCommand cmd = {leftTarget, rightTarget, false};
                QueueManager::getInstance().sendMotorCommand(cmd);

            } else {
                // Not line following: reset PID state
                error = 0.0f;
                lastError = 0.0f;
                integral = 0.0f;
                lostLineCounter = 0;
            }
        }

        // Notify watchdog
        WatchdogTask::feed(LINE_FOLLOWER_TASK);
    }
}
