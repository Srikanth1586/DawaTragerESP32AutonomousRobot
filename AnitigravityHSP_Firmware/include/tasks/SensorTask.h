#ifndef TASKS_SENSOR_TASK_H
#define TASKS_SENSOR_TASK_H

#include <Arduino.h>

class SensorTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_SENSOR_TASK_H
