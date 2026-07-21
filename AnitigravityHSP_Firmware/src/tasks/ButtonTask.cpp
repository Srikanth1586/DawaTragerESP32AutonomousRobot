#include "tasks/ButtonTask.h"
#include "config/pins.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"

// Define static queue
QueueHandle_t ButtonTask::s_isrQueue = nullptr;

void ButtonTask::run(void* pvParameters) {
    // Instantiate the ISR communications queue
    s_isrQueue = xQueueCreate(10, sizeof(ButtonId));

    // Configure pins as input pull-ups
    pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
    pinMode(PIN_BTN_SELECT, INPUT_PULLUP);
    pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);

    // Attach interrupts on FALLING edge (active low buttons)
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_LEFT), handleLeftButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_SELECT), handleSelectButtonISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN_RIGHT), handleRightButtonISR, FALLING);

    ButtonId btnId;
    uint32_t lastPressTimeL = 0;
    uint32_t lastPressTimeSel = 0;
    uint32_t lastPressTimeR = 0;
    constexpr uint32_t DEBOUNCE_DELAY_MS = 250; // 250ms debounce window

    while (true) {
        // Block indefinitely until a button event is pushed from ISR
        if (xQueueReceive(s_isrQueue, &btnId, portMAX_DELAY) == pdPASS) {
            uint32_t now = millis();
            RobotState current = StateManager::getInstance().getState();

            if (btnId == ButtonId::LEFT) {
                if (now - lastPressTimeL > DEBOUNCE_DELAY_MS) {
                    lastPressTimeL = now;
                    
                    // Left Button: If in LINE_FOLLOWING, turn slightly left or toggle manual speed
                    if (current == RobotState::LINE_FOLLOWING) {
                        // Nudge robot left
                        MotorCommand cmd = {80, 160, false};
                        QueueManager::getInstance().sendMotorCommand(cmd);
                    } else if (current == RobotState::IDLE) {
                        // Sound test beep
                        NavigationCommand beepCmd = {RobotState::IDLE, "Beep", false, 0, 0};
                        QueueManager::getInstance().sendNavigationCommand(beepCmd);
                    }
                }
            } 
            else if (btnId == ButtonId::SELECT) {
                if (now - lastPressTimeSel > DEBOUNCE_DELAY_MS) {
                    lastPressTimeSel = now;

                    // Select Button: Toggle between LINE_FOLLOWING and IDLE, or reset EMERGENCY_STOP
                    NavigationCommand navCmd;
                    navCmd.hasTargetSpeed = false;
                    strncpy(navCmd.nextStation, "None", sizeof(navCmd.nextStation));

                    if (current == RobotState::EMERGENCY_STOP) {
                        // Reset Emergency Stop to IDLE
                        EventManager::getInstance().clearBits(EventManager::EMERGENCY_STOP);
                        navCmd.targetState = RobotState::IDLE;
                    } else if (current == RobotState::IDLE) {
                        // Start Following Line
                        navCmd.targetState = RobotState::LINE_FOLLOWING;
                    } else {
                        // Stop and return to IDLE
                        navCmd.targetState = RobotState::IDLE;
                    }

                    QueueManager::getInstance().sendNavigationCommand(navCmd);
                }
            } 
            else if (btnId == ButtonId::RIGHT) {
                if (now - lastPressTimeR > DEBOUNCE_DELAY_MS) {
                    lastPressTimeR = now;

                    // Right Button: If in LINE_FOLLOWING, simulate checkpoint NFC trigger
                    if (current == RobotState::LINE_FOLLOWING) {
                        NavigationCommand checkpointCmd;
                        checkpointCmd.targetState = RobotState::CHECKPOINT_DETECTED;
                        strncpy(checkpointCmd.nextStation, "BtnStation", sizeof(checkpointCmd.nextStation) - 1);
                        checkpointCmd.nextStation[sizeof(checkpointCmd.nextStation) - 1] = '\0';
                        checkpointCmd.hasTargetSpeed = false;
                        
                        QueueManager::getInstance().sendNavigationCommand(checkpointCmd);
                    } else if (current == RobotState::IDLE) {
                        // Go into Charging mode
                        NavigationCommand chargeCmd = {RobotState::CHARGING, "Dock", false, 0, 0};
                        QueueManager::getInstance().sendNavigationCommand(chargeCmd);
                    }
                }
            }
        }
    }
}

// ISR Handlers
void IRAM_ATTR ButtonTask::handleLeftButtonISR() {
    if (s_isrQueue == nullptr) return;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ButtonId btn = ButtonId::LEFT;
    xQueueSendFromISR(s_isrQueue, &btn, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR ButtonTask::handleSelectButtonISR() {
    if (s_isrQueue == nullptr) return;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ButtonId btn = ButtonId::SELECT;
    xQueueSendFromISR(s_isrQueue, &btn, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR ButtonTask::handleRightButtonISR() {
    if (s_isrQueue == nullptr) return;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ButtonId btn = ButtonId::RIGHT;
    xQueueSendFromISR(s_isrQueue, &btn, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
