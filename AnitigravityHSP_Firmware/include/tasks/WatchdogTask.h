#ifndef TASKS_WATCHDOG_TASK_H
#define TASKS_WATCHDOG_TASK_H

#include <Arduino.h>

enum TaskId : uint8_t {
    SENSOR_TASK = 0,
    ENCODER_TASK,
    LINE_FOLLOWER_TASK,
    OBSTACLE_DETECTION_TASK,
    NAVIGATION_TASK,
    NFC_TASK,
    MOTOR_CONTROL_TASK,
    OLED_TASK,
    COMM_TASK,
    HEARTBEAT_TASK,
    NUM_TASKS
};

class WatchdogTask {
public:
    static void run(void* pvParameters);
    static void feed(TaskId id);
};

#endif // TASKS_WATCHDOG_TASK_H
