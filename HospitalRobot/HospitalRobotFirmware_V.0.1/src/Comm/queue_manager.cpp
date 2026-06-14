#include "queue_manager.h"
#include "config.h"

// ==========================
// Queue Definitions
// ==========================

QueueHandle_t motorQueue;
QueueHandle_t encoderQueue;
QueueHandle_t lineSensorQueue;
QueueHandle_t nfcQueue;
QueueHandle_t tofQueue;
QueueHandle_t commandQueue;  
extern QueueHandle_t messageQueue;


// ==========================
// Queue Initialization
// ==========================

void initQueues()
{
     // Create message queue (10 items, each sizeof(String))
    messageQueue = xQueueCreate(MESSAGE_QUEUE_SIZE, sizeof(String));
    
    if(messageQueue == NULL)
    {
        Serial.println("Failed to create messageQueue");
    }
    
    motorQueue = xQueueCreate(
        MOTOR_QUEUE_SIZE,
        sizeof(MotorCommand_t));

    encoderQueue = xQueueCreate(
        SENSOR_QUEUE_SIZE,
        sizeof(EncoderData_t));

    lineSensorQueue = xQueueCreate(
        SENSOR_QUEUE_SIZE,
        sizeof(LineSensorData_t));

    nfcQueue = xQueueCreate(
        SENSOR_QUEUE_SIZE,
        sizeof(NFCData_t));

    tofQueue = xQueueCreate(
        SENSOR_QUEUE_SIZE,
        sizeof(ToFData_t));

    commandQueue = xQueueCreate(
        10,
        sizeof(RobotCommand_t));
}