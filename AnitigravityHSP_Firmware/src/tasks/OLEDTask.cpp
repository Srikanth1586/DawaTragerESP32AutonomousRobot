#include "tasks/OLEDTask.h"
#include "tasks/WatchdogTask.h"
#include "drivers/OLEDDriver.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "managers/EventManager.h"
#include "config/robot_config.h"

void OLEDTask::run(void* pvParameters) {
    OLEDDriver oled;
    bool oledOk = oled.init();

    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(100); // 100ms period

    DisplayData queueData;
    uint32_t alertTimeoutCycles = 0;

    while (true) {
        if (oledOk) {
            // Check if there is an alert/override screen requested in the display queue
            if (xQueueReceive(QueueManager::getInstance().getDisplayQueue(), &queueData, 0) == pdPASS) {
                // Show custom alert
                oled.showAlert("ALERT MESSAGE", queueData.robotID); // Or map queue values
                alertTimeoutCycles = 30; // Display alert for 3 seconds (30 * 100ms)
            }

            if (alertTimeoutCycles > 0) {
                alertTimeoutCycles--;
            } else {
                // Build normal status screen data
                DisplayData statusData;
                statusData.batteryPercentage = StateManager::getInstance().getBatteryPercent();
                statusData.currentState = StateManager::getInstance().getState();
                StateManager::getInstance().getCurrentStation(statusData.currentStation, sizeof(statusData.currentStation));
                
                EventBits_t events = EventManager::getInstance().getBits();
                statusData.wifiConnected = (events & EventManager::WIFI_CONNECTED);
                statusData.serverConnected = (events & EventManager::SERVER_CONNECTED);
                
                strncpy(statusData.robotID, ROBOT_ID, sizeof(statusData.robotID) - 1);
                statusData.robotID[sizeof(statusData.robotID) - 1] = '\0';
                
                statusData.obstacleDistanceMm = StateManager::getInstance().getObstacleDistance();
                statusData.obstacleAlert = (events & EventManager::OBSTACLE_PRESENT);

                // Update OLED screen
                oled.updateScreen(statusData);
            }
        }

        // Notify watchdog
        WatchdogTask::feed(OLED_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
