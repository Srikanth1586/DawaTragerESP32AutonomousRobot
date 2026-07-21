#include "tasks/HeartbeatTask.h"
#include "tasks/WatchdogTask.h"
#include "managers/QueueManager.h"
#include "managers/StateManager.h"
#include "config/robot_config.h"
#include <ArduinoJson.h>

void HeartbeatTask::run(void* pvParameters) {
    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(5000); // 5 seconds period

    char stationBuf[16];

    while (true) {
        // Collect current state details
        float batteryPct = StateManager::getInstance().getBatteryPercent();
        RobotState state = StateManager::getInstance().getState();
        StateManager::getInstance().getCurrentStation(stationBuf, sizeof(stationBuf));

        // Format JSON payload
        StaticJsonDocument<256> doc;
        doc["type"] = "heartbeat";
        doc["robot_id"] = ROBOT_ID;
        doc["battery"] = batteryPct;
        doc["state"] = robotStateToString(state);
        doc["station"] = stationBuf;

        CommunicationMessage msg;
        msg.length = serializeJson(doc, msg.payload, sizeof(msg.payload));

        // Push to CommunicationQueue for CommTask to transmit
        QueueManager::getInstance().sendCommunicationMessage(msg);

        // Notify watchdog
        WatchdogTask::feed(HEARTBEAT_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
