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

    char statusMessage[24];

    char currentPos[24];
};

enum RobotMode
{
    STATE_SEARCH_DOCK,
    STATE_WAIT_ORDER,
    STATE_GO_TO_PHARMACY,
    STATE_AT_PHARMACY_WAIT_DOOR,
    STATE_SEARCH_DESTINATION,
    STATE_UNLOADING_MEDICINE,
    STATE_RETURN_TO_DOCK,
    STATE_STOP
};

extern RobotState robotState;
extern RobotMode currentState;
