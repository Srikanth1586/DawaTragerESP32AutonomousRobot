
#pragma once
#include <U8g2lib.h>
#include "robot_state.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void drawNotificationBar();
void drawBatteryIcon(int x, int y, int level, bool charging);
void drawWifiIcon(int x, int y, WifiStatus status);