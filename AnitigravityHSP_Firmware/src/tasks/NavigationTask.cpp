#include "tasks/NavigationTask.h"
#include "tasks/WatchdogTask.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"
#include "config/pins.h"
#include "config/robot_config.h"

void NavigationTask::run(void* pvParameters) {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(50); // 50ms period

    NavigationCommand navCmd;
    RobotState previousState = RobotState::IDLE;

    // Non-blocking buzzer state variables
    uint32_t buzzerCounter = 0;
    bool buzzerActive = false;

    while (true) {
        // 1. Process navigation commands (non-blocking read)
        if (xQueueReceive(QueueManager::getInstance().getNavigationQueue(), &navCmd, 0) == pdPASS) {
            RobotState current = StateManager::getInstance().getState();

            // Ignore state transitions if we are in EMERGENCY_STOP unless it's a reset/recovery
            if (current == RobotState::EMERGENCY_STOP && navCmd.targetState != RobotState::IDLE) {
                // Ignore other transitions until reset to IDLE
            } else {
                if (current != navCmd.targetState) {
                    previousState = current;
                    StateManager::getInstance().setState(navCmd.targetState);
                    
                    // State transition buzzer notifications
                    buzzerActive = true;
                    if (navCmd.targetState == RobotState::EMERGENCY_STOP) {
                        buzzerCounter = 9999; // Continuous beep
                    } else if (navCmd.targetState == RobotState::CHECKPOINT_DETECTED) {
                        buzzerCounter = 4;    // 200ms beep (4 * 50ms)
                        StateManager::getInstance().setCurrentStation(navCmd.nextStation);
                    } else if (navCmd.targetState == RobotState::CHARGING) {
                        buzzerCounter = 6;    // 300ms double-beep pattern
                        EventManager::getInstance().setBits(EventManager::CHARGING_ACTIVE);
                    } else {
                        buzzerCounter = 2;    // Short 100ms beep
                    }
                }
            }
        }

        // 2. State Machine Logic (runs every 50ms)
        RobotState state = StateManager::getInstance().getState();
        EventBits_t events = EventManager::getInstance().getBits();

        switch (state) {
            case RobotState::IDLE:
                // Stop motors
                {
                    MotorCommand stopCmd = {0, 0, false};
                    QueueManager::getInstance().sendMotorCommand(stopCmd);
                }
                EventManager::getInstance().clearBits(EventManager::CHARGING_ACTIVE);
                break;

            case RobotState::LINE_FOLLOWING:
                // LineFollowerTask is actively sending motor commands based on PID
                break;

            case RobotState::CHECKPOINT_DETECTED:
                // Temporary state: Stop at the station for a few seconds, then resume
                {
                    static uint32_t stationWaitCycles = 0;
                    stationWaitCycles++;
                    
                    // Stop motors while at checkpoint
                    MotorCommand stopCmd = {0, 0, false};
                    QueueManager::getInstance().sendMotorCommand(stopCmd);

                    // Wait 3 seconds (3000ms / 50ms = 60 cycles)
                    if (stationWaitCycles >= 60) {
                        stationWaitCycles = 0;
                        // Resume line following
                        StateManager::getInstance().setState(RobotState::LINE_FOLLOWING);
                    }
                }
                break;

            case RobotState::OBSTACLE_AVOIDANCE:
                // Stop motors and alert
                {
                    MotorCommand stopCmd = {0, 0, false};
                    QueueManager::getInstance().sendMotorCommand(stopCmd);
                }
                
                // If obstacle is removed, transition back to LINE_FOLLOWING (or IDLE if it was IDLE)
                if (!(events & EventManager::OBSTACLE_PRESENT)) {
                    if (previousState == RobotState::LINE_FOLLOWING) {
                        StateManager::getInstance().setState(RobotState::LINE_FOLLOWING);
                    } else {
                        StateManager::getInstance().setState(RobotState::IDLE);
                    }
                } else {
                    // Beep buzzer alert rapidly
                    buzzerActive = true;
                    if (buzzerCounter == 0) {
                        buzzerCounter = 2; // Beep every few cycles
                    }
                }
                break;

            case RobotState::CHARGING:
                // Stop motors
                {
                    MotorCommand stopCmd = {0, 0, false};
                    QueueManager::getInstance().sendMotorCommand(stopCmd);
                }
                break;

            case RobotState::EMERGENCY_STOP:
                // Shut down motors immediately via safety stop
                {
                    MotorCommand stopCmd = {0, 0, true};
                    QueueManager::getInstance().sendMotorCommand(stopCmd);
                }
                break;
        }

        // 3. Non-blocking Buzzer Control
        if (buzzerActive) {
            if (buzzerCounter > 0) {
                // If emergency, continuous beep
                if (state == RobotState::EMERGENCY_STOP) {
                    digitalWrite(PIN_BUZZER, HIGH);
                } else {
                    // Toggle beep
                    digitalWrite(PIN_BUZZER, (buzzerCounter % 2 == 0) ? HIGH : LOW);
                    buzzerCounter--;
                }
            } else {
                digitalWrite(PIN_BUZZER, LOW);
                buzzerActive = false;
            }
        } else {
            digitalWrite(PIN_BUZZER, LOW);
        }

        // Notify watchdog
        WatchdogTask::feed(NAVIGATION_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
