#ifndef TASKS_ENCODER_TASK_H
#define TASKS_ENCODER_TASK_H

#include <Arduino.h>

class EncoderTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_ENCODER_TASK_H
