#include <Arduino.h>
#include <unity.h>
#include "USS.h"

// --- This is your Sandbox ---
// We wrap your direct hardware test inside a single function so Unity can launch it.
void raw_hardware_sandbox(void) {
    Serial.println("--- HARDWARE SANDBOX STARTED ---");

    // Start your actual hardware task manually here if it wasn't started in setup
    xTaskCreatePinnedToCore(vNavTask, "NavTask", 4096, NULL, 3, NULL, 1);

    // Let your hardware code run interactively.
    // If your vNavTask runs an infinite loop reading sensors and printing to Serial,
    // this delay keeps the "test" alive for 30 seconds so you can watch/test the hardware live.
    vTaskDelay(pdMS_TO_TICKS(30000)); 

    Serial.println("--- HARDWARE SANDBOX FINISHED ---");
    
    // Optional: If you want to force the test to pass at the end of 30 seconds
    //TEST_PASS_MESSAGE("Hardware evaluation complete."); 
}

// --- This task strictly manages PlatformIO's environment requirements ---
void vTestRunnerTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Handshake window for USB connection

    UNITY_BEGIN(); 

    // This launches your sandbox loop
    RUN_TEST(raw_hardware_sandbox); 

    UNITY_END(); 

    vTaskDelete(NULL);
}

void setup() {
    Serial.begin(115200);

    xTaskCreatePinnedToCore(
        vTestRunnerTask, 
        "TestRunner", 
        4096, 
        NULL, 
        3, 
        NULL, 
        1);
}

void loop() {
    vTaskDelete(NULL);
}