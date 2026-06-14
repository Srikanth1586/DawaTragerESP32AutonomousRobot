
#ifndef USS_H
#define USS_H
#include <Arduino.h>
#include <Config.h> // For pin definitions

void IRAM_ATTR echo_isr();
void setup_ultrasonic();
void vNavTask(void *pvParameters);


#endif