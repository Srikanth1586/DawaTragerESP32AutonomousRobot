#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <Arduino.h>

// ====================================
// Servo Door Types
// ====================================

typedef enum
{
    DOOR_L_BOX,
    DOOR_M_BOX
} DoorType;

typedef enum
{
    DOOR_OPEN,
    DOOR_CLOSE
} DoorCommand;

// ====================================
// Functions
// ====================================

void servoInit();
void controlDoor(DoorType door, DoorCommand command);
void setServoAngle(DoorType door, int angle);

#endif