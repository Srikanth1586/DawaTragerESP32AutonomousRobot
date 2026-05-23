#pragma once
enum WifiStatus
{
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
};
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