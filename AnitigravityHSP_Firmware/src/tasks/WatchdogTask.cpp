#include "tasks/WatchdogTask.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "config/pins.h"
#include <esp_task_wdt.h>

// Initialize static countdowns array
static volatile int16_t s_taskCountdowns[NUM_TASKS];
static portMUX_TYPE s_watchdogMux = portMUX_INITIALIZER_UNLOCKED;

// Default timeouts in seconds for each task
static const int16_t s_taskTimeouts[NUM_TASKS] = {
    3,  // SENSOR_TASK (20ms period)
    2,  // ENCODER_TASK (10ms period)
    3,  // LINE_FOLLOWER_TASK (20ms period)
    3,  // OBSTACLE_DETECTION_TASK (50ms period)
    3,  // NAVIGATION_TASK (50ms period)
    5,  // NFC_TASK (100ms period)
    2,  // MOTOR_CONTROL_TASK (10ms period)
    5,  // OLED_TASK (100ms period)
    15, // COMM_TASK (reconnects take time)
    15  // HEARTBEAT_TASK (5s period)
};

void WatchdogTask::feed(TaskId id) {
    if (id >= NUM_TASKS) return;
    portENTER_CRITICAL(&s_watchdogMux);
    s_taskCountdowns[id] = s_taskTimeouts[id];
    portEXIT_CRITICAL(&s_watchdogMux);
}

void WatchdogTask::run(void* pvParameters) {
    // Enable ESP32 hardware watchdog for this task (timeout 3 seconds)
    esp_task_wdt_init(3, true); 
    esp_task_wdt_add(NULL); // Subscribe WatchdogTask to hardware watchdog

    // Initialize all task countdowns to their starting values
    for (int i = 0; i < NUM_TASKS; i++) {
        s_taskCountdowns[i] = s_taskTimeouts[i];
    }

    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(1000); // 1 second period

    while (true) {
        // Feed ESP32 hardware watchdog
        esp_task_wdt_reset();

        bool systemHealthy = true;
        TaskId failedTaskId = NUM_TASKS;

        portENTER_CRITICAL(&s_watchdogMux);
        for (int i = 0; i < NUM_TASKS; i++) {
            s_taskCountdowns[i]--;
            if (s_taskCountdowns[i] <= 0) {
                systemHealthy = false;
                failedTaskId = static_cast<TaskId>(i);
                break; // A task hung!
            }
        }
        portEXIT_CRITICAL(&s_watchdogMux);

        // Check for queue overflows
        QueueManager& qm = QueueManager::getInstance();
        if (uxQueueSpacesAvailable(qm.getSensorQueue()) == 0 ||
            uxQueueSpacesAvailable(qm.getMotorQueue()) == 0 ||
            uxQueueSpacesAvailable(qm.getNavigationQueue()) == 0) {
            
            Serial.println("[WATCHDOG] Queue overflow detected!");
            systemHealthy = false;
        }

        if (!systemHealthy) {
            // Sound alarm tone
            pinMode(PIN_BUZZER, OUTPUT);
            digitalWrite(PIN_BUZZER, HIGH);

            Serial.printf("[WATCHDOG] CRITICAL FAILURE: Task ID %d hung or queue overflowed! Restarting...\n", failedTaskId);
            delay(1000); // Give time for debug logs and buzzer
            
            // Trigger reboot
            esp_restart();
        }

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
