#include <Arduino.h>
#include "gui.h"
#include "display.h"
#include "robot_state.h"

void oledTask(void *pvParameters)
{
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (1)
    {
        // WiFi blink animation
        wifiBlinkState = !wifiBlinkState;

        // Battery animation frame
        batteryAnimFrame++;

        if (batteryAnimFrame > 1)
        {
            batteryAnimFrame = 0;
        }

        // Clear screen buffer
        u8g2.clearBuffer();

        // Draw top notification panel
        drawNotificationBar();

        // Demo center text
        u8g2.setFont(u8g2_font_ncenB08_tr);
        const char *title = "HOSPITAL ROBOT";
        int16_t titleWidth = u8g2.getUTF8Width(title);
        int16_t titleX = max(0, (128 - titleWidth) / 2);
        u8g2.drawStr(titleX,
                     35,
                     title);

        u8g2.setFont(u8g2_font_6x10_tf);

        char stateText[40];
        if (millis() < 5000)
        {
            snprintf(stateText, sizeof(stateText), "INIT");
        }
        else if (strcmp(robotState.currentTask, "INIT") == 0)
        {
            snprintf(stateText, sizeof(stateText), "DOCKING");
        }
        else
        {
            snprintf(stateText, sizeof(stateText), "%s", robotState.currentTask);
        }
        int16_t stateWidth = u8g2.getUTF8Width(stateText);
        int16_t stateX = max(0, (128 - stateWidth) / 2);
        u8g2.drawStr(stateX,
                     48,
                     stateText);

        char posText[40];
        snprintf(posText, sizeof(posText), "POS: %s", robotState.currentPos);
        int16_t posWidth = u8g2.getUTF8Width(posText);
        int16_t posX = max(0, (128 - posWidth) / 2);
        u8g2.drawStr(posX,
                     62,
                     posText);

        // Send buffer to OLED
        u8g2.sendBuffer();

        // 200ms refresh
        vTaskDelayUntil(&lastWakeTime,
                        pdMS_TO_TICKS(200));
    }
}
