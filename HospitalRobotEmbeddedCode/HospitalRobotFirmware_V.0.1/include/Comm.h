#ifndef COMM_H
#define COMM_H

#include <Arduino.h>
#include "rfid_data.h"

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

// Send RFID data to server
void sendRFIDDataToServer(const RFIDResult_t *rfidData);

void communicationTask(void *pvParameters);

#endif