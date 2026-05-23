#include "display.h"
#include "robot_state.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "gui.h"

// The concrete definition must exist in a .cpp file that both src and tests can link against
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);


bool wifiBlinkState = true;
int batteryAnimFrame = 0;

void drawBatteryIcon(int x, int y, int level, bool charging)
{
    // Battery outline
    u8g2.drawFrame(x, y, 16, 8);

    // Battery terminal
    u8g2.drawBox(x + 16, y + 2, 2, 4);

    // Battery fill
    int fillWidth = map(level, 0, 100, 0, 14);

    u8g2.drawBox(x + 1, y + 1, fillWidth, 6);

    // Charging animation
    if (charging)
    {
        if (batteryAnimFrame == 0)
        {
            u8g2.drawLine(x + 6, y + 1, x + 4, y + 4);
            u8g2.drawLine(x + 4, y + 4, x + 7, y + 4);
            u8g2.drawLine(x + 7, y + 4, x + 5, y + 7);
        }
    }
}

void drawWifiIcon(int x, int y, WifiStatus status)
{
    switch (status)
    {
        case WIFI_CONNECTED:
        {
            // Center dot
            u8g2.drawDisc(x, y, 1);

            // Small arc
            u8g2.drawCircle(x, y, 3, U8G2_DRAW_UPPER_LEFT |
                                      U8G2_DRAW_UPPER_RIGHT);

            // Medium arc
            u8g2.drawCircle(x, y, 6, U8G2_DRAW_UPPER_LEFT |
                                      U8G2_DRAW_UPPER_RIGHT);

            // Large arc
            u8g2.drawCircle(x, y, 9, U8G2_DRAW_UPPER_LEFT |
                                      U8G2_DRAW_UPPER_RIGHT);

            break;
        }

        case WIFI_CONNECTING:
        {
            if (wifiBlinkState)
            {
                u8g2.drawDisc(x, y, 1);

                u8g2.drawCircle(x, y, 3,
                                U8G2_DRAW_UPPER_LEFT |
                                U8G2_DRAW_UPPER_RIGHT);

                u8g2.drawCircle(x, y, 6,
                                U8G2_DRAW_UPPER_LEFT |
                                U8G2_DRAW_UPPER_RIGHT);
            }

            break;
        }

        case WIFI_DISCONNECTED:
        {
            // Partial WiFi arcs
            u8g2.drawCircle(x, y, 3,
                            U8G2_DRAW_UPPER_LEFT |
                            U8G2_DRAW_UPPER_RIGHT);

            u8g2.drawCircle(x, y, 6,
                            U8G2_DRAW_UPPER_LEFT |
                            U8G2_DRAW_UPPER_RIGHT);

            // X symbol
            u8g2.drawLine(x - 4, y - 4,
                           x + 4, y + 4);

            u8g2.drawLine(x - 4, y + 4,
                           x + 4, y - 4);

            break;
        }
    }
}

void drawNotificationBar()
{
    // Top separator line
    u8g2.drawLine(0, 12, 127, 12);

    // Small font
    u8g2.setFont(u8g2_font_5x7_tf);

    // Battery icon
    drawBatteryIcon(2, 2,
                     robotState.batteryPercent,
                     robotState.charging);

    // Battery text
    char batteryText[8];

    sprintf(batteryText, "%d%%",
            robotState.batteryPercent);

    u8g2.drawStr(22, 9, batteryText);

    // WiFi icon
    WifiStatus wifiStatus = robotState.wifiConnected ? WIFI_CONNECTED : WIFI_DISCONNECTED;
    drawWifiIcon(50, 7, wifiStatus);

    // Mode
    u8g2.drawStr(62, 9,
                 robotState.mode);

    // Current task short name
    u8g2.drawStr(95, 9,
                 robotState.currentTask);
}
