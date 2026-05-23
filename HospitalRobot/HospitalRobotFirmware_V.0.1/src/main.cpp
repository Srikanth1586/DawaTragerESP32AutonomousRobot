#ifndef UNIT_TEST
#include <Arduino.h>

#include "Comm.h"

// ======================================================
// TASK HANDLE
// ======================================================

TaskHandle_t communicationTaskHandle = NULL;

// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println("ESP32 STARTING...");

    // ==========================================
    // WIFI
    // ==========================================

    initWiFi();

    // ==========================================
    // WEBSOCKET
    // ==========================================

    initWebSocket();

    // ==========================================
    // CREATE QUEUE
    // ==========================================

    messageQueue =
    xQueueCreate(10, sizeof(String));

    // ==========================================
    // CREATE COMM TASK
    // ==========================================

    xTaskCreatePinnedToCore(
        communicationTask,       // Task Function
        "Communication Task",    // Task Name
        9192,                    // Stack Size
        NULL,                    // Parameters
        1,                       // Priority
        &communicationTaskHandle,// Task Handle
        0                        // Core
    );

    Serial.println("SYSTEM READY");
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    // EMPTY

    vTaskDelay(pdMS_TO_TICKS(1000));
}
#endif