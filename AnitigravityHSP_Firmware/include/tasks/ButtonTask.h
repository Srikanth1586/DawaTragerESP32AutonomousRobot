#ifndef TASKS_BUTTON_TASK_H
#define TASKS_BUTTON_TASK_H

#include <Arduino.h>

enum class ButtonId : uint8_t {
    LEFT = 0,
    SELECT,
    RIGHT
};

class ButtonTask {
public:
    static void run(void* pvParameters);

    // Static ISR handlers
    static void IRAM_ATTR handleLeftButtonISR();
    static void IRAM_ATTR handleSelectButtonISR();
    static void IRAM_ATTR handleRightButtonISR();

private:
    static QueueHandle_t s_isrQueue;
};

#endif // TASKS_BUTTON_TASK_H
