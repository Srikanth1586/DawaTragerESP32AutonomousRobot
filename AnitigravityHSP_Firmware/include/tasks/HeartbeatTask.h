#ifndef TASKS_HEARTBEAT_TASK_H
#define TASKS_HEARTBEAT_TASK_H

#include <Arduino.h>

class HeartbeatTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_HEARTBEAT_TASK_H
