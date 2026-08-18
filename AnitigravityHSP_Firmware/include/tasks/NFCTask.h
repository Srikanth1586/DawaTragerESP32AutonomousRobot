#ifndef TASKS_NFC_TASK_H
#define TASKS_NFC_TASK_H

#include <Arduino.h>

class NFCTask {
public:
    static void run(void* pvParameters);
};

#endif // TASKS_NFC_TASK_H
