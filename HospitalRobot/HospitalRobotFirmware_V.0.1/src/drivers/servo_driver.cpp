#include "servo_driver.h"
#include "Config.h"
#include <ESP32Servo.h>

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
    servoLeftBox.attach(SERVO_L_BOX_DOOR);   // Attach to pin with pulse width range
    servoMiddleBox.attach(SERVO_M_BOX_DOOR);
    
    // Initialize doors in closed position
    servoLeftBox.write(DOOR_CLOSE_ANGLE);
    servoMiddleBox.write(DOOR_CLOSE_ANGLE);
    Serial.println("Servo Motors Initialized");
}

// ====================================
// Control Door (Open/Close)
// ====================================

void controlDoor(DoorType door, DoorCommand command)
{int angle;
     // Ensure servos are initialized before controlling    
   
    String doorName;
    
    // Determine angle based on command
    if (command == DOOR_OPEN)
    {
        angle = DOOR_OPEN_ANGLE;
        doorName = (door == DOOR_L_BOX) ? "L_BOX_DOOR" : "M_BOX_DOOR";
        Serial.print("Opening SRK");
        Serial.println(doorName);
        Serial.print("L_BOX_DOOR angle: ");
        Serial.println(angle);
        
       
    }
    else
    {
        angle = DOOR_CLOSE_ANGLE;
        doorName = (door == DOOR_L_BOX) ? "L_BOX_DOOR" : "M_BOX_DOOR";
        Serial.print("Closing SRK ");
        Serial.println(doorName);
        Serial.print("L_BOX_DOOR angle: ");
        Serial.println(angle);
   // Allow time for the servo to reach the position
    }
    
    // Move servo to target angle
    setServoAngle(door, angle);
}

// ====================================
// Set Servo Angle
// ====================================

void setServoAngle(DoorType door, int sangle)
{
    if (door == DOOR_L_BOX)
    {
        Serial.println("Trying Left Servo");

        if (!servoLeftBox.attached())
        {
            Serial.println("Left servo not attached, reattaching...");
            servoLeftBox.attach(SERVO_L_BOX_DOOR);
        }

        servoLeftBox.write(sangle);
        Serial.print("Left servo angle: ");
        Serial.println(sangle);
    }

    else if (door == DOOR_M_BOX)
    {
        Serial.println("Trying Middle Servo");

        if (!servoMiddleBox.attached())
        {
            Serial.println("Middle servo not attached, reattaching...");
            servoMiddleBox.attach(SERVO_M_BOX_DOOR);
        }

        servoMiddleBox.write(sangle);
        Serial.print("Middle servo angle: ");
        Serial.println(sangle);
    }
}