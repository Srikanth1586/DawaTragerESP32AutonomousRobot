#ifndef TASKS_COMM_TASK_H
#define TASKS_COMM_TASK_H

#include <Arduino.h>

class CommTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_COMM_TASK_H
