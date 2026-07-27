#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "structs.h"

// ==========================
// Queue Handles
// ==========================

extern QueueHandle_t motorQueue;
extern QueueHandle_t encoderQueue;
extern QueueHandle_t lineSensorQueue;
extern QueueHandle_t nfcQueue;
extern QueueHandle_t tofQueue;
extern QueueHandle_t commandQueue;

// ==========================
// Init Function
// ==========================

void initQueues();

#endif