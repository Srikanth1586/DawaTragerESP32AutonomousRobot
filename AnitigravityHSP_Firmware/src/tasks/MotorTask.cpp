#include "tasks/MotorTask.h"
#include "tasks/WatchdogTask.h"
#include "drivers/MotorDriver.h"
#include "managers/QueueManager.h"
#include "managers/EventManager.h"
#include "managers/StateManager.h"
#include "config/robot_config.h"

void MotorTask::run(void* pvParameters) {
    MotorDriver motors;
    motors.init();

    // PID states
    float errorL = 0.0f, lastErrorL = 0.0f, integralL = 0.0f;
    float errorR = 0.0f, lastErrorR = 0.0f, integralR = 0.0f;

    constexpr float dt = 0.01f; // 10ms control loop (EncoderTask period)
    constexpr float MAX_SPEED_MPS = 0.5f; // 0.5 meters/sec max speed

    // Local target speeds (in m/s)
    float targetSpeedL = 0.0f;
    float targetSpeedR = 0.0f;

    EncoderData encoderData;
    MotorCommand motorCmd;

    while (true) {
        // 1. Block on encoder data queue (arrives every 10ms)
        if (xQueueReceive(QueueManager::getInstance().getEncoderQueue(), &encoderData, portMAX_DELAY) == pdPASS) {
            
            // 2. Check for new motor target commands (non-blocking)
            if (xQueueReceive(QueueManager::getInstance().getMotorQueue(), &motorCmd, 0) == pdPASS) {
                if (motorCmd.isEmergencyStop) {
                    EventManager::getInstance().setBits(EventManager::EMERGENCY_STOP);
                }
                
                // Convert raw command PWM scale (-255 to 255) to target speed in m/s
                targetSpeedL = (static_cast<float>(motorCmd.leftTargetSpeed) / 255.0f) * MAX_SPEED_MPS;
                targetSpeedR = (static_cast<float>(motorCmd.rightTargetSpeed) / 255.0f) * MAX_SPEED_MPS;
            }

            // 3. Safety check: read event bits
            EventBits_t activeEvents = EventManager::getInstance().getBits();
            bool safetyStop = (activeEvents & EventManager::EMERGENCY_STOP) || 
                              (activeEvents & EventManager::OBSTACLE_PRESENT);

            if (safetyStop || StateManager::getInstance().getState() == RobotState::EMERGENCY_STOP) {
                // Hard safety stop
                motors.stop();
                
                // Reset PID states
                errorL = lastErrorL = integralL = 0.0f;
                errorR = lastErrorR = integralR = 0.0f;
                targetSpeedL = targetSpeedR = 0.0f;
                continue;
            }

            // 4. Left Wheel PID
            float actualSpeedL = encoderData.leftSpeed;
            errorL = targetSpeedL - actualSpeedL;
            
            if (abs(targetSpeedL) < 0.01f) {
                integralL = 0.0f;
                errorL = 0.0f;
            } else {
                integralL += errorL * dt;
                integralL = constrain(integralL, -100.0f, 100.0f);
            }
            
            float derivativeL = (errorL - lastErrorL) / dt;
            lastErrorL = errorL;

            // Compute control effort (PID + Feedforward)
            float feedforwardL = (targetSpeedL / MAX_SPEED_MPS) * 200.0f; // feedforward base PWM
            float outputL = feedforwardL + (MOTOR_KP * errorL * 100.0f) + 
                            (MOTOR_KI * integralL * 100.0f) + (MOTOR_KD * derivativeL * 100.0f);

            // 5. Right Wheel PID
            float actualSpeedR = encoderData.rightSpeed;
            errorR = targetSpeedR - actualSpeedR;

            if (abs(targetSpeedR) < 0.01f) {
                integralR = 0.0f;
                errorR = 0.0f;
            } else {
                integralR += errorR * dt;
                integralR = constrain(integralR, -100.0f, 100.0f);
            }

            float derivativeR = (errorR - lastErrorR) / dt;
            lastErrorR = errorR;

            float feedforwardR = (targetSpeedR / MAX_SPEED_MPS) * 200.0f;
            float outputR = feedforwardR + (MOTOR_KP * errorR * 100.0f) + 
                            (MOTOR_KI * integralR * 100.0f) + (MOTOR_KD * derivativeR * 100.0f);

            // If target is 0, make sure motor is stopped
            int16_t finalPWML = (abs(targetSpeedL) < 0.01f) ? 0 : static_cast<int16_t>(constrain(outputL, -255.0f, 255.0f));
            int16_t finalPWMR = (abs(targetSpeedR) < 0.01f) ? 0 : static_cast<int16_t>(constrain(outputR, -255.0f, 255.0f));

            // Write outputs to motor driver
            motors.setSpeeds(finalPWML, finalPWMR);
        }

        // Notify watchdog
        WatchdogTask::feed(MOTOR_CONTROL_TASK);
    }
}
