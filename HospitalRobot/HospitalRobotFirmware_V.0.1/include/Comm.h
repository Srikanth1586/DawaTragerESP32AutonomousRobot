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

bool initWiFi();
bool getWiFiStatus();

void initWebSocket();
bool getServerStatus();
void servoInit();

void sendHeartbeat();

void sendRobotMessage(String message);

void communicationTask(void *pvParameters);

#endif