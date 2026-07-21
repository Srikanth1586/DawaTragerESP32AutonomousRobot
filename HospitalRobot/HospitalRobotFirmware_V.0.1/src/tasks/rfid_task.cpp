#include <Arduino.h>
#include "queue_manager.h"
#include "event_manager.h"
#include "rfid_driver.h"
#include "system_state.h"
#include "structs.h"
#include "rfid_data.h"
#include "robot_state.h"
#include "Comm.h"

RFIDResult_t lastScannedCard;

void RFIDTask(void *pvParameters)
{
    NFCData_t nfcData;
    char uid[32];

    Serial.println("🔄 RFID Task starting initialization...");
    
    if (!rfidInit())
    {
        Serial.println("❌ RFID Init Failed - Task Stopped");
        vTaskDelete(NULL);
    }

    Serial.println("✅ RFID Task initialized successfully");

    while (1)
    {
        // try reading card
        if (readRFID(uid, sizeof(uid)))
        {
            strncpy(nfcData.uid, uid, sizeof(nfcData.uid) - 1);
            nfcData.uid[sizeof(nfcData.uid) - 1] = '\0';

            // send to queue
            if (xQueueSend(nfcQueue, &nfcData, 0) == pdPASS)
            {
                Serial.print("📤 RFID data sent to queue: ");
                Serial.println(uid);
            }

            // set event flag
            xEventGroupSetBits(
                robotEventGroup,
                EVENT_NFC_DETECTED);

            // prevent multiple reads of same card
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Monitor task to process RFID data from queue
void RFIDMonitorTask(void *pvParameters)
{
    NFCData_t receivedData;

    Serial.println("🔄 RFID Monitor Task started");

    while (1)
    {
        // Try to receive data from the RFID queue
        if (xQueueReceive(nfcQueue, &receivedData, pdMS_TO_TICKS(500)) == pdPASS)
        {
    
            RFIDResult_t card = getRFIDInfo(receivedData.uid);
            Serial.println("═══════════════════════════════════");
            Serial.println("📍 RFID CARD DETECTED!");
            Serial.print("Card UID: ");
            Serial.println(card.uid);
            if(card.found)
                {
                    Serial.print("Location  : ");
                    Serial.println(card.location);

                    Serial.print("Room No   : ");
                    Serial.println(card.roomNumber);

                    char statusBuffer[24];
                    snprintf(statusBuffer, sizeof(statusBuffer), "AT %s", card.location);
                    strncpy(robotState.statusMessage, statusBuffer, sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                }
                else
                {
                    Serial.println("❌ Unknown RFID Card");
                    strncpy(robotState.statusMessage, "UNKNOWN CARD", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                }

                Serial.println("═══════════════════════════════════");

                // Send NFC status to server
                sendRFIDDataToServer(&card);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
