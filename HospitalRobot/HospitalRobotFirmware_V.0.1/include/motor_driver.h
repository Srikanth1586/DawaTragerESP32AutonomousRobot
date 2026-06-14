#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

void motorInit();

void moveForward(int leftSpeed, int rightSpeed);
void moveBackward(int leftSpeed, int rightSpeed);

void turnLeft(int speed);
void turnRight(int speed);

void stopMotors();

#endif