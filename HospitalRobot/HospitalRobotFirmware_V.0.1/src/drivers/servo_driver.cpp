#include "servo_driver.h"
#include "Config.h"
#include "ESP32Servo.h"

// ====================================
// Servo Objects
// ====================================

Servo servoLeftBox;
Servo servoMiddleBox;

// ====================================
// Servo Initialization
// ====================================

void servoInit()
{
    // Initialize servos with PWM channels
    servoLeftBox.setPeriodHertz(50);      // 50 Hz for standard servo
    servoMiddleBox.setPeriodHertz(50);
    
    servoLeftBox.attach(SERVO_L_BOX_DOOR, 1000, 2000);   // Attach to pin with pulse width range
    servoMiddleBox.attach(SERVO_M_BOX_DOOR, 1000, 2000);
    
    // Initialize doors in closed position
    servoLeftBox.write(DOOR_CLOSE_ANGLE);
    servoMiddleBox.write(DOOR_CLOSE_ANGLE);
    
    Serial.println("Servo Motors Initialized");
    Serial.print("L_BOX_DOOR: Pin ");
    Serial.println(SERVO_L_BOX_DOOR);
    Serial.print("M_BOX_DOOR: Pin ");
    Serial.println(SERVO_M_BOX_DOOR);
}

// ====================================
// Control Door (Open/Close)
// ====================================

void controlDoor(DoorType door, DoorCommand command)
{
    int angle;
    String doorName;
    
    // Determine angle based on command
    if (command == DOOR_OPEN)
    {
        angle = DOOR_OPEN_ANGLE;
        doorName = (door == DOOR_L_BOX) ? "L_BOX_DOOR" : "M_BOX_DOOR";
        Serial.print("Opening ");
        Serial.println(doorName);
    }
    else
    {
        angle = DOOR_CLOSE_ANGLE;
        doorName = (door == DOOR_L_BOX) ? "L_BOX_DOOR" : "M_BOX_DOOR";
        Serial.print("Closing ");
        Serial.println(doorName);
    }
    
    // Move servo to target angle
    setServoAngle(door, angle);
}

// ====================================
// Set Servo Angle
// ====================================

void setServoAngle(DoorType door, int angle)
{
    // Constrain angle to valid range (0-180)
    angle = constrain(angle, 0, 180);
    
    if (door == DOOR_L_BOX)
    {
        servoLeftBox.write(angle);
        Serial.print("L_BOX_DOOR angle: ");
        Serial.println(angle);
    }
    else if (door == DOOR_M_BOX)
    {
        servoMiddleBox.write(angle);
        Serial.print("M_BOX_DOOR angle: ");
        Serial.println(angle);
    }
}