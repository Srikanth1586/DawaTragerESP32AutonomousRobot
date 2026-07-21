#include <Arduino.h>
#include "display.h"
#include "gui.h"
#include "robot_state.h"
#include <unity.h>
#include "queue_manager.h"
#include "event_manager.h"
#include "structs.h"
#include "Config.h"
#include "motor_driver.h"
#include "tof_driver.h"
#include "rfid_driver.h"
#include "Comm.h"
#include "servo_driver.h"
#include "System_state.h"
#include <Esp32Servo.h>




RobotState robotState;  
extern void oledTask(void *pvParameters);



// ==========================
// Task Function Prototypes
// ==========================

void LineFollowTask(void *pvParameters);
void MotorControlTask(void *pvParameters);
void EncoderTask(void *pvParameters);
void RFIDTask(void *pvParameters);
void RFIDMonitorTask(void *pvParameters);
void ControlTask(void *pvParameters);
void ToFTask(void *pvParameters);
void communicationTask(void *pvParameters);


void setup()
{
    Serial.begin(115200);

    u8g2.begin();
       // ======================
    // Initialize RTOS Objects FIRST
    // ======================
    initQueues();
    initEventGroups();

        // OLED task
        // Initial values
    robotState.wifiConnected = false;
    robotState.batteryPercent = 20;
    robotState.charging = true;
    strcpy(robotState.mode, "BOOT");
    strcpy(robotState.currentTask, "INIT");
    strcpy(robotState.statusMessage, "READY");
    xTaskCreatePinnedToCore(
        oledTask,
        "Test Runner    Task",
        4096,
        NULL,
        1,
        NULL,
        1);
     // ======================
    // Initialize Hardware
    // ======================
    initWiFi();
    initWebSocket();

    // Configure sensor and servo pins before starting tasks
    pinMode(IR_LEFT, INPUT);
    pinMode(IR_RIGHT, INPUT_PULLUP);


    servoInit();
    robotState.wifiConnected = getWiFiStatus();
    // ======================
    // Create Tasks
    // ======================

    xTaskCreatePinnedToCore(
    communicationTask,
    "CommunicationTask",
    4096,
    NULL,
    3,        // Priority (adjust as needed)
    NULL,
    1);       // Core 1

   xTaskCreatePinnedToCore(
        LineFollowTask,
        "LineFollowTask",
        4096,
        NULL,
        4,
        NULL,
        1);

   xTaskCreatePinnedToCore(
        MotorControlTask,
        "MotorControlTask",
        4096,
        NULL,
        5,
        NULL,
        1);



    xTaskCreatePinnedToCore(
        ControlTask,
        "ControlTask",
        4096,
        NULL,
        5,
        NULL,
        1);

    // =======================
    // RFID Task and Monitor
    // =======================
    xTaskCreatePinnedToCore(
        RFIDTask,
        "RFIDTask",
        4096,
        NULL,
        2,
        NULL,
        0);

    xTaskCreatePinnedToCore(
        RFIDMonitorTask,
        "RFIDMonitorTask",
        4096,
        NULL,
        2,
        NULL,
        1);

    Serial.println("Robot System Started");

   /* xTaskCreatePinnedToCore(
    ToFTask,
    "ToF",
    4096,
    NULL,
    3,
    NULL,
    1);*/
}


void loop()
{
}