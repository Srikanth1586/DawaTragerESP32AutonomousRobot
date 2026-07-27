#pragma once
#include <Arduino.h>
#include "display.h"
#include "robot_state.h"


extern bool wifiBlinkState;
extern int batteryAnimFrame;

extern void oledTask(void *pvParameters);