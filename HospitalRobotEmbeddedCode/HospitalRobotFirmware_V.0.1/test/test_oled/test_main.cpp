#include <Arduino.h>
#include "display.h"
#include "gui.h"
#include "robot_state.h"
#include <unity.h>

RobotState robotState;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;    
extern void oledTask(void *pvParameters);
void raw_hardware_sandbox(void) {
    Serial.println("--- OLED HARDWARE SANDBOX STARTED ---");
     xTaskCreatePinnedToCore(oledTask, "OLED Task", 4096, NULL, 1, NULL, 1);
    // Let your hardware code run interactively.
    // If your oledTask runs an infinite loop reading robotState and printing to the OLED,
    // this delay keeps the "test" alive for 30 seconds so you can watch/test the hardware live.
    vTaskDelay(pdMS_TO_TICKS(30000)); 

    Serial.println("--- OLED HARDWARE SANDBOX FINISHED ---");
    
    // Optional: If you want to force the test to pass at the end of 30 seconds
    //TEST_PASS_MESSAGE("OLED Hardware evaluation complete."); 
}

void vTestRunnerTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Handshake window for USB connection

    UNITY_BEGIN(); 

    // This launches your sandbox loop
    RUN_TEST(raw_hardware_sandbox); 

    UNITY_END(); 

    vTaskDelete(NULL);
}

void setup()
{
    Serial.begin(115200);

    u8g2.begin();

    // Initial values
    robotState.batteryPercent = 85;

    robotState.charging = true;

    robotState.wifiConnected = true;

    strcpy(robotState.mode, "AUTO");

    strcpy(robotState.currentTask, "NAV");

    // OLED task
    xTaskCreatePinnedToCore(
        vTestRunnerTask,
        "Test Runner    Task",
        4096,
        NULL,
        1,
        NULL,
        1);
}

void loop()
{
}