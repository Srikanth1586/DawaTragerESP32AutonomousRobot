#include <Arduino.h>
#include "queue_manager.h"
#include "event_manager.h"
#include "nfc_driver.h"
#include "system_state.h"

void NFCTask(void *pvParameters)
{
    NFCData_t nfcData;

    char uid[20];

    if (!nfcInit())
    {
        Serial.println("❌ NFC Init Failed - Task Stopped");
        vTaskDelete(NULL);
    }

    while (1)
    {
        // try reading card
        if (readNFC(uid))
        {
            strcpy(nfcData.uid, uid);

            // send to queue
            xQueueSend(nfcQueue, &nfcData, 0);

            // set event flag
            xEventGroupSetBits(
                robotEventGroup,
                EVENT_NFC_DETECTED);

            Serial.print("📡 NFC UID: ");
            Serial.println(uid);

            // prevent multiple reads of same card
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
