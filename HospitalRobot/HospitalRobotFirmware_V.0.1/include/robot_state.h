#pragma once

struct RobotState
{
    int batteryPercent;

    bool charging;

    bool wifiConnected;

    bool serverConnected;

    char mode[12];

    char currentTask[20];
};

extern RobotState robotState;