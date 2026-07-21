#ifndef STRUCTS_H
#define STRUCTS_H

#include <Arduino.h>
// ==========================
// Motion Types
// ==========================
enum MotionType
{
    MOTION_STOP,
    MOTION_FORWARD,
    MOTION_BACKWARD,
    MOTION_LEFT,
    MOTION_RIGHT
};

typedef struct
{
    MotionType motion;

    int leftSpeed;
    int rightSpeed;

} MotorCommand_t;
// ==========================
// Encoder Data
// ==========================

typedef struct
{
    long leftTicks;
    long rightTicks;

    float leftRPM;
    float rightRPM;

} EncoderData_t;

// ==========================
// Line Sensor Data
// ==========================

typedef struct
{
    int left;
    int center;
    int right;

    float error;

} LineSensorData_t;

// ==========================
// NFC Tag Data
// ==========================

typedef struct
{
    char uid[32];

} NFCData_t;

typedef struct
{
    uint16_t distanceMM;
    bool obstacleDetected;

} ToFData_t;

// ==========================
// Robot Command (from Comm)
// ==========================
typedef struct
{
    String type;         // message type
    String robotId;      // robot ID
    String command;      // e.g., "go_to_pharmacy", "deliver_medicine"
    char roomNumber[16]; // destination room number or room tag
} RobotCommand_t;

#endif