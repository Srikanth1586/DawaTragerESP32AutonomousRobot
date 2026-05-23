#ifndef COMM_H
#define COMM_H

#include <Arduino.h>

// =========================
// GLOBAL QUEUE
// =========================

extern QueueHandle_t messageQueue;

// =========================
// FUNCTIONS
// =========================

void initWiFi();

void initWebSocket();

void sendHeartbeat();

void sendRobotMessage(String message);

void communicationTask(void *pvParameters);

#endif