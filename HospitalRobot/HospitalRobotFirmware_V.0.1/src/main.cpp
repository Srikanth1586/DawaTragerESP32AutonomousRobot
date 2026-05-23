#include <Arduino.h>
#include "display.h"
#include "gui.h"
#include "robot_state.h"
#include <unity.h>

RobotState robotState;  
extern void oledTask(void *pvParameters);

void setup()
{
    Serial.begin(115200);

    u8g2.begin();

    // Initial values
    robotState.batteryPercent = 20;

    robotState.charging = false;

    robotState.wifiConnected = false;

    strcpy(robotState.mode, "BOOT");

    strcpy(robotState.currentTask, "INIT");

    // OLED task
    xTaskCreatePinnedToCore(
        oledTask,
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