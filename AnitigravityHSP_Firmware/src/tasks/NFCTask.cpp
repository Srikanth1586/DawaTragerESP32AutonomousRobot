#include "tasks/NFCTask.h"
#include "tasks/WatchdogTask.h"
#include "drivers/PN532Driver.h"
#include "managers/QueueManager.h"
#include "managers/EventManager.h"
#include "managers/StateManager.h"

void NFCTask::run(void* pvParameters) {
    PN532Driver nfc;
    bool nfcOk = nfc.init();

    uint32_t lastWakeTime = xTaskGetTickCount();
    constexpr TickType_t period = pdMS_TO_TICKS(100); // 100ms period

    char stationBuf[16];

    while (true) {
        if (nfcOk) {
            bool tagFound = nfc.readTag(stationBuf, sizeof(stationBuf));

            if (tagFound) {
                // Set NFC event bit
                EventManager::getInstance().setBits(EventManager::NFC_DETECTED);

                // Publish station data
                NFCData data;
                strncpy(data.stationID, stationBuf, sizeof(data.stationID) - 1);
                data.stationID[sizeof(data.stationID) - 1] = '\0';
                data.tagDetected = true;

                QueueManager::getInstance().sendNFCData(data);

                // Send navigation command to state machine (e.g. check station arrival)
                NavigationCommand navCmd = {RobotState::CHECKPOINT_DETECTED, "", false, 0, 0};
                strncpy(navCmd.nextStation, stationBuf, sizeof(navCmd.nextStation) - 1);
                navCmd.nextStation[sizeof(navCmd.nextStation) - 1] = '\0';
                QueueManager::getInstance().sendNavigationCommand(navCmd);
            } else {
                // Clear NFC event bit
                EventManager::getInstance().clearBits(EventManager::NFC_DETECTED);
            }
        }

        // Notify watchdog
        WatchdogTask::feed(NFC_TASK);

        vTaskDelayUntil(&lastWakeTime, period);
    }
}
