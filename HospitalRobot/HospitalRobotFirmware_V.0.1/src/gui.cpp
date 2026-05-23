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

        u8g2.drawStr(18,
                     35,
                     "HOSPITAL ROBOT");

        u8g2.setFont(u8g2_font_6x10_tf);

        u8g2.drawStr(30,
                     52,
                     "STATUS: READY");

        // Send buffer to OLED
        u8g2.sendBuffer();

        // 200ms refresh
        vTaskDelayUntil(&lastWakeTime,
                        pdMS_TO_TICKS(200));
    }
}
