#ifndef DRIVERS_MOTOR_DRIVER_H
#define DRIVERS_MOTOR_DRIVER_H

#include <Arduino.h>

class MotorDriver {
public:
    MotorDriver() = default;
    ~MotorDriver() = default;

    void init();
    void setSpeeds(int16_t leftSpeed, int16_t rightSpeed);
    void stop();

private:
    void setMotorSpeed(uint8_t pwmPin, uint8_t dir1Pin, uint8_t dir2Pin, uint8_t ledcChannel, int16_t speed);
};

#endif // DRIVERS_MOTOR_DRIVER_H
