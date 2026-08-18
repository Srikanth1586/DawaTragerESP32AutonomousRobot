#include "drivers/MotorDriver.h"
#include "config/pins.h"
#include "config/robot_config.h"

void MotorDriver::init() {
    // Configure direction control pins as outputs
    pinMode(PIN_MOTOR_L_DIR1, OUTPUT);
    pinMode(PIN_MOTOR_L_DIR2, OUTPUT);
    pinMode(PIN_MOTOR_R_DIR1, OUTPUT);
    pinMode(PIN_MOTOR_R_DIR2, OUTPUT);

    // Initial states: stop motors
    digitalWrite(PIN_MOTOR_L_DIR1, LOW);
    digitalWrite(PIN_MOTOR_L_DIR2, LOW);
    digitalWrite(PIN_MOTOR_R_DIR1, LOW);
    digitalWrite(PIN_MOTOR_R_DIR2, LOW);

    // Setup LEDC PWM channels
    ledcSetup(LEDC_CHANNEL_L, MOTOR_PWM_FREQ, MOTOR_PWM_RES);
    ledcSetup(LEDC_CHANNEL_R, MOTOR_PWM_FREQ, MOTOR_PWM_RES);

    // Attach the PWM pins to the LEDC channels
    ledcAttachPin(PIN_MOTOR_L_PWM, LEDC_CHANNEL_L);
    ledcAttachPin(PIN_MOTOR_R_PWM, LEDC_CHANNEL_R);

    // Start with 0 speed
    ledcWrite(LEDC_CHANNEL_L, 0);
    ledcWrite(LEDC_CHANNEL_R, 0);
}

void MotorDriver::setSpeeds(int16_t leftSpeed, int16_t rightSpeed) {
    // Constrain speeds to limits
    leftSpeed = constrain(leftSpeed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    rightSpeed = constrain(rightSpeed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

    setMotorSpeed(PIN_MOTOR_L_PWM, PIN_MOTOR_L_DIR1, PIN_MOTOR_L_DIR2, LEDC_CHANNEL_L, leftSpeed);
    setMotorSpeed(PIN_MOTOR_R_PWM, PIN_MOTOR_R_DIR1, PIN_MOTOR_R_DIR2, LEDC_CHANNEL_R, rightSpeed);
}

void MotorDriver::stop() {
    setSpeeds(0, 0);
}

void MotorDriver::setMotorSpeed(uint8_t pwmPin, uint8_t dir1Pin, uint8_t dir2Pin, uint8_t ledcChannel, int16_t speed) {
    if (speed > 0) {
        // Forward direction: IN1 High, IN2 Low
        digitalWrite(dir1Pin, HIGH);
        digitalWrite(dir2Pin, LOW);
        ledcWrite(ledcChannel, static_cast<uint32_t>(speed));
    } else if (speed < 0) {
        // Reverse direction: IN1 Low, IN2 High
        digitalWrite(dir1Pin, LOW);
        digitalWrite(dir2Pin, HIGH);
        ledcWrite(ledcChannel, static_cast<uint32_t>(-speed));
    } else {
        // Motor Stop/Brake: IN1 Low, IN2 Low
        digitalWrite(dir1Pin, LOW);
        digitalWrite(dir2Pin, LOW);
        ledcWrite(ledcChannel, 0);
    }
}
