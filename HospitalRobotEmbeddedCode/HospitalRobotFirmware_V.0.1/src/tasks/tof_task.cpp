#include <Arduino.h>

#include "tof_driver.h"
#include "queue_manager.h"
#include "event_manager.h"
#include "structs.h"

#define OBSTACLE_THRESHOLD_MM 300

void ToFTask(void *pvParameters)
{
    ToFData_t tofData;

    if(!tofInit())
    {
        vTaskDelete(NULL);
    }

    while(1)
    {
        uint16_t distance = readDistance();

        tofData.distanceMM = distance;

        if(distance < OBSTACLE_THRESHOLD_MM)
        {
            tofData.obstacleDetected = true;

            xEventGroupSetBits(
                robotEventGroup,
                EVENT_OBSTACLE_DETECTED);
        }
        else
        {
            tofData.obstacleDetected = false;

            xEventGroupClearBits(
                robotEventGroup,
                EVENT_OBSTACLE_DETECTED);
        }

        xQueueOverwrite(
            tofQueue,
            &tofData);

        Serial.print("ToF: ");
        Serial.print(distance);
        Serial.println(" mm");

        vTaskDelay(
            pdMS_TO_TICKS(50));
    }
}