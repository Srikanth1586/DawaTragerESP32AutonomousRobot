#include "motor_driver.h"
#include "Config.h"

// PWM Configuration
#define PWM_FREQ       1000
#define PWM_RESOLUTION 8

// Avoid using LEDC channels 0 and 1 because ESP32Servo may allocate them for servos.
#define LEFT_PWM_CH    2
#define RIGHT_PWM_CH   3

void motorInit()
{
    pinMode(LEFT_MOTOR_IN1, OUTPUT);
    pinMode(LEFT_MOTOR_IN2, OUTPUT);

    pinMode(RIGHT_MOTOR_IN1, OUTPUT);
    pinMode(RIGHT_MOTOR_IN2, OUTPUT);

    // Configure PWM channels
    ledcSetup(LEFT_PWM_CH, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(RIGHT_PWM_CH, PWM_FREQ, PWM_RESOLUTION);

    ledcAttachPin(LEFT_MOTOR_PWM, LEFT_PWM_CH);
    ledcAttachPin(RIGHT_MOTOR_PWM, RIGHT_PWM_CH);

    stopMotors();
}

void moveForward(int leftSpeed, int rightSpeed)
{
    leftSpeed  = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    digitalWrite(LEFT_MOTOR_IN1, HIGH);
    digitalWrite(LEFT_MOTOR_IN2, LOW);

    digitalWrite(RIGHT_MOTOR_IN1, HIGH);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);

    ledcWrite(LEFT_PWM_CH, leftSpeed);
    ledcWrite(RIGHT_PWM_CH, rightSpeed);
}

void moveBackward(int leftSpeed, int rightSpeed)
{
    leftSpeed  = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    digitalWrite(LEFT_MOTOR_IN1, LOW);
    digitalWrite(LEFT_MOTOR_IN2, HIGH);

    digitalWrite(RIGHT_MOTOR_IN1, LOW);
    digitalWrite(RIGHT_MOTOR_IN2, HIGH);

    ledcWrite(LEFT_PWM_CH, leftSpeed);
    ledcWrite(RIGHT_PWM_CH, rightSpeed);
}

void turnLeft(int speed)
{
    speed = constrain(speed, 0, 255);

    // Left motor slower/stopped
    digitalWrite(LEFT_MOTOR_IN1, HIGH);
    digitalWrite(LEFT_MOTOR_IN2, LOW);

    digitalWrite(RIGHT_MOTOR_IN1, LOW);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);

    ledcWrite(LEFT_PWM_CH, speed);
    //ledcWrite(RIGHT_PWM_CH, speed);
}

void turnRight(int speed)
{
    speed = constrain(speed, 0, 255);

    digitalWrite(LEFT_MOTOR_IN1, LOW);
    digitalWrite(LEFT_MOTOR_IN2, LOW);

    digitalWrite(RIGHT_MOTOR_IN1, HIGH);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);

    //ledcWrite(LEFT_PWM_CH, speed);
    ledcWrite(RIGHT_PWM_CH, speed);
}

void stopMotors()
{
    ledcWrite(LEFT_PWM_CH, 0);
    ledcWrite(RIGHT_PWM_CH, 0);

    digitalWrite(LEFT_MOTOR_IN1, LOW);
    digitalWrite(LEFT_MOTOR_IN2, LOW);

    digitalWrite(RIGHT_MOTOR_IN1, LOW);
    digitalWrite(RIGHT_MOTOR_IN2, LOW);
}