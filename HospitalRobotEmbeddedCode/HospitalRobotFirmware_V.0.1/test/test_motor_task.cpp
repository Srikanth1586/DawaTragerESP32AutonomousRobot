#include <Arduino.h>

#include "queue_manager.h"

void TestMotorTask(void *pvParameters)
{
    MotorCommand_t cmd;

    while(1)
    {
        cmd.motion = MOTION_FORWARD;
        cmd.leftSpeed = 180;
        cmd.rightSpeed = 180;

        xQueueSend(motorQueue, &cmd, portMAX_DELAY);

        Serial.println("Forward");

        vTaskDelay(pdMS_TO_TICKS(3000));

        cmd.motion = MOTION_STOP;

        xQueueSend(motorQueue, &cmd, portMAX_DELAY);

        Serial.println("Stop");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}