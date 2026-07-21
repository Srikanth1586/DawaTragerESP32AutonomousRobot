#ifndef TASKS_MOTOR_TASK_H
#define TASKS_MOTOR_TASK_H

#include <Arduino.h>

class MotorTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_MOTOR_TASK_H
